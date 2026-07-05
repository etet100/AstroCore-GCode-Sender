// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "feedrateconverter.h"
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
    : m_multiplier(multiplier)
{
}

QList<GCodeItem> FeedRateConverter::push(const GCodeItem &input)
{
    if (input.line.trimmed().isEmpty() || input.line.trimmed().startsWith(';')) {
        return { input };
    }

    static const QRegularExpression feedRateRegex("F([0-9.]+)",
                                                  QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = feedRateRegex.match(input.line);

    if (!match.hasMatch()) {
        return { input };
    }

    double originalFeedRate = match.captured(1).toDouble();
    double newFeedRate = originalFeedRate * m_multiplier;

    QString newFeedRateStr = QString("F%1").arg(newFeedRate, 0, 'f', 2);

    GCodeItem out = input;
    out.line.replace(match.capturedStart(), match.capturedLength(), newFeedRateStr);

    return { out };
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
    : m_offsetX(offsetX)
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

QList<GCodeItem> CoordinateOffsetConverter::push(const GCodeItem &input)
{
    if (!input.isMovement) {
        return { input };
    }

    bool modified = false;
    QString line = input.line;

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

    if (!modified) {
        return { input };
    }

    GCodeItem out = input;
    out.line = line;

    return { out };
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
  "description": "Example converter demonstrating 1-line lookahead via internal buffering. Annotates an M5 that is immediately followed by a rapid move.",
  "image": ":/images/converters/safespindlestop.svg",
  "fields": []
})JSON");
}

QList<GCodeItem> SafeSpindleStopConverter::push(const GCodeItem &input)
{
    QList<GCodeItem> out;

    if (m_pending) {
        GCodeItem item = *m_pending;
        if (item.command().startsWith("M5") && input.command().startsWith("G0")) {
            item.line += " ; Warning: Rapid move after spindle stop";
        }
        out << item;
    }

    m_pending = input;

    return out;
}

QList<GCodeItem> SafeSpindleStopConverter::flush()
{
    if (!m_pending) {
        return {};
    }

    QList<GCodeItem> out = { *m_pending };
    m_pending.reset();

    return out;
}

QString MovementOptimizerConverter::parameterSchema()
{
    return QStringLiteral(R"JSON({
  "title": "Movement optimizer (example)",
  "description": "Example converter demonstrating N-line lookahead via internal buffering. Annotates G1 movement lines that are followed by more G1 movements.",
  "image": ":/images/converters/movementoptimizer.svg",
  "fields": []
})JSON");
}

QList<GCodeItem> MovementOptimizerConverter::push(const GCodeItem &input)
{
    m_buffer.append(input);

    if (m_buffer.size() <= LOOKAHEAD) {
        return {};
    }

    return { annotate(m_buffer.takeFirst()) };
}

QList<GCodeItem> MovementOptimizerConverter::flush()
{
    QList<GCodeItem> out;
    while (!m_buffer.isEmpty()) {
        out << annotate(m_buffer.takeFirst());
    }

    return out;
}

GCodeItem MovementOptimizerConverter::annotate(const GCodeItem &item) const
{
    if (!item.command().startsWith("G1") || !item.isMovement) {
        return item;
    }

    int consecutiveMoves = 0;
    for (const auto &ahead : m_buffer) {
        if (!ahead.command().startsWith("G1") || !ahead.isMovement) {
            break;
        }
        consecutiveMoves++;
    }

    if (consecutiveMoves == 0) {
        return item;
    }

    GCodeItem out = item;
    out.line += QString(" ; %1 consecutive moves").arg(consecutiveMoves + 1);

    return out;
}
