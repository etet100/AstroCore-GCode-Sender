// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef HEIGHTMAPBICUBICINTERPOLATOR_H
#define HEIGHTMAPBICUBICINTERPOLATOR_H

#include <QPointF>
#include "heightmapinterpolator.h"

// Class for bicubic interpolation of heightmap data points. Points are provided as a grid (Heightmap class).
// For points near the edges of the grid (going outside the grid), edge points are duplicated.
class HeightmapBicubicInterpolator : public HeightmapInterpolator
{
    public:
        HeightmapBicubicInterpolator(const Heightmap& heightmap);
        double interpolate(QPointF point) const override;

    private:
        double cubicInterpolate(double p0, double p1, double p2, double p3, double t) const;
        double getValueAtClampedPoint(int x, int y) const;
};

#endif // HEIGHTMAPBICUBICINTERPOLATOR_H
