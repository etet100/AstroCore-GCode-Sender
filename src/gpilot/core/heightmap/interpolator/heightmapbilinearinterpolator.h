// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#ifndef HEIGHTMAPBILINEARINTERPOLATOR_H
#define HEIGHTMAPBILINEARINTERPOLATOR_H

#include <QPointF>
#include "abstractheightmapinterpolator.h"

// Class for bilinear interpolation of heightmap data points. Points are provided as a grid (Heightmap class).
class HeightmapBilinearInterpolator : public AbstractHeightmapInterpolator
{
    public:
        HeightmapBilinearInterpolator(const Heightmap* heightmap);
        double interpolate(QPointF point) const override;
};

#endif // HEIGHTMAPBILINEARINTERPOLATOR_H
