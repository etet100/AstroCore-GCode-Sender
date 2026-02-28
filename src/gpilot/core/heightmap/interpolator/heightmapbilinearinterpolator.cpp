// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#include "heightmapbilinearinterpolator.h"
#include <algorithm>
#include <cmath>

HeightmapBilinearInterpolator::HeightmapBilinearInterpolator(const Heightmap* heightmap) : HeightmapInterpolator(heightmap)
{
}

double HeightmapBilinearInterpolator::interpolate(QPointF ptMm) const
{
    auto [x, y] = ptMm;

    // Clamp coordinates to valid grid range
    int xi = static_cast<int>(x);
    int yi = static_cast<int>(y);
    int xi1 = std::min(xi + 1, m_heightmap->gridWidth() - 1);
    int yi1 = std::min(yi + 1, m_heightmap->gridHeight() - 1);

    xi = std::clamp(xi, 0, m_heightmap->gridWidth() - 1);
    yi = std::clamp(yi, 0, m_heightmap->gridHeight() - 1);

    // Z values at the corners of the grid cell
    double z00 = m_heightmap->at(xi, yi);
    double z10 = m_heightmap->at(xi1, yi);
    double z01 = m_heightmap->at(xi, yi1);
    double z11 = m_heightmap->at(xi1, yi1);

    // Calculate fractional offset in the grid
    double dx = x - static_cast<double>(xi);
    double dy = y - static_cast<double>(yi);

    // Clamp fractional parts to [0, 1]
    dx = std::clamp(dx, 0.0, 1.0);
    dy = std::clamp(dy, 0.0, 1.0);

    // Bilinear interpolation
    double z0 = z00 * (1.0 - dx) + z10 * dx;
    double z1 = z01 * (1.0 - dx) + z11 * dx;
    double z = z0 * (1.0 - dy) + z1 * dy;

    return z;
}
