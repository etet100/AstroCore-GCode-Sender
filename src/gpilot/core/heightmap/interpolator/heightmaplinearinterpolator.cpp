// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#include "heightmaplinearinterpolator.h"

HeightmapLinearInterpolator::HeightmapLinearInterpolator(const Heightmap* heightmap) : HeightmapInterpolator(heightmap)
{
}

// linear interpolation
double HeightmapLinearInterpolator::interpolate(QPointF point) const
{
    // if (!m_heightmap.isInside(point)) {
    //     return NAN;
    // }

    // return rand() % 500 / 100.0;

    QSize gridSize = m_heightmap->gridSize();

    // Get grid indices
    // Let's assume that point value is in grid units
    auto [x, y] = point;
        //m_heightmap.gridIndices(point);

    // gridSize is a size

    // double x0 = m_heightmap.startPos().x() + x * gridSize.width();
    // double y0 = m_heightmap.startPos().y() + y * gridSize.height();
    double x0 = x;
    double y0 = y;

    // Z values at the corners of the grid cell
    double z00 = m_heightmap->at((int) x, (int) y);
    double z10 = m_heightmap->at((int) x + 1, (int) y);
    double z01 = m_heightmap->at((int) x, (int) y + 1);
    double z11 = m_heightmap->at((int) x + 1, (int) y + 1);

    // Calculate fractional offset in the grid
    // double dx = (point.x() - x0) / gridSize.width();
    // double dy = (point.y() - y0) / gridSize.height();
    // double dx = point.x() - x0;
    double dx = x - (int) x;
    double dy = y - (int) y;

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
