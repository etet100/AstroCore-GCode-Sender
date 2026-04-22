// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "applyheightmap.h"
#include "core/heightmap/interpolator/heightmapbicubicinterpolator.h"
#include "core/heightmap/interpolator/heightmapbilinearinterpolator.h"
#include "core/heightmap/interpolator/heightmaplinearinterpolator.h"
#include "core/heightmap/interpolator/heightmapnearestneighbourinterpolator.h"
#include "core/gcode/gcode.h"
#include "core/gcode/parser/gcodeparser.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>
#include <QtMath>

ApplyHeightmap::ApplyHeightmap(Heightmap* heightmap, double segmentLength, bool applyToRapids, QObject *parent)
    : QObject(parent)
    , m_heightmap(heightmap)
    , m_interpolator(nullptr)
    , m_parser(nullptr)
    , m_gcode(nullptr)
    , m_segmentLength(segmentLength)
    , m_arcPreserveTolerance(0.01)
    , m_currentIndex(0)
    , m_applyToRapids(applyToRapids)
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
            case Heightmap::InterpolationMode::NearestNeighbour:
                m_interpolator = new HeightmapNearestNeighbourInterpolator(m_heightmap);
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

    if (!item.isMovement) {
        result.append(item);
        m_parser->addCommand(item);
        return result;
    }

    if (!m_applyToRapids && item.group == GCodeItemGroup::RapidMovement) {
        result.append(item);
        m_parser->addCommand(item);
        return result;
    }

    QVector3D startPoint = *m_parser->getCurrentPoint();

    m_parser->pushState();
    PointSegment *ps = m_parser->addCommand(item);

    if (!ps) {
        m_parser->popState();
        result.append(item);
        return result;
    }

    QVector3D endPoint = *ps->point();

    // Save arc parameters before popState() deletes ps.
    bool isArc = ps->isArc();
    QVector3D arcCenter;
    double arcRadius = 0.0;
    bool arcClockwise = false;
    PointSegment::planes arcPlane = PointSegment::XY;

    if (isArc) {
        arcCenter   = *ps->center();
        arcRadius   = ps->getRadius();
        arcClockwise = ps->isClockwise();
        arcPlane    = ps->plane();
    }

    m_parser->popState(); // deletes ps

    // For G17 arcs: if Z offsets are uniform along the arc, keep it as G2/G3.
    if (isArc && arcPlane == PointSegment::XY) {
        double avgOffset;
        if (checkArcZOffsetUniform(startPoint, endPoint, arcCenter, arcRadius, arcClockwise, avgOffset)) {
            GCodeItem preservedItem = item;
            preservedItem.line = adjustZInArcLine(item, endPoint.z(), avgOffset);
            preservedItem.args = GcodePreprocessorUtils::splitCommand(preservedItem.line);
            m_parser->addCommand(preservedItem);
            result.append(preservedItem);
            return result;
        }
    }

    // Segmentation path: arcs are linearised, their segments use G1.
    QList<QVector3D> points;
    if (isArc) {
        points = segmentArc(startPoint, endPoint, arcCenter, arcRadius, arcClockwise, arcPlane);
    } else {
        points = segmentLine(startPoint, endPoint);
    }

    QString outputCmd = isArc ? "G1" : QString();

    if (points.size() <= 2) {
        if (points.size() == 2) {
            // points[1] already has heightmap applied - do not apply again.
            GCodeItem modifiedItem = item;
            modifiedItem.line = generateGCodeLine(startPoint, points[1], item, true, outputCmd);
            modifiedItem.args = GcodePreprocessorUtils::splitCommand(modifiedItem.line);
            // Arc linearised to G1 — update group to reflect the actual command.
            if (isArc) modifiedItem.group = GCodeItemGroup::Movement;
            result.append(modifiedItem);

            m_parser->addCommand(modifiedItem);
        } else {
            result.append(item);
            m_parser->addCommand(item);
        }
        return result;
    }

    for (int i = 0; i < points.size() - 1; i++) {
        GCodeItem segmentItem;
        segmentItem.line = generateGCodeLine(points[i], points[i + 1], item, i == 0, outputCmd);
        segmentItem.state = GCodeItem::InQueue;
        segmentItem.args = GcodePreprocessorUtils::splitCommand(segmentItem.line);
        segmentItem.isMovement = true;
        // Arcs are linearised to G1 (Movement). Lines keep the original group
        // so that G0 segments (when applyToRapids=true) stay RapidMovement.
        segmentItem.group = isArc ? GCodeItemGroup::Movement : item.group;
        segmentItem.commandNumber = item.commandNumber;

        result.append(segmentItem);

        m_parser->addCommand(segmentItem);
    }

    return result;
}

