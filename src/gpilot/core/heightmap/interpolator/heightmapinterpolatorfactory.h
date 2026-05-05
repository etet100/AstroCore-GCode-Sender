// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#ifndef HEIGHTMAPINTERPOLATORFACTORY_H
#define HEIGHTMAPINTERPOLATORFACTORY_H

#include "abstractheightmapinterpolator.h"
#include "../heightmap.h"

class HeightmapInterpolatorFactory
{
    public:
        static AbstractHeightmapInterpolator* create(const Heightmap* heightmap, Heightmap::InterpolationMode mode);
        static AbstractHeightmapInterpolator* create(const Heightmap* heightmap);
};

#endif // HEIGHTMAPINTERPOLATORFACTORY_H
