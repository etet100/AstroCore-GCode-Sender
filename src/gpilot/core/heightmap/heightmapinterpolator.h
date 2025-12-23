// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef HEIGHTMAPINTERPOLATOR_H
#define HEIGHTMAPINTERPOLATOR_H

#include <QPointF>
#include "heightmap.h"

// Class for bilinear interpolation of heightmap data points. Points are provided as a grid (Heightmap class).
class HeightmapInterpolator
{
    public:
        HeightmapInterpolator(const Heightmap& heightmap);
        // Get interpolated height at the given (x, y) coordinates.
        double interpolate(QPointF point) const;

    private:
        const Heightmap& m_heightmap;
};

#endif // HEIGHTMAPINTERPOLATOR_H
