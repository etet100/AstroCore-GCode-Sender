// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#include "heightmapviewtransform.h"
#include "core/heightmap/interpolator/heightmapbicubicinterpolator.h"
#include "core/heightmap/interpolator/heightmapbilinearinterpolator.h"
#include "core/heightmap/interpolator/heightmaplinearinterpolator.h"
#include "core/heightmap/interpolator/heightmapnearestneighbourinterpolator.h"
#include <QtMath>

void HeightmapViewTransform::updateInterpolator()
{
    if (m_interpolator != nullptr) {
        delete m_interpolator;
    } else if (!m_heightmap) {
        return;
    }

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
    m_interpolatorMode = m_heightmap->interpolationMode();
}

HeightmapViewTransform::HeightmapViewTransform(Heightmap* heightmap, double segmentLength)
    : m_heightmap(heightmap)
    , m_interpolator(nullptr)
    , m_segmentLength(segmentLength)
{
    if (!m_heightmap) {
        return;
    }

    updateInterpolator();
}

HeightmapViewTransform::~HeightmapViewTransform()
{
    delete m_interpolator;
}

QList<LineSegment> HeightmapViewTransform::apply(const QList<LineSegment>& input) const
{
    if (!m_heightmap || !m_interpolator || m_segmentLength <= 0.0) {
        return input;
    }

    QList<LineSegment> result;
    result.reserve(input.size());

    for (const LineSegment& src : input) {
        const QVector3D& start = src.getStart();
        const QVector3D& end = src.getEnd();

        const double length = (end - start).length();

        if (length <= m_segmentLength || qIsNaN(length) || length == 0.0) {
            QVector3D newStart = start;
            QVector3D newEnd = end;
            newStart.setZ(newStart.z() + zOffsetAt(QPointF(newStart.x(), newStart.y())));
            newEnd.setZ(newEnd.z() + zOffsetAt(QPointF(newEnd.x(), newEnd.y())));
            result.append(makeSegmentLike(src, newStart, newEnd));
            continue;
        }

        const int numSegments = qCeil(length / m_segmentLength);
        const QVector3D step = (end - start) / static_cast<float>(numSegments);

        QVector3D prev = start;
        prev.setZ(prev.z() + zOffsetAt(QPointF(prev.x(), prev.y())));

        for (int i = 1; i <= numSegments; i++) {
            QVector3D next = (i == numSegments) ? end : start + step * static_cast<float>(i);
            next.setZ(next.z() + zOffsetAt(QPointF(next.x(), next.y())));
            result.append(makeSegmentLike(src, prev, next));
            prev = next;
        }
    }

    return result;
}

double HeightmapViewTransform::zOffsetAt(const QPointF& xy) const
{
    if (!m_heightmap->isInside(xy)) {
        return 0.0;
    }
    const double offset = m_interpolator->interpolate(xy);

    return qIsNaN(offset) ? 0.0 : offset;
}

LineSegment HeightmapViewTransform::makeSegmentLike(const LineSegment& source,
                                                    const QVector3D& start,
                                                    const QVector3D& end) const
{
    LineSegment ls(start, end, source.getLineNumber());
    ls.setIsArc(source.isArc());
    ls.setIsClockwise(source.isClockwise());
    ls.setPlane(source.plane());
    ls.setIsFastTraverse(source.isFastTraverse());
    ls.setIsZMovement(source.isZMovement());
    ls.setIsMetric(source.isMetric());
    ls.setIsAbsolute(source.isAbsolute());
    ls.setSpeed(source.getSpeed());
    ls.setSpindleSpeed(source.getSpindleSpeed());
    ls.setDwell(source.getDwell());

    return ls;
}
