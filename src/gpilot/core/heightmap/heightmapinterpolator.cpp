// This file is a part of "G-Pilot (formerly Candle)" application.
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
    auto [i, j] = m_heightmap.gridIndices(ptMm);

    double x0 = m_heightmap.startPos().x() + i * gridSize.width();
    double y0 = m_heightmap.startPos().y() + j * gridSize.height();

    // Z values at the corners of the grid cell
    double z00 = m_heightmap.valueAt(QPoint(i, j));
    double z10 = m_heightmap.valueAt(QPoint(i+1, j));
    double z01 = m_heightmap.valueAt(QPoint(i, j+1));
    double z11 = m_heightmap.valueAt(QPoint(i+1, j+1));

    // Calcualte fractional offset in the grid
    double dx = (ptMm.x() - x0) / gridSize.width();
    double dy = (ptMm.y() - y0) / gridSize.height();

    // Bilinear interpolation
    double z0 = z00 * (1 - dx) + z10 * dx;
    double z1 = z01 * (1 - dx) + z11 * dx;
    double z = z0 * (1 - dy) + z1 * dy;

    return z;
}
