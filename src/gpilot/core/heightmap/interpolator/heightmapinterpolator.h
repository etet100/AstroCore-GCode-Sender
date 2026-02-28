// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#ifndef HEIGHTMAPINTERPOLATOR_H
#define HEIGHTMAPINTERPOLATOR_H

#include <QPointF>
#include "../heightmap.h"

class HeightmapInterpolator
{
    public:
        HeightmapInterpolator(const Heightmap* heightmap);
        virtual ~HeightmapInterpolator() = default;

        // Get interpolated height at the given (x, y) coordinates.
        virtual double interpolate(QPointF point) const = 0;

    protected:
        const Heightmap* m_heightmap;
};

#endif // HEIGHTMAPINTERPOLATOR_H