QList<QVector3D> ApplyHeightmap::segmentLine(const QVector3D &start, const QVector3D &end)
{
    QList<QVector3D> points;
    points.append(start);

    double length = (end - start).length();

    if (length <= m_segmentLength) {
        QVector3D endWithHeight = end;
        applyHeightmapToPoint(endWithHeight);
        points.append(endWithHeight);
        return points;
    }

    int numSegments = qCeil(length / m_segmentLength);
    QVector3D direction = (end - start).normalized();
    double segmentLen = length / numSegments;

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

    QList<QVector3D> arcPoints = GcodePreprocessorUtils::generatePointsAlongArcBDring(
        plane, start, end, center, clockwise, radius,
        m_segmentLength, m_segmentLength, false
    );

    for (QVector3D &point : arcPoints) {
        applyHeightmapToPoint(point);
        points.append(point);
    }

    return points;
}

bool ApplyHeightmap::checkArcZOffsetUniform(
    const QVector3D &start, const QVector3D &end,
    const QVector3D &center, double radius, bool clockwise, double &avgOffset) const
{
    constexpr int NUM_SAMPLES = 8;

    double startAngle = qAtan2(start.y() - center.y(), start.x() - center.x());
    double endAngle   = qAtan2(end.y()   - center.y(), end.x()   - center.x());
    double sweep      = GcodePreprocessorUtils::calculateSweep(startAngle, endAngle, clockwise);

    QList<QVector3D> samples = GcodePreprocessorUtils::generatePointsAlongArcBDring(
        PointSegment::XY, start, end, center, clockwise, radius,
        startAngle, sweep, NUM_SAMPLES
    );

    if (samples.size() < 2) return false;

    double minOffset = qInf();
    double maxOffset = -qInf();
    double sumOffset = 0.0;
    int validCount = 0;

    for (const QVector3D &pt : samples) {
        QPointF xy(pt.x(), pt.y());
        if (!m_heightmap->isInside(xy)) continue;

        double offset = m_interpolator->interpolate(xy);
        if (qIsNaN(offset)) continue;

        minOffset = qMin(minOffset, offset);
        maxOffset = qMax(maxOffset, offset);
        sumOffset += offset;
        validCount++;
    }

    if (validCount == 0) {
        // Arc is outside heightmap bounds - no Z modification needed.
        avgOffset = 0.0;
        return true;
    }

    if ((maxOffset - minOffset) > m_arcPreserveTolerance) return false;

    avgOffset = sumOffset / validCount;
    return true;
}

QString ApplyHeightmap::adjustZInArcLine(const GCodeItem &item, double originalEndZ, double zOffset) const
{
    // Non-helical arcs have no Z in the line. Since start Z == end Z,
    // a uniform offset keeps them equal - no Z parameter needed.
    static QRegularExpression zRegex("Z[+-]?[0-9]*\\.?[0-9]+",
                                     QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = zRegex.match(item.line);

    if (!match.hasMatch()) return item.line;

    QString newZStr = QString("Z%1").arg(originalEndZ + zOffset, 0, 'f', 3);
    QString result = item.line;
    result.replace(match.capturedStart(), match.capturedLength(), newZStr);
    return result;
}

void ApplyHeightmap::applyHeightmapToPoint(QVector3D &point)
{
    if (!m_interpolator || !m_heightmap) {
        return;
    }

    QPointF xyPoint(point.x(), point.y());
    if (!m_heightmap->isInside(xyPoint)) {
        return;
    }

    double zOffset = m_interpolator->interpolate(xyPoint);
    if (!qIsNaN(zOffset)) {
        point.setZ(point.z() + zOffset);
    }
}

QString ApplyHeightmap::generateGCodeLine(const QVector3D &start, const QVector3D &end,
                                          const GCodeItem &originalItem, bool isFirstSegment,
                                          const QString &commandOverride)
{
    QString command;
    if (!commandOverride.isEmpty()) {
        command = commandOverride;
    } else {
        // Take only the G/M code prefix (first token) because later we append
        // fresh X/Y/Z/F values; the original args would duplicate coordinates.
        const QString full = originalItem.command();
        const int sp = full.indexOf(' ');
        command = (sp < 0) ? full : full.left(sp);
    }
    if (command.isEmpty() && !originalItem.args.empty()) {
        command = QString::fromStdString(originalItem.args.front());
    }

    QString line = command;

    if (qAbs(end.x() - start.x()) > 0.0001) line += QString(" X%1").arg(end.x(), 0, 'f', 3);
    if (qAbs(end.y() - start.y()) > 0.0001) line += QString(" Y%1").arg(end.y(), 0, 'f', 3);
    if (qAbs(end.z() - start.z()) > 0.0001) line += QString(" Z%1").arg(end.z(), 0, 'f', 3);

    if (isFirstSegment) {
        static QRegularExpression feedRegex("F([0-9.]+)", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = feedRegex.match(originalItem.line);
        if (match.hasMatch()) {
            line += " " + match.captured(0);
        }
    }

    return line;
}
