// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#include "heightmapnearestneighbourinterpolator.h"
#include <cmath>

HeightmapNearestNeighbourInterpolator::HeightmapNearestNeighbourInterpolator(const Heightmap* heightmap)
    : HeightmapInterpolator(heightmap)
{
}

double HeightmapNearestNeighbourInterpolator::interpolate(QPointF point) const
{
    auto [x, y] = point;

    return m_heightmap->at(
        static_cast<int>(std::round(x)),
        static_cast<int>(std::round(y))
    );
}
