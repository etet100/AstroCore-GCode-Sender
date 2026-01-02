// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef HEIGHTMAPBILINEARINTERPOLATOR_H
#define HEIGHTMAPBILINEARINTERPOLATOR_H

#include <QPointF>
#include "heightmapinterpolator.h"

// Class for bilinear interpolation of heightmap data points. Points are provided as a grid (Heightmap class).
class HeightmapBilinearInterpolator : public HeightmapInterpolator
{
    public:
        HeightmapBilinearInterpolator(const Heightmap& heightmap);
        double interpolate(QPointF point) const override;
};

#endif // HEIGHTMAPBILINEARINTERPOLATOR_H
