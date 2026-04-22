// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "movepath.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>
#include <QtGlobal>
#include <cmath>

QString MovePath::parameterSchema()
{
    return QStringLiteral(R"JSON({
  "title": "Move path",
  "description": "Shifts every G90 (absolute) movement line by fixed X/Y/Z offsets. G91 (incremental) lines are left unchanged.",
  "image": ":/images/converters/movepath.svg",
  "fields": [
    {
      "name": "offsetX",
      "label": "Offset X",
      "type": "float",
      "min": -10000.0,
      "max": 10000.0,
      "default": 0.0,
      "description": "Value added to X coordinates on G90 movement lines. Units: mm."
    },
    {
      "name": "offsetY",
      "label": "Offset Y",
      "type": "float",
      "min": -10000.0,
      "max": 10000.0,
      "default": 0.0,
      "description": "Value added to Y coordinates on G90 movement lines. Units: mm."
    },
    {
      "name": "offsetZ",
      "label": "Offset Z",
      "type": "float",
      "min": -10000.0,
      "max": 10000.0,
      "default": 0.0,
      "description": "Value added to Z coordinates on G90 movement lines. Units: mm."
    }
  ]
})JSON");
}

MovePath::MovePath(double offsetX, double offsetY, double offsetZ)
    : m_offsetX(offsetX), m_offsetY(offsetY), m_offsetZ(offsetZ)
{
    reset();
}

void MovePath::reset()
{
    m_absoluteMode = true;  // G90 is the default mode in most CNC controllers
}

void MovePath::setOffset(double x, double y, double z)
{
    m_offsetX = x;
    m_offsetY = y;
    m_offsetZ = z;
}

bool MovePath::convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser)
{
    Q_UNUSED(gcode); Q_UNUSED(currentIndex); Q_UNUSED(parser);

    if (item.state == GCodeItem::EmptyLine || item.state == GCodeItem::Comment) {
        return false;
    }

    // Track absolute / incremental mode changes
    for (float gc : GcodePreprocessorUtils::parseCodes(item.args, 'G')) {
        int g = qRound(gc);
        if (g == 90)      m_absoluteMode = true;
        else if (g == 91) m_absoluteMode = false;
    }

    if (!item.isMovement || !m_absoluteMode) return false;

    // Split line into command and comment parts so we don't modify comment text
    int parenPos = item.line.indexOf('(');
    int semiPos  = item.line.indexOf(';');
    int splitPos = -1;
    if (parenPos >= 0 && semiPos >= 0) splitPos = qMin(parenPos, semiPos);
    else if (parenPos >= 0)            splitPos = parenPos;
    else if (semiPos >= 0)             splitPos = semiPos;

    QString cmdPart     = (splitPos >= 0) ? item.line.left(splitPos) : item.line;
    QString commentPart = (splitPos >= 0) ? item.line.mid(splitPos)  : QString();

    QString newCmd = cmdPart;

    if (m_offsetX != 0.0 && !qIsNaN(GcodePreprocessorUtils::parseCoord(item.args, 'X')))
        newCmd = applyOffset(newCmd, 'X', m_offsetX);
    if (m_offsetY != 0.0 && !qIsNaN(GcodePreprocessorUtils::parseCoord(item.args, 'Y')))
        newCmd = applyOffset(newCmd, 'Y', m_offsetY);
    if (m_offsetZ != 0.0 && !qIsNaN(GcodePreprocessorUtils::parseCoord(item.args, 'Z')))
        newCmd = applyOffset(newCmd, 'Z', m_offsetZ);

    if (newCmd == cmdPart) return false;

    item.line = newCmd + commentPart;
    item.args = GcodePreprocessorUtils::splitCommand(
        GcodePreprocessorUtils::removeComment(item.line)
    );
    return true;
}

QString MovePath::applyOffset(const QString &cmd, char axis, double offset) const
{
    // Matches: axis letter (case-insensitive), optional space, then a signed number
    QRegularExpression re(
        QString("[%1]\\s*([+-]?[0-9]*\\.?[0-9]+)").arg(axis),
        QRegularExpression::CaseInsensitiveOption
    );

    QRegularExpressionMatch match = re.match(cmd);
    if (!match.hasMatch()) return cmd;

    double originalVal = match.captured(1).toDouble();
    double newVal      = originalVal + offset;

    // Compact format: 4 decimal places, trailing zeros removed
    QString newStr = QString("%1%2").arg(axis).arg(newVal, 0, 'f', 4);
    if (newStr.contains('.')) {
        newStr.remove(QRegularExpression("0+$"));
        newStr.remove(QRegularExpression("\\.$"));
    }

    QString result = cmd;
    result.replace(match.capturedStart(), match.capturedLength(), newStr);
    return result;
}
