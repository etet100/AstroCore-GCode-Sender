// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#ifndef HEIGHTMAPVIEWTRANSFORM_H
#define HEIGHTMAPVIEWTRANSFORM_H

#include "abstractviewtransform.h"
#include "core/heightmap/heightmap.h"

class AbstractHeightmapInterpolator;

class HeightmapViewTransform : public AbstractViewTransform
{
    public:
        HeightmapViewTransform(Heightmap* heightmap, double segmentLength);
        ~HeightmapViewTransform() override;

        HeightmapViewTransform(const HeightmapViewTransform&) = delete;
        HeightmapViewTransform& operator=(const HeightmapViewTransform&) = delete;

        QList<LineSegment> apply(const QList<LineSegment>& input) const override;

        double segmentLength() const { return m_segmentLength; }
        void setSegmentLength(double length) { m_segmentLength = length; }

        void updateInterpolator();

    private:
        Heightmap* m_heightmap;
        Heightmap::InterpolationMode m_interpolatorMode;
        AbstractHeightmapInterpolator* m_interpolator = nullptr;
        double m_segmentLength;

        double zOffsetAt(const QPointF& xy) const;
        LineSegment makeSegmentLike(const LineSegment& source,
                                    const QVector3D& start,
                                    const QVector3D& end) const;
};

#endif // HEIGHTMAPVIEWTRANSFORM_H
