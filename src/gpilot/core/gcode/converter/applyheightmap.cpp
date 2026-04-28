// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "applyheightmap.h"
#include "core/heightmap/interpolator/heightmapbicubicinterpolator.h"
#include "core/heightmap/interpolator/heightmapbilinearinterpolator.h"
#include "core/heightmap/interpolator/heightmaplinearinterpolator.h"
#include "core/heightmap/interpolator/heightmapnearestneighbourinterpolator.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>
#include <QtMath>

QString ApplyHeightmap::parameterSchema()
{
    return QStringLiteral(R"JSON({
  "title": "Apply heightmap",
  "description": "Segments movement lines and shifts their Z by the height sampled from the probed heightmap. Use to compensate for an uneven work surface (e.g. PCB isolation).",
  "image": ":/images/converters/applyheightmap.png",
  "fields": [
    {
      "name": "segmentLength",
      "label": "Segment length",
      "type": "float",
      "min": 0.1,
      "max": 100.0,
      "default": 1.0,
      "description": "Movement lines are split every N mm before heightmap Z-offset is applied. Smaller values follow the surface more accurately, but produce more lines. Units: mm."
    },
    {
      "name": "applyToRapids",
      "label": "Apply to rapids",
      "type": "bool",
      "default": false,
      "description": "When on, heightmap Z-offset is applied also to G0 rapid moves. Usually left off so rapids stay at clearance height."
    }
  ]
})JSON");
}

ApplyHeightmap::ApplyHeightmap(Heightmap* heightmap, double segmentLength, bool applyToRapids)
    : m_heightmap(heightmap)
    , m_interpolator(nullptr)
    , m_parser(new GcodeParser())
    , m_segmentLength(segmentLength)
    , m_arcPreserveTolerance(0.01)
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

void ApplyHeightmap::reset()
{
    m_parser->reset();
}

QList<GCodeItem> ApplyHeightmap::push(const GCodeItem &input)
{
    if (!m_heightmap || !m_interpolator) {
        return { input };
    }

    if (!input.isMovement) {
        m_parser->addCommand(input);

        return { input };
    }

    if (!m_applyToRapids && input.group == GCodeItemGroup::RapidMovement) {
        m_parser->addCommand(input);

        return { input };
    }

    QVector3D startPoint = *m_parser->getCurrentPoint();

    m_parser->pushState();
    PointSegment *ps = m_parser->addCommand(input);

    if (!ps) {
        m_parser->popState();

        return { input };
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

    m_parser->popState();

    // For G17 arcs: if Z offsets are uniform along the arc, keep it as G2/G3.
    if (isArc && arcPlane == PointSegment::XY) {
        double avgOffset;
        if (checkArcZOffsetUniform(startPoint, endPoint, arcCenter, arcRadius, arcClockwise, avgOffset)) {
            GCodeItem preserved = input;
            preserved.line = adjustZInArcLine(input, endPoint.z(), avgOffset);
            preserved.args = GcodePreprocessorUtils::splitCommand(preserved.line);
            // Track raw G-code position (without heightmap offset) so the next
            // line's start Z is not already shifted, preventing double-application.
            m_parser->addCommand(input);

            return { preserved };
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
        QList<GCodeItem> out;
        if (points.size() == 2) {
            // points[1] already has heightmap applied — do not apply again.
            GCodeItem modified = input;
            modified.line = generateGCodeLine(startPoint, points[1], input, true, outputCmd);
            modified.args = GcodePreprocessorUtils::splitCommand(modified.line);
            // Arc linearised to G1 — update group to reflect the actual command.
            if (isArc) modified.group = GCodeItemGroup::Movement;
            out << modified;
        } else {
            out << input;
        }
        // Track raw G-code position (without heightmap offset) so the next
        // line's start Z is not already shifted, preventing double-application.
        m_parser->addCommand(input);

        return out;
    }

    QList<GCodeItem> out;
    out.reserve(points.size() - 1);
    for (int i = 0; i < points.size() - 1; i++) {
        GCodeItem segment;
        segment.line = generateGCodeLine(points[i], points[i + 1], input, i == 0, outputCmd);
        segment.state = GCodeItem::InQueue;
        segment.args = GcodePreprocessorUtils::splitCommand(segment.line);
        segment.isMovement = true;
        // Arcs are linearised to G1 (Movement). Lines keep the original group
        // so that G0 segments (when applyToRapids=true) stay RapidMovement.
        segment.group = isArc ? GCodeItemGroup::Movement : input.group;
        segment.commandNumber = input.commandNumber;
        segment.overlayId     = input.overlayId;
        out << segment;
    }
    // Track raw G-code position (without heightmap offset) so the next
    // line's start Z is not already shifted, preventing double-application.
    m_parser->addCommand(input);

    return out;
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
    // Include the raw start so the loop in push() generates the first chord
    // (startPoint → arc[0]) correctly, matching segmentLine's convention.
    // generatePointsAlongArcBDring does NOT include the start point.
    points.append(start);

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
        // Always emit an explicit motion code so that the machine's modal state
        // from a preceding G2/G3 (preserved arc) does not carry over into what
        // should be a G1/G0 move. A modal G1 line ("X10 Y5" without a G-word)
        // would otherwise be interpreted as G2/G3 by the controller.
        switch (originalItem.group) {
            case GCodeItemGroup::Movement:      command = "G1"; break;
            case GCodeItemGroup::RapidMovement: command = "G0"; break;
            default: {
                const QString full = originalItem.command();
                const int sp = full.indexOf(' ');
                command = (sp < 0) ? full : full.left(sp);
                break;
            }
        }
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
