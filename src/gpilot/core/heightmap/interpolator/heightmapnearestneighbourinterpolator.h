// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#ifndef HEIGHTMAPNEARESTNEIGHBOURINTERPOLATOR_H
#define HEIGHTMAPNEARESTNEIGHBOURINTERPOLATOR_H

#include <QPointF>
#include "heightmapinterpolator.h"

// Class for nearest neighbour interpolation of heightmap data points.
// Returns the value at the closest grid point without any interpolation.
class HeightmapNearestNeighbourInterpolator : public HeightmapInterpolator
{
    public:
        HeightmapNearestNeighbourInterpolator(const Heightmap* heightmap);
        double interpolate(QPointF point) const override;
};

#endif // HEIGHTMAPNEARESTNEIGHBOURINTERPOLATOR_H
