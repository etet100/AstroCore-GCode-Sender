// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationheightmap.h"
#include "core/config/configuration.h"

const QMap<QString,QVariant> DEFAULTS = {
    {"heightmapAreaX1", 0},
    {"heightmapAreaY1", 0},
    {"heightmapAreaX2", 100},
    {"heightmapAreaY2", 100},
    {"heightmapGridX", 5},
    {"heightmapGridY", 5},
    {"heightmapGridZTop", 1},
    {"heightmapGridZBottom", -1},
    {"heightmapProbeFeed", 10},
    {"heightmapInterpolationStepX", 5},
    {"heightmapInterpolationStepY", 5},
    {"heightmapInterpolationType", 0},
};

ConfigurationHeightmap::ConfigurationHeightmap(QObject *parent) : AbstractConfigurationModule(parent, DEFAULTS)
{
}

ConfigurationHeightmap& ConfigurationHeightmap::instance()
{
    static ConfigurationHeightmap inst;
    return inst;
}

void ConfigurationHeightmap::registerWith(Configuration& cfg)
{
    cfg.registerModule(&instance());
}
