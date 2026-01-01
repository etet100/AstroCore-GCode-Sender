// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "heightmapinterpolator.h"

HeightmapInterpolator::HeightmapInterpolator(const Heightmap& heightmap) : m_heightmap(heightmap)
{
}

double HeightmapInterpolator::interpolate(QPointF ptMm) const
{
    if (!m_heightmap.isInside(ptMm)) {
        return NAN;
    }

    QSize gridSize = m_heightmap.gridSize();

    // Take physical coordinates
    auto [x, y] = m_heightmap.gridIndices(ptMm);

    double x0 = m_heightmap.startPos().x() + x * gridSize.width();
    double y0 = m_heightmap.startPos().y() + y * gridSize.height();

    // Z values at the corners of the grid cell
    double z00 = m_heightmap.at(x, y);
    double z10 = m_heightmap.at(x+1, y);
    double z01 = m_heightmap.at(x, y+1);
    double z11 = m_heightmap.at(x+1, y+1);

    // Calcualte fractional offset in the grid
    double dx = (ptMm.x() - x0) / gridSize.width();
    double dy = (ptMm.y() - y0) / gridSize.height();

    // Bilinear interpolation
    double z0 = z00 * (1 - dx) + z10 * dx;
    double z1 = z01 * (1 - dx) + z11 * dx;
    double z = z0 * (1 - dy) + z1 * dy;

    return z;
}
