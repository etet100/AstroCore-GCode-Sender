// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#include "heightmapnearestneighbourinterpolator.h"
#include <cmath>
#include <algorithm>

HeightmapNearestNeighbourInterpolator::HeightmapNearestNeighbourInterpolator(const Heightmap* heightmap)
    : HeightmapInterpolator(heightmap)
{
}

double HeightmapNearestNeighbourInterpolator::interpolate(QPointF point) const
{
    auto [x, y] = point;

    // Round to nearest integer and clamp to valid grid range
    int xi = std::clamp(
        static_cast<int>(std::round(x)),
        0,
        m_heightmap->gridWidth() - 1
    );

    int yi = std::clamp(
        static_cast<int>(std::round(y)),
        0,
        m_heightmap->gridHeight() - 1
    );

    return m_heightmap->at(xi, yi);
}
