// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#include "heightmaplinearinterpolator.h"
#include <algorithm>

HeightmapLinearInterpolator::HeightmapLinearInterpolator(const Heightmap* heightmap) : AbstractHeightmapInterpolator(heightmap)
{
}

double HeightmapLinearInterpolator::interpolate(QPointF point) const
{
    auto [x, y] = point;

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

    // Divide rectangle into two triangles along diagonal (1,0)-(0,1)
    // This ensures proper interpolation when 4 points don't lie on same plane
    if (dx + dy <= 1.0) {
        // Triangle 1: (0,0), (1,0), (0,1) with values z00, z10, z01
        // Barycentric interpolation
        double w0 = 1.0 - dx - dy;
        double w1 = dx;
        double w2 = dy;

        return z00 * w0 + z10 * w1 + z01 * w2;
    } else {
        // Triangle 2: (1,0), (0,1), (1,1) with values z10, z01, z11
        // Barycentric interpolation
        double w0 = 1.0 - dy;
        double w1 = 1.0 - dx;
        double w2 = dx + dy - 1.0;

        return z10 * w0 + z01 * w1 + z11 * w2;
    }
}
