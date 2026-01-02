// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "heightmapbicubicinterpolator.h"
#include <algorithm>

HeightmapBicubicInterpolator::HeightmapBicubicInterpolator(const Heightmap& heightmap) : HeightmapInterpolator(heightmap)
{
}

double HeightmapBicubicInterpolator::getValueAtClampedPoint(int x, int y) const
{
    int clampedX = std::clamp(x, 0, m_heightmap.gridWidth() - 1);
    int clampedY = std::clamp(y, 0, m_heightmap.gridHeight() - 1);

    return m_heightmap.at(clampedX, clampedY);
}

double HeightmapBicubicInterpolator::cubicInterpolate(double p0, double p1, double p2, double p3, double t) const
{
    // Catmull-Rom spline interpolation
    double a0 = -0.5 * p0 + 1.5 * p1 - 1.5 * p2 + 0.5 * p3;
    double a1 = p0 - 2.5 * p1 + 2.0 * p2 - 0.5 * p3;
    double a2 = -0.5 * p0 + 0.5 * p2;
    double a3 = p1;

    return a0 * t * t * t + a1 * t * t + a2 * t + a3;
}

double HeightmapBicubicInterpolator::interpolate(QPointF ptMm) const
{
    auto [x, y] = ptMm;

    // Get integer grid coordinates
    int xi = static_cast<int>(x);
    int yi = static_cast<int>(y);

    // Calculate fractional offset in the grid
    double dx = x - xi;
    double dy = y - yi;

    // Get 16 points (4x4 grid) around the point
    // For points near edges, getValueClamped will duplicate edge points
    double values[4][4];

    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            values[j][i] = getValueAtClampedPoint(xi + i - 1, yi + j - 1);
        }
    }

    // Interpolate along x for each row
    double col[4];
    for (int j = 0; j < 4; j++) {
        col[j] = cubicInterpolate(values[j][0], values[j][1], values[j][2], values[j][3], dx);
    }

    // Interpolate along y using the interpolated x values
    double z = cubicInterpolate(col[0], col[1], col[2], col[3], dy);

    return z;
}
