// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "shakinggcode.h"
#include "core/gcode/gcode.h"
#include "core/gcode/parser/gcodeparser.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>
#include <QtMath>

// Test converter that randomly modifies movement paths
// - Segments lines every 5mm (configurable)
// - Adds random offset to each point (±1mm configurable)
// - Randomly varies feed rate (±20% configurable)

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
      "description": "Maximum random XYZ offset added to each segment endpoint (±). Units: mm."
    }
  ]
})JSON");
}

ShakingGCode::ShakingGCode(double segmentLength, double maxOffset, QObject *parent)
    : QObject(parent)
    , m_parser(nullptr)
    , m_gcode(nullptr)
    , m_random(nullptr)
    , m_segmentLength(segmentLength)
    , m_maxOffset(maxOffset)
    , m_feedRateVariation(0.2)  // ±20% by default
    , m_currentIndex(0)
{
    m_random = QRandomGenerator::global();
}

ShakingGCode::~ShakingGCode()
{
    delete m_parser;
}

void ShakingGCode::setSeed(quint32 seed)
{
    m_random = new QRandomGenerator(seed);
}

void ShakingGCode::setGCode(GCode *gcode)
{
    m_gcode = gcode;
    reset();
}

void ShakingGCode::reset()
{
    m_currentIndex = 0;

    if (!m_parser) {
        m_parser = new GcodeParser();
    }
    m_parser->reset();
}

int ShakingGCode::convertNext(int count)
{
    if (!m_gcode) {
        return 0;
    }

    int processed = 0;
    int targetCount = qMin(m_currentIndex + count, m_gcode->count());

    QList<GCodeItem> accumulatedItems;

    for (int i = m_currentIndex; i < targetCount; ++i) {
        const GCodeItem &sourceItem = m_gcode->at(i);
        bool nextIsArc = (i + 1 < m_gcode->count()) && m_gcode->at(i + 1).isArc();
        QList<GCodeItem> resultItems = processLine(sourceItem, nextIsArc);

        accumulatedItems.append(resultItems);
        processed++;

        if (processed % 10 == 0) {
            emit progressChanged(m_currentIndex + processed, m_gcode->count());
        }
    }

    // Replace processed lines with results
    if (!accumulatedItems.isEmpty()) {
        m_gcode->erase(m_currentIndex, m_currentIndex + processed);

        for (int i = 0; i < accumulatedItems.size(); ++i) {
            m_gcode->insert(m_currentIndex + i, accumulatedItems[i]);
        }
    }

    m_currentIndex += accumulatedItems.size();

    return processed;
}

bool ShakingGCode::hasMore() const
{
    return m_gcode && m_currentIndex < m_gcode->count();
}

int ShakingGCode::totalLines() const
{
    return m_gcode ? m_gcode->count() : 0;
}

GCode* ShakingGCode::convertAll()
{
    if (!m_gcode) {
        return nullptr;
    }

    GCode *result = new GCode();
    m_parser->reset();

    int sourceCount = m_gcode->count();
    int lastProgress = 0;

    for (int i = 0; i < sourceCount; ++i) {
        const GCodeItem &sourceItem = m_gcode->at(i);
        bool nextIsArc = (i + 1 < sourceCount) && m_gcode->at(i + 1).isArc();
        QList<GCodeItem> resultItems = processLine(sourceItem, nextIsArc);

        // Append all result items (may be 1 or many)
        for (const GCodeItem &item : resultItems) {
            *result << item;
        }

        // Emit progress
        int progress = (i * 100) / sourceCount;
        if (progress != lastProgress && progress % 5 == 0) {
            emit progressChanged(i, sourceCount);
            lastProgress = progress;
        }
    }

    emit progressChanged(sourceCount, sourceCount);

    return result;
}

