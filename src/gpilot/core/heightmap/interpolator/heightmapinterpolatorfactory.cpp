// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#include "heightmapinterpolatorfactory.h"
#include "heightmapnearestneighbourinterpolator.h"
#include "heightmaplinearinterpolator.h"
#include "heightmapbilinearinterpolator.h"
#include "heightmapbicubicinterpolator.h"

AbstractHeightmapInterpolator* HeightmapInterpolatorFactory::create(const Heightmap* heightmap, Heightmap::InterpolationMode mode)
{
    if (!heightmap) {
        return nullptr;
    }

    switch (mode) {
        case Heightmap::InterpolationMode::NearestNeighbour:
            return new HeightmapNearestNeighbourInterpolator(heightmap);
        case Heightmap::InterpolationMode::Linear:
            return new HeightmapLinearInterpolator(heightmap);
        case Heightmap::InterpolationMode::Bilinear:
            return new HeightmapBilinearInterpolator(heightmap);
        case Heightmap::InterpolationMode::Bicubic:
            return new HeightmapBicubicInterpolator(heightmap);
        default:
            return new HeightmapBicubicInterpolator(heightmap);
    }
}

AbstractHeightmapInterpolator* HeightmapInterpolatorFactory::create(const Heightmap* heightmap)
{
    if (!heightmap) {
        return nullptr;
    }

    return create(heightmap, heightmap->interpolationMode());
}
