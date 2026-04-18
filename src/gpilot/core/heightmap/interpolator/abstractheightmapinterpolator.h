// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#ifndef ABSTRACTHEIGHTMAPINTERPOLATOR_H
#define ABSTRACTHEIGHTMAPINTERPOLATOR_H

#include <QPointF>
#include "../heightmap.h"

class AbstractHeightmapInterpolator
{
    public:
        AbstractHeightmapInterpolator(const Heightmap* heightmap);
        virtual ~AbstractHeightmapInterpolator() = default;

        // Get interpolated height at the given (x, y) coordinates.
        virtual double interpolate(QPointF point) const = 0;

    protected:
        const Heightmap* m_heightmap;
};

#endif // ABSTRACTHEIGHTMAPINTERPOLATOR_H
