// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#ifndef HEIGHTMAPLINEARINTERPOLATOR_H
#define HEIGHTMAPLINEARINTERPOLATOR_H

#include "abstractheightmapinterpolator.h"

class HeightmapLinearInterpolator : public AbstractHeightmapInterpolator
{
    public:
        HeightmapLinearInterpolator(const Heightmap* heightmap);
        double interpolate(QPointF point) const override;
};

#endif // HEIGHTMAPLINEARINTERPOLATOR_H
