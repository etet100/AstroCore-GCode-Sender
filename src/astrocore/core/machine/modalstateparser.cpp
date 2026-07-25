// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "modalstateparser.h"
#include <QStringList>

std::optional<ModalState> ModalStateParser::parse(const QString &line)
{
    if (!line.startsWith("[GC:") || !line.endsWith(']')) {
        return std::nullopt;
    }

    const QString inner = line.mid(4, line.length() - 5);
    const QStringList tokens = inner.split(' ', Qt::SkipEmptyParts);

    ModalState state;
    state.raw = inner;

    for (const QString &token : tokens) {
        if (token.startsWith('G')) {
            const QString upper = token.toUpper();

            if (upper == "G54" || upper == "G55" || upper == "G56" ||
                upper == "G57" || upper == "G58" || upper == "G59")
            {
                state.coordinateSystem = upper;
            } else if (upper == "G17" || upper == "G18" || upper == "G19") {
                state.workPlane = upper;
            } else if (upper == "G20" || upper == "G21") {
                state.units = upper;
            } else if (upper == "G90" || upper == "G91") {
                state.motionMode = upper;
            } else if (upper == "G93" || upper == "G94" || upper == "G95") {
                state.feedMode = upper;
            }
        } else if (token.startsWith('M')) {
            const QString upper = token.toUpper();

            if (upper == "M3" || upper == "M4" || upper == "M5") {
                state.spindleMode = upper;
            }
        } else if (token.startsWith('F')) {
            // Some firmwares report fractional values (F500.000), so parse as double.
            bool ok = false;
            double value = token.mid(1).toDouble(&ok);
            if (ok) {
                state.feedRate = qRound(value);
            }
        } else if (token.startsWith('S')) {
            bool ok = false;
            double value = token.mid(1).toDouble(&ok);
            if (ok) {
                state.spindleSpeed = qRound(value);
            }
        }
    }

    return state;
}

QString ModalState::toString() const
{
    QString sep = "————————————";
    QString result;
    result += sep + "\n";
    result += QString("Coord: %1\n").arg(coordinateSystem);
    result += QString("Plane: %1\n").arg(workPlane);
    result += QString("Units: %1\n").arg(units == "G20" ? "inches" : units == "G21" ? "mm" : units);
    result += QString("Motion: %1\n").arg(motionMode == "G90" ? "absolute" : motionMode == "G91" ? "relative" : motionMode);
    result += QString("Feed mode: %1\n").arg(feedMode);
    result += QString("Feed: %1\n").arg(feedRate);
    result += QString("Spindle: %1\n").arg(spindleMode == "M3" ? "CW" : spindleMode == "M4" ? "CCW" : spindleMode == "M5" ? "off" : spindleMode);
    result += QString("Speed: %1\n").arg(spindleSpeed);
    result += sep;

    return result;
}
