// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef APPLYHEIGHTMAP_H
#define APPLYHEIGHTMAP_H

#include "converter.h"
#include "core/heightmap/heightmap.h"
#include "core/heightmap/heightmapinterpolator.h"

class ApplyHeightmap : public Converter
{
    public:
        ApplyHeightmap(GCode &data, Heightmap &heightmap);

    private:
        Heightmap &m_heightmap;
        HeightmapInterpolator m_interpolator;
};

#endif // APPLYHEIGHTMAP_H
