// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "shakinggcode.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>
#include <QtMath>

QString ShakingGCode::parameterSchema()
{
    return QStringLiteral(R"JSON({
  "title": "Shake path",
  "description": "Test converter. Segments movement lines and adds a random XYZ offset to each segment endpoint. Intended for intentional path distortion when testing the sender.",
  "image": ":/images/converters/shakinggcode.svg",
  "fields": [
    {
      "name": "segmentLength",
      "label": "Segment length",
      "type": "float",
      "min": 0.1,
      "max": 100.0,
      "default": 5.0,
      "description": "Movement lines are split every N mm before random offsets are applied. Units: mm."
    },
    {
      "name": "maxOffset",
      "label": "Max offset",
      "type": "float",
      "min": 0.0,
      "max": 10.0,
      "default": 1.0,
      "description": "Maximum random XY (and optionally Z) offset added to each segment endpoint (±). Units: mm."
    },
    {
      "name": "shakeZ",
      "label": "Shake Z axis",
      "type": "bool",
      "default": false,
      "description": "When on, random offset is also applied to the Z axis. When off, only X and Y are disturbed."
    }
  ]
})JSON");
}

ShakingGCode::ShakingGCode(double segmentLength, double maxOffset, bool shakeZ)
    : m_parser(new GcodeParser())
    , m_random(QRandomGenerator::global())
    , m_ownsRandom(false)
    , m_segmentLength(segmentLength)
    , m_maxOffset(maxOffset)
    , m_feedRateVariation(0.2)
    , m_shakeZ(shakeZ)
{
}

ShakingGCode::~ShakingGCode()
{
    delete m_parser;
    if (m_ownsRandom) {
        delete m_random;
    }
}

void ShakingGCode::setSeed(quint32 seed)
{
    if (m_ownsRandom) {
        delete m_random;
    }
    m_random = new QRandomGenerator(seed);
    m_ownsRandom = true;
}

void ShakingGCode::reset()
{
    m_parser->reset();
    m_pending.reset();
}

QList<GCodeItem> ShakingGCode::push(const GCodeItem &input)
{
    QList<GCodeItem> out;

    if (m_pending) {
        out = processOne(*m_pending, /*nextIsArc=*/input.isArc());
    }

    m_pending = input;

    return out;
}

QList<GCodeItem> ShakingGCode::flush()
{
    if (!m_pending) {
        return {};
    }

    QList<GCodeItem> out = processOne(*m_pending, /*nextIsArc=*/false);
    m_pending.reset();

    return out;
}

QList<GCodeItem> ShakingGCode::processOne(const GCodeItem &item, bool nextIsArc)
{
    if (!item.isMovement) {
        m_parser->addCommand(item);

        return { item };
    }

    QVector3D startPoint = *m_parser->getCurrentPoint();

    m_parser->pushState();
    PointSegment *ps = m_parser->addCommand(item);

    if (!ps) {
        m_parser->popState();

        return { item };
    }

    QVector3D endPoint = *ps->point();

    // Only handle linear movements (G0/G1) — skip arcs for simplicity.
    if (ps->isArc()) {
        m_parser->popState();
        m_parser->addCommand(item);

        return { item };
    }

    m_parser->popState();

    QList<QVector3D> points = segmentLine(startPoint, endPoint, nextIsArc);

    if (points.size() <= 2) {
        if (points.size() == 2) {
            QVector3D modifiedEnd = points[1];
            if (!nextIsArc) {
                applyRandomOffset(modifiedEnd);
            }

            GCodeItem modified = item;
            modified.line = generateGCodeLine(startPoint, modifiedEnd, item, true);
            modified.args = GcodePreprocessorUtils::splitCommand(modified.line);
            m_parser->addCommand(modified);

            return { modified };
        }

        m_parser->addCommand(item);

        return { item };
    }

    QList<GCodeItem> out;
    out.reserve(points.size() - 1);
    for (int i = 0; i < points.size() - 1; i++) {
        GCodeItem segment;
        segment.line          = generateGCodeLine(points[i], points[i + 1], item, i == 0);
        segment.state         = GCodeItem::InQueue;
        segment.args          = GcodePreprocessorUtils::splitCommand(segment.line);
        segment.isMovement    = true;
        segment.group         = item.group;
        segment.commandNumber = item.commandNumber;
        segment.overlayId     = item.overlayId;

        out << segment;
        m_parser->addCommand(segment);
    }

    return out;
}

