// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationheightmap.h"
\
const QMap<QString,QVariant> DEFAULTS = {
    {"heightmapAreaX1", 0},
    {"heightmapAreaY1", 0},
    {"heightmapAreaX2", 100},
    {"heightmapAreaY2", 100},
    {"heightmapAreaShow", false},
    {"heightmapGridX", 1},
    {"heightmapGridY", 1},
    {"heightmapGridZTop", 1},
    {"heightmapGridZBottom", -1},
    {"heightmapProbeFeed", 10},
    {"heightmapGridShow", false},
    {"heightmapInterpolationStepX", 5},
    {"heightmapInterpolationStepY", 5},
    {"heightmapInterpolationType", 0},
    {"heightmapInterpolationShow", false},
};

ConfigurationHeightmap::ConfigurationHeightmap(QObject *parent) : ConfigurationModule(parent, DEFAULTS)
{
}