QList<GCodeItem> ShakingGCode::processLine(const GCodeItem &item, bool nextIsArc)
{
    QList<GCodeItem> result;

    // Non-movement commands pass through unchanged
    if (!item.isMovement) {
        result.append(item);
        m_parser->addCommand(item);
        return result;
    }

    // Get start point (current parser position)
    QVector3D startPoint = *m_parser->getCurrentPoint();

    // Parse command to get end point
    m_parser->pushState();
    PointSegment *ps = m_parser->addCommand(item);

    if (!ps) {
        m_parser->popState();
        result.append(item);
        return result;
    }

    QVector3D endPoint = *ps->point();

    // Only handle linear movements (G0/G1) - skip arcs for simplicity
    if (ps->isArc()) {
        m_parser->popState();
        result.append(item);
        m_parser->addCommand(item);
        return result;
    }

    m_parser->popState();

    // Segment the line
    QList<QVector3D> points = segmentLine(startPoint, endPoint, nextIsArc);

    // If no segmentation needed (short line)
    if (points.size() <= 2) {
        if (points.size() == 2) {
            // Apply random offset to end point, unless the next command is an arc
            QVector3D modifiedEnd = points[1];
            if (!nextIsArc) {
                applyRandomOffset(modifiedEnd);
            }

            GCodeItem modifiedItem = item;
            modifiedItem.line = generateGCodeLine(startPoint, modifiedEnd, item, true);
            modifiedItem.args = GcodePreprocessorUtils::splitCommand(modifiedItem.line);
            result.append(modifiedItem);

            m_parser->addCommand(modifiedItem);
        } else {
            result.append(item);
            m_parser->addCommand(item);
        }
        return result;
    }

    // Multiple segments - create GCodeItem for each
    for (int i = 0; i < points.size() - 1; i++) {
        GCodeItem segmentItem;
        segmentItem.line = generateGCodeLine(points[i], points[i + 1], item, i == 0);
        segmentItem.state = GCodeItem::InQueue;
        segmentItem.args = GcodePreprocessorUtils::splitCommand(segmentItem.line);
        segmentItem.isMovement = true;
        segmentItem.group = item.group;
        segmentItem.commandNumber = item.commandNumber;

        result.append(segmentItem);
        m_parser->addCommand(segmentItem);
    }

    return result;
}

QList<QVector3D> ShakingGCode::segmentLine(const QVector3D &start, const QVector3D &end, bool nextIsArc)
{
    QList<QVector3D> points;
    points.append(start);

    double length = (end - start).length();

    // If line is short enough, no segmentation needed
    if (length <= m_segmentLength) {
        QVector3D endWithOffset = end;
        if (!nextIsArc) {
            applyRandomOffset(endWithOffset);
        }
        points.append(endWithOffset);
        return points;
    }

    // Calculate number of segments
    int numSegments = qCeil(length / m_segmentLength);
    QVector3D direction = (end - start).normalized();
    double segmentLen = length / numSegments;

    // Generate intermediate points with random offsets
    // The last point (arc start) must not be offset when the next command is an arc
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

    // Generate random offset in range [-maxOffset, +maxOffset] for each axis
    double offsetX = (m_random->generateDouble() * 2.0 - 1.0) * m_maxOffset;
    double offsetY = (m_random->generateDouble() * 2.0 - 1.0) * m_maxOffset;
    double offsetZ = (m_random->generateDouble() * 2.0 - 1.0) * m_maxOffset;

    point.setX(point.x() + offsetX);
    point.setY(point.y() + offsetY);
    point.setZ(point.z() + offsetZ);
}

double ShakingGCode::getRandomFeedRate(double originalFeedRate)
{
    if (originalFeedRate <= 0.0 || m_feedRateVariation <= 0.0) {
        return originalFeedRate;
    }

    // Generate random variation: original * (1 ± variation)
    // E.g., if variation = 0.2, result will be in range [0.8*original, 1.2*original]
    double factor = 1.0 + (m_random->generateDouble() * 2.0 - 1.0) * m_feedRateVariation;

    // Clamp to reasonable range
    factor = qMax(0.5, qMin(1.5, factor));

    return originalFeedRate * factor;
}

QString ShakingGCode::generateGCodeLine(const QVector3D &start, const QVector3D &end,
                                        const GCodeItem &originalItem, bool isFirstSegment)
{
    QString line;

    // Extract only the G/M code prefix (first token). We append fresh
    // coordinates below, so the original args would duplicate them.
    QString command;
    {
        const QString full = originalItem.command();
        const int sp = full.indexOf(' ');
        command = (sp < 0) ? full : full.left(sp);
    }
    if (command.isEmpty() && !originalItem.args.empty()) {
        command = QString::fromStdString(originalItem.args.front());
    }

    // Start with command
    line = command;

    // Add coordinates - always specify all coordinates for clarity
    line += QString(" X%1").arg(end.x(), 0, 'f', 3);
    line += QString(" Y%1").arg(end.y(), 0, 'f', 3);
    line += QString(" Z%1").arg(end.z(), 0, 'f', 3);

    // Handle feed rate
    static QRegularExpression feedRegex("F([0-9.]+)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = feedRegex.match(originalItem.line);

    if (match.hasMatch()) {
        double originalFeedRate = match.captured(1).toDouble();
        double modifiedFeedRate = getRandomFeedRate(originalFeedRate);

        // Add feed rate (always, to make each segment independently controllable)
        line += QString(" F%1").arg(modifiedFeedRate, 0, 'f', 1);
    }

    return line;
}