QList<QVector3D> ShakingGCode::segmentLine(const QVector3D &start, const QVector3D &end, bool nextIsArc)
{
    QList<QVector3D> points;
    points.append(start);

    double length = (end - start).length();

    if (length <= m_segmentLength) {
        QVector3D endWithOffset = end;
        if (!nextIsArc) {
            applyRandomOffset(endWithOffset);
        }
        points.append(endWithOffset);

        return points;
    }

    int numSegments = qCeil(length / m_segmentLength);
    QVector3D direction = (end - start).normalized();
    double segmentLen = length / numSegments;

    // Generate intermediate points with random offsets.
    // The last point (arc start) must not be offset when the next command is an arc.
    for (int i = 1; i <= numSegments; i++) {
        QVector3D point = start + direction * (segmentLen * i);
        bool isLastPoint = (i == numSegments);
        if (!isLastPoint || !nextIsArc) {
            applyRandomOffset(point);
        }
        points.append(point);
    }

    return points;
}

void ShakingGCode::applyRandomOffset(QVector3D &point)
{
    if (m_maxOffset <= 0.0) {
        return;
    }

    double offsetX = (m_random->generateDouble() * 2.0 - 1.0) * m_maxOffset;
    double offsetY = (m_random->generateDouble() * 2.0 - 1.0) * m_maxOffset;

    point.setX(point.x() + offsetX);
    point.setY(point.y() + offsetY);

    if (m_shakeZ) {
        double offsetZ = (m_random->generateDouble() * 2.0 - 1.0) * m_maxOffset;
        point.setZ(point.z() + offsetZ);
    }
}

double ShakingGCode::getRandomFeedRate(double originalFeedRate)
{
    if (originalFeedRate <= 0.0 || m_feedRateVariation <= 0.0) {
        return originalFeedRate;
    }

    double factor = 1.0 + (m_random->generateDouble() * 2.0 - 1.0) * m_feedRateVariation;
    factor = qMax(0.5, qMin(1.5, factor));

    return originalFeedRate * factor;
}

QString ShakingGCode::generateGCodeLine(const QVector3D &start, const QVector3D &end,
                                        const GCodeItem &originalItem, bool isFirstSegment)
{
    Q_UNUSED(start);
    Q_UNUSED(isFirstSegment);

    QString command;
    {
        const QString full = originalItem.command();
        const int sp = full.indexOf(' ');
        command = (sp < 0) ? full : full.left(sp);
    }
    if (command.isEmpty() && !originalItem.args.empty()) {
        command = QString::fromStdString(originalItem.args.front());
    }

    QString line = command;

    line += QString(" X%1").arg(end.x(), 0, 'f', 3);
    line += QString(" Y%1").arg(end.y(), 0, 'f', 3);
    line += QString(" Z%1").arg(end.z(), 0, 'f', 3);

    // F is appended to every segment so each one is independently controllable
    // (the random feed-rate variation makes each segment intentionally distinct).
    static QRegularExpression feedRegex("F([0-9.]+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = feedRegex.match(originalItem.line);

    if (match.hasMatch()) {
        double originalFeedRate = match.captured(1).toDouble();
        double modifiedFeedRate = getRandomFeedRate(originalFeedRate);

        line += QString(" F%1").arg(modifiedFeedRate, 0, 'f', 1);
    }

    return line;
}
