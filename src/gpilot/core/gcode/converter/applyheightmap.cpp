// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "applyheightmap.h"
#include "core/heightmap/interpolator/heightmapbicubicinterpolator.h"
#include "core/heightmap/interpolator/heightmapbilinearinterpolator.h"
#include "core/heightmap/interpolator/heightmaplinearinterpolator.h"
#include "core/gcode/gcode.h"
#include "core/gcode/parser/gcodeparser.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>
#include <QtMath>

// This converter applies a heightmap to the G-code commands.
// - Splits long movement lines into segments (default 1mm)
// - Applies Z offset from heightmap at each segment point
// - Handles both linear movements (G0/G1) and arcs (G2/G3)
// - Preserves G90/G91 mode and other parser state
//
// Implementation note: This converter creates new lines during processing,
// so it implements ConverterInterface directly rather than using Pipeline.

ApplyHeightmap::ApplyHeightmap(Heightmap* heightmap, double segmentLength, QObject *parent)
    : QObject(parent)
    , m_heightmap(heightmap)
    , m_interpolator(nullptr)
    , m_parser(nullptr)
    , m_gcode(nullptr)
    , m_segmentLength(segmentLength)
    , m_currentIndex(0)
{
    if (m_heightmap) {
        // Use interpolation mode from heightmap
        switch (m_heightmap->interpolationMode()) {
            case Heightmap::InterpolationMode::Bicubic:
                m_interpolator = new HeightmapBicubicInterpolator(m_heightmap);
                break;
            case Heightmap::InterpolationMode::Bilinear:
                m_interpolator = new HeightmapBilinearInterpolator(m_heightmap);
                break;
            case Heightmap::InterpolationMode::Linear:
                m_interpolator = new HeightmapLinearInterpolator(m_heightmap);
                break;
            default:
                m_interpolator = new HeightmapBicubicInterpolator(m_heightmap);
                break;
        }
    }
}

ApplyHeightmap::~ApplyHeightmap()
{
    delete m_interpolator;
    delete m_parser;
}

void ApplyHeightmap::setGCode(GCode *gcode)
{
    m_gcode = gcode;
    reset();
}

void ApplyHeightmap::reset()
{
    m_currentIndex = 0;

    if (!m_parser) {
        m_parser = new GcodeParser();
    }
    m_parser->reset();
}

int ApplyHeightmap::convertNext(int count)
{
    if (!m_gcode || !m_heightmap || !m_interpolator) {
        return 0;
    }

    // WARNING: Pull mode with line insertion is problematic.
    // This implementation processes lines but doesn't handle
    // the growing GCode list properly. Use convertAll() instead.

    int processed = 0;
    int targetCount = qMin(m_currentIndex + count, m_gcode->count());

    QList<GCodeItem> accumulatedItems;

    for (int i = m_currentIndex; i < targetCount; ++i) {
        const GCodeItem &sourceItem = m_gcode->at(i);
        QList<GCodeItem> resultItems = processLine(sourceItem);

        accumulatedItems.append(resultItems);
        processed++;

        if (processed % 10 == 0) {
            emit progressChanged(m_currentIndex + processed, m_gcode->count());
        }
    }

    // Replace processed lines with results
    // This is inefficient but necessary for pull mode
    if (!accumulatedItems.isEmpty()) {
        m_gcode->erase(m_currentIndex, m_currentIndex + processed);

        for (int i = 0; i < accumulatedItems.size(); ++i) {
            m_gcode->insert(m_currentIndex + i, accumulatedItems[i]);
        }
    }

    m_currentIndex += accumulatedItems.size();

    return processed;
}

bool ApplyHeightmap::hasMore() const
{
    return m_gcode && m_currentIndex < m_gcode->count();
}

int ApplyHeightmap::totalLines() const
{
    return m_gcode ? m_gcode->count() : 0;
}

