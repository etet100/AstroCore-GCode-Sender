// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "feedrateconverter.h"
#include "core/gcode/parser/gcodeparser.h"
#include <QRegularExpression>

QString FeedRateConverter::parameterSchema()
{
    return QStringLiteral(R"JSON({
  "title": "Feed rate multiplier (example)",
  "description": "Example converter: multiplies every F value by a constant. Use 'Modify feed rate' for the production version with percentage and range checks.",
  "image": ":/images/converters/feedrate.svg",
  "fields": [
    {
      "name": "multiplier",
      "label": "Multiplier",
      "type": "float",
      "min": 0.1,
      "max": 10.0,
      "default": 1.0,
      "description": "Scaling factor applied to every F value. 1.0 = no change, 0.5 = half, 2.0 = double."
    }
  ]
})JSON");
}

FeedRateConverter::FeedRateConverter(double multiplier)
    : AbstractConverter()
    , m_multiplier(multiplier)
{
}

bool FeedRateConverter::convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser)
{
    Q_UNUSED(gcode);
    Q_UNUSED(currentIndex);
    Q_UNUSED(parser);

    if (item.line.trimmed().isEmpty() || item.line.trimmed().startsWith(';')) {
        return false;
    }

    static QRegularExpression feedRateRegex("F([0-9.]+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = feedRateRegex.match(item.line);

    if (match.hasMatch()) {
        double originalFeedRate = match.captured(1).toDouble();
        double newFeedRate = originalFeedRate * m_multiplier;

        QString newFeedRateStr = QString("F%1").arg(newFeedRate, 0, 'f', 2);
        item.line.replace(match.capturedStart(), match.capturedLength(), newFeedRateStr);

        return true;
    }

    return false;
}

void FeedRateConverter::reset()
{
    AbstractConverter::reset();
}

QString CoordinateOffsetConverter::parameterSchema()
{
    return QStringLiteral(R"JSON({
  "title": "Coordinate offset (example)",
  "description": "Example converter: adds fixed X/Y/Z offsets to movement lines. Use 'Move path' for the production version that tracks G90/G91 mode.",
  "image": ":/images/converters/coordinateoffset.svg",
  "fields": [
    {
      "name": "offsetX",
      "label": "Offset X",
      "type": "float",
      "min": -10000.0,
      "max": 10000.0,
      "default": 0.0,
      "description": "Value added to X coordinates. Units: mm."
    },
    {
      "name": "offsetY",
      "label": "Offset Y",
      "type": "float",
      "min": -10000.0,
      "max": 10000.0,
      "default": 0.0,
      "description": "Value added to Y coordinates. Units: mm."
    },
    {
      "name": "offsetZ",
      "label": "Offset Z",
      "type": "float",
      "min": -10000.0,
      "max": 10000.0,
      "default": 0.0,
      "description": "Value added to Z coordinates. Units: mm."
    }
  ]
})JSON");
}

CoordinateOffsetConverter::CoordinateOffsetConverter(double offsetX, double offsetY, double offsetZ)
    : AbstractConverter()
    , m_offsetX(offsetX)
    , m_offsetY(offsetY)
    , m_offsetZ(offsetZ)
{
}

void CoordinateOffsetConverter::setOffset(double x, double y, double z)
{
    m_offsetX = x;
    m_offsetY = y;
    m_offsetZ = z;
}

bool CoordinateOffsetConverter::convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser)
{
    Q_UNUSED(gcode);
    Q_UNUSED(currentIndex);
    Q_UNUSED(parser);

    if (!item.isMovement) {
        return false;
    }

    bool modified = false;
    QString line = item.line;

    if (m_offsetX != 0.0) {
        QString newLine = modifyCoordinate(line, 'X', m_offsetX);
        if (newLine != line) {
            line = newLine;
            modified = true;
        }
    }

    if (m_offsetY != 0.0) {
        QString newLine = modifyCoordinate(line, 'Y', m_offsetY);
        if (newLine != line) {
            line = newLine;
            modified = true;
        }
    }

    if (m_offsetZ != 0.0) {
        QString newLine = modifyCoordinate(line, 'Z', m_offsetZ);
        if (newLine != line) {
            line = newLine;
            modified = true;
        }
    }

    if (modified) {
        item.line = line;
    }

    return modified;
}

void CoordinateOffsetConverter::reset()
{
    AbstractConverter::reset();
}

QString CoordinateOffsetConverter::modifyCoordinate(const QString &arg, char axis, double offset)
{
    QString pattern = QString("%1([+-]?[0-9.]+)").arg(axis);
    QRegularExpression regex(pattern, QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = regex.match(arg);

    if (match.hasMatch()) {
        double originalValue = match.captured(1).toDouble();
        double newValue = originalValue + offset;

        QString newCoordStr = QString("%1%2").arg(axis).arg(newValue, 0, 'f', 3);
        QString result = arg;
        result.replace(match.capturedStart(), match.capturedLength(), newCoordStr);
        return result;
    }

    return arg;
}

QString SafeSpindleStopConverter::parameterSchema()
{
    return QStringLiteral(R"JSON({
  "title": "Safe spindle stop (example)",
  "description": "Example converter demonstrating the lookahead mechanism. Ensures the spindle is not stopped immediately before a cutting move.",
  "image": ":/images/converters/safespindlestop.svg",
  "fields": []
})JSON");
}

SafeSpindleStopConverter::SafeSpindleStopConverter()
    : AbstractConverter()
{
}

bool SafeSpindleStopConverter::convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser)
{
    Q_UNUSED(parser);

    if (!item.command().startsWith("M5")) {
        return false;
    }

    GCodeItem *nextLine = gcode ? gcode->lookAhead(currentIndex, 1) : nullptr;
    if (!nextLine) {
        return false;
    }

    if (nextLine->command().startsWith("G0")) {
        item.line += " ; Warning: Rapid move after spindle stop";
        return true;
    }

    return false;
}

QString MovementOptimizerConverter::parameterSchema()
{
    return QStringLiteral(R"JSON({
  "title": "Movement optimizer (example)",
  "description": "Example converter demonstrating full G-code access and lookahead. Analyzes neighbouring movement lines to spot optimisations.",
  "image": ":/images/converters/movementoptimizer.svg",
  "fields": []
})JSON");
}

MovementOptimizerConverter::MovementOptimizerConverter()
    : AbstractConverter()
{
}

bool MovementOptimizerConverter::convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser)
{
    Q_UNUSED(parser);

    if (!item.command().startsWith("G1") || !item.isMovement) {
        return false;
    }

    int consecutiveMoves = 0;
    for (int i = 1; i <= 5; ++i) {
        GCodeItem *ahead = gcode ? gcode->lookAhead(currentIndex, i) : nullptr;
        if (!ahead || !ahead->command().startsWith("G1") || !ahead->isMovement) {
            break;
        }
        consecutiveMoves++;
    }

    if (consecutiveMoves > 0) {
        item.line += QString(" ; %1 consecutive moves").arg(consecutiveMoves + 1);
        return true;
    }

    // Example: scan entire program for statistics
    if (gcode && currentIndex < 10) {
        int totalMoves = 0;
        for (int i = 0; i < gcode->count(); ++i) {
            if ((*gcode)[i].isMovement) {
                totalMoves++;
            }
        }
        item.line += QString(" ; Total moves: %1").arg(totalMoves);
        return true;
    }

    return false;
}
