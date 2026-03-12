// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "fusionrestorerapidmovements.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QSet>
#include <QtGlobal>
#include <cmath>

static const QSet<int> MOTION_GCODES = {0, 1, 2, 3, 33, 38, 73, 76, 80, 81, 82, 84, 85, 86, 87, 88, 89};
static const QSet<int> HOME_GCODES   = {28, 30};

FusionRestoreRapidMovements::FusionRestoreRapidMovements()
{
    reset();
}

void FusionRestoreRapidMovements::reset()
{
    m_lastMotionGcode = -1;
    m_zcur            = qQNaN();
    m_zlast           = qQNaN();
    m_zfeed           = qQNaN();
    m_zfeedNotSet     = true;
    m_feedcur         = 0.0;
    m_needFeed        = false;
    m_lockSpeed       = false;
}

bool FusionRestoreRapidMovements::convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser)
{
    Q_UNUSED(gcode)
    Q_UNUSED(currentIndex)
    Q_UNUSED(parser)

    if (item.state == GCodeItem::EmptyLine || item.state == GCodeItem::Comment) {
        return false;
    }

    const auto &args = item.args;

    // Handle M49/M48 (disable/enable speed overrides)
    for (float mc : GcodePreprocessorUtils::parseCodes(args, 'M')) {
        int m = qRound(mc);
        if (m == 49)      m_lockSpeed = true;
        else if (m == 48) m_lockSpeed = false;
    }

    // Find the first motion or home G-code on this line
    bool fNoMotionGcode = true;
    bool fHomeGcode     = false;
    for (float gc : GcodePreprocessorUtils::parseCodes(args, 'G')) {
        int g = static_cast<int>(gc);
        if (HOME_GCODES.contains(g)) {
            fHomeGcode = true;
            break;
        }
        if (MOTION_GCODES.contains(g)) {
            fNoMotionGcode    = false;
            m_lastMotionGcode = g;
            if (g == 0) m_needFeed = false;
            break;
        }
    }

    if (fHomeGcode) {
        return false;
    }

    double zVal = GcodePreprocessorUtils::parseCoord(args, 'Z');
    double fVal = GcodePreprocessorUtils::parseCoord(args, 'F');
    QString xy  = extractXY(args);
    bool hasZ   = !qIsNaN(zVal);
    bool hasXY  = !xy.isEmpty();
    bool hasF   = !qIsNaN(fVal);

    if (hasZ) {
        m_zlast = m_zcur;
        m_zcur  = zVal;
    }
    if (hasF) {
        m_feedcur = fVal;
    }

    QString original = item.line.trimmed();
    QString newLine;

    // Step A: learn Zfeed from the first two Z-only moves (G0 or G1, no XY)
    if ((qIsNaN(m_zfeed) || m_zfeedNotSet)
        && (m_lastMotionGcode == 0 || m_lastMotionGcode == 1)
        && hasZ && !hasXY)
    {
        if (!qIsNaN(m_zfeed)) {
            m_zfeedNotSet = false;
        }
        m_zfeed = m_zcur;

        if (m_lastMotionGcode != 0) {
            // This G1 Z-only move is the clearance/plunge detection point — make it rapid
            newLine           = QString("G0 Z%1 (Changed from: \"%2\")").arg(formatZ(m_zcur)).arg(original);
            m_needFeed        = true;
            m_lastMotionGcode = 0;
        }
    }

    // Step B: convert G1 to G0 when the move is above feed height or clearly retract
    if (m_lastMotionGcode == 1 && !m_lockSpeed) {
        if (hasZ) {
            // Upward move, or above feed height, or zero feed — convert to rapid
            // Python bug fix: Zlast == None crashes; we guard with qIsNaN check
            bool upward = (!qIsNaN(m_zlast) && m_zcur >= m_zlast)
                       || (!qIsNaN(m_zfeed) && m_zcur >= m_zfeed)
                       || m_feedcur == 0.0;
            if (!hasXY && upward) {
                newLine           = QString("G0 Z%1 (Changed from: \"%2\")").arg(formatZ(m_zcur)).arg(original);
                m_needFeed        = true;
                m_lastMotionGcode = 0;
            }
        } else if (!qIsNaN(m_zfeed) && m_zcur >= m_zfeed) {
            // XY-only move at or above feed height
            newLine           = QString("G0 %1 (Changed from: \"%2\")").arg(xy).arg(original);
            m_needFeed        = true;
            m_lastMotionGcode = 0;
        }
    }
    // Step C: restore G1 on modal lines that follow a converted G0 and descend below feed height
    else if (m_needFeed && fNoMotionGcode) {
        if (hasZ) {
            if (hasXY) {
                newLine = QString("G1 %1 Z%2 F%3 (Changed from: \"%4\")")
                              .arg(xy).arg(formatZ(m_zcur)).arg(formatF(m_feedcur)).arg(original);
                m_needFeed        = false;
                m_lastMotionGcode = 1;
            } else if (!qIsNaN(m_zfeed) && m_zcur < m_zfeed
                       && !qIsNaN(m_zlast) && m_zcur <= m_zlast)
            {
                newLine = QString("G1 Z%1 F%2 (Changed from: \"%3\")")
                              .arg(formatZ(m_zcur)).arg(formatF(m_feedcur)).arg(original);
                m_needFeed        = false;
                m_lastMotionGcode = 1;
            }
        } else if (hasXY && !qIsNaN(m_zfeed) && m_zcur < m_zfeed) {
            newLine = QString("G1 %1 F%2 (Changed from: \"%3\")")
                          .arg(xy).arg(formatF(m_feedcur)).arg(original);
            m_needFeed        = false;
            m_lastMotionGcode = 1;
        }
    }

    // Step D: add explicit F to G1 lines that follow a G0 conversion but weren't covered by C
    if (m_lastMotionGcode != 0 && m_needFeed) {
        if (!hasF) {
            QString base = newLine.isEmpty() ? original : newLine;
            newLine = base + QString(" F%1 (Feed rate added)").arg(formatF(m_feedcur));
        }
        m_needFeed = false;
    }

    // Step E: adjust Zfeed upward if a cutting move is found above it (Zfeed was wrong)
    if (!qIsNaN(m_zcur) && !qIsNaN(m_zfeed) && m_zcur >= m_zfeed
        && m_lastMotionGcode != -1 && m_lastMotionGcode != 0
        && hasXY && (hasZ || m_lastMotionGcode != 1))
    {
        m_zfeed = m_zcur + 0.001;
    }

    if (newLine.isEmpty()) {
        return false;
    }

    item.line       = newLine;
    item.command    = GcodePreprocessorUtils::removeComment(newLine);
    item.args       = GcodePreprocessorUtils::splitCommand(item.command);
    item.isMovement = true;
    item.group      = (m_lastMotionGcode == 0) ? GCodeItemGroup::RapidMovement
                                               : GCodeItemGroup::Movement;

    return true;
}

QString FusionRestoreRapidMovements::formatZ(double z)
{
    return QString::number(z, 'f', 3);
}

QString FusionRestoreRapidMovements::formatF(double f)
{
    return QString::number(f, 'f', 1);
}

QString FusionRestoreRapidMovements::extractXY(const std::vector<std::string> &args)
{
    QString xy;
    for (const std::string &arg : args) {
        if (!arg.empty() && (arg[0] == 'X' || arg[0] == 'Y')) {
            if (!xy.isEmpty()) xy += ' ';
            xy += QString::fromLatin1(arg.c_str(), (qsizetype)arg.size());
        }
    }

    return xy;
}