GCode* ApplyHeightmap::convertAll()
{
    if (!m_gcode || !m_heightmap || !m_interpolator) {
        return nullptr;
    }

    GCode *result = new GCode();
    m_parser->reset();

    int sourceCount = m_gcode->count();
    int lastProgress = 0;

    for (int i = 0; i < sourceCount; ++i) {
        const GCodeItem &sourceItem = m_gcode->at(i);
        QList<GCodeItem> resultItems = processLine(sourceItem);

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

QList<GCodeItem> ApplyHeightmap::processLine(const GCodeItem &item)
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

    // Parse command to get end point and segment info
    m_parser->pushState();
    PointSegment *ps = m_parser->addCommand(item);

    if (!ps) {
        m_parser->popState();
        result.append(item);
        return result;
    }

    QVector3D endPoint = *ps->point();

    // Generate segmented points with heightmap applied
    QList<QVector3D> points;

    if (ps->isArc()) {
        // Handle arc (G2/G3)
        points = segmentArc(startPoint, endPoint, *ps->center(),
                           ps->getRadius(), ps->isClockwise(), ps->plane());
    } else {
        // Handle linear movement (G0/G1)
        points = segmentLine(startPoint, endPoint);
    }

    m_parser->popState();

    // If only 2 points (start and end), no segmentation needed
    if (points.size() <= 2) {
        if (points.size() == 2) {
            QVector3D modifiedEnd = points[1];
            applyHeightmapToPoint(modifiedEnd);

            GCodeItem modifiedItem = item;
            modifiedItem.line = generateGCodeLine(startPoint, modifiedEnd, item, true);
            modifiedItem.args = GcodePreprocessorUtils::splitCommand(modifiedItem.line);
            result.append(modifiedItem);

            // Update parser with the actual processed line
            m_parser->addCommand(modifiedItem);
        } else {
            result.append(item);
            m_parser->addCommand(item);
        }
        return result;
    }

    // Multiple segments needed - create one GCodeItem per segment
    for (int i = 0; i < points.size() - 1; i++) {
        GCodeItem segmentItem;
        segmentItem.line = generateGCodeLine(points[i], points[i + 1], item, i == 0);
        segmentItem.command = item.command;
        segmentItem.state = GCodeItem::InQueue;
        segmentItem.args = GcodePreprocessorUtils::splitCommand(segmentItem.line);
        segmentItem.isMovement = true;
        segmentItem.group = item.group;
        segmentItem.commandNumber = item.commandNumber;

        result.append(segmentItem);

        // Update parser state
        m_parser->addCommand(segmentItem);
    }

    return result;
}

QList<QVector3D> ApplyHeightmap::segmentLine(const QVector3D &start, const QVector3D &end)
{
    QList<QVector3D> points;
    points.append(start);

    double length = (end - start).length();

    // If line is short enough, no segmentation needed
    if (length <= m_segmentLength) {
        QVector3D endWithHeight = end;
        applyHeightmapToPoint(endWithHeight);
        points.append(endWithHeight);
        return points;
    }

    // Calculate number of segments
    int numSegments = qCeil(length / m_segmentLength);
    QVector3D direction = (end - start).normalized();
    double segmentLen = length / numSegments;

    // Generate intermediate points
    for (int i = 1; i <= numSegments; i++) {
        QVector3D point = start + direction * (segmentLen * i);
        applyHeightmapToPoint(point);
        points.append(point);
    }

    return points;
}

QList<QVector3D> ApplyHeightmap::segmentArc(const QVector3D &start, const QVector3D &end,
                                            const QVector3D &center, double radius,
                                            bool clockwise, PointSegment::planes plane)
{
    QList<QVector3D> points;

    // Use existing arc generation utility
    QList<QVector3D> arcPoints = GcodePreprocessorUtils::generatePointsAlongArcBDring(
        plane, start, end, center, clockwise, radius,
        m_segmentLength, // minArcLength
        m_segmentLength, // arcPrecision
        false            // arcDegreeMode
    );

    // Apply heightmap to all points
    for (QVector3D &point : arcPoints) {
        applyHeightmapToPoint(point);
        points.append(point);
    }

    return points;
}

void ApplyHeightmap::applyHeightmapToPoint(QVector3D &point)
{
    if (!m_interpolator || !m_heightmap) {
        return;
    }

    // Check if point is within heightmap area
    QPointF xyPoint(point.x(), point.y());
    if (!m_heightmap->isInside(xyPoint)) {
        return;
    }

    // Get Z offset from heightmap
    double zOffset = m_interpolator->interpolate(xyPoint);

    if (!qIsNaN(zOffset)) {
        point.setZ(point.z() + zOffset);
    }
}

QString ApplyHeightmap::generateGCodeLine(const QVector3D &start, const QVector3D &end,
                                          const GCodeItem &originalItem, bool isFirstSegment)
{
    QString line;

    // Extract command (G0, G1, etc.)
    QString command = originalItem.command;
    if (command.isEmpty() && !originalItem.args.isEmpty()) {
        command = originalItem.args.first();
    }

    // Start with command
    line = command;

    // Add coordinates - only specify coordinates that changed
    if (qAbs(end.x() - start.x()) > 0.0001) {
        line += QString(" X%1").arg(end.x(), 0, 'f', 3);
    }
    if (qAbs(end.y() - start.y()) > 0.0001) {
        line += QString(" Y%1").arg(end.y(), 0, 'f', 3);
    }
    if (qAbs(end.z() - start.z()) > 0.0001) {
        line += QString(" Z%1").arg(end.z(), 0, 'f', 3);
    }

    // For first segment, preserve feed rate if present
    if (isFirstSegment) {
        static QRegularExpression feedRegex("F([0-9.]+)", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = feedRegex.match(originalItem.line);
        if (match.hasMatch()) {
            line += " " + match.captured(0);
        }
    }

    return line;
}
