// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationheightmap.h"
\
const QMap<QString,QVariant> DEFAULTS = {
    {"heightmapBorderX", 0},
    {"heightmapBorderY", 0},
    {"heightmapBorderWidth", 1},
    {"heightmapBorderHeight", 1},
    {"heightmapBorderShow", false},
    {"heightmapGridX", 1},
    {"heightmapGridY", 1},
    {"heightmapGridZTop", 1},
    {"heightmapGridZBottom", -1},
    {"heightmapProbeFeed", 10},
    {"heightmapGridShow", false},
    {"heightmapInterpolationStepX", 1},
    {"heightmapInterpolationStepY", 1},
    {"heightmapInterpolationType", 0},
    {"heightmapInterpolationShow", false},
};

ConfigurationHeightmap::ConfigurationHeightmap(QObject *parent) : ConfigurationModule(parent, DEFAULTS)
{
}

Q_PROPERTY(double heightmapBorderX MEMBER m_heightmapBorderX NOTIFY changed)
Q_PROPERTY(double heightmapBorderY MEMBER m_heightmapBorderY NOTIFY changed)
Q_PROPERTY(double heightmapBorderWidth MEMBER m_heightmapBorderWidth NOTIFY changed)
Q_PROPERTY(double heightmapBorderHeight MEMBER m_heightmapBorderHeight NOTIFY changed)
Q_PROPERTY(bool heightmapBorderShow MEMBER m_heightmapBorderShow NOTIFY changed)
Q_PROPERTY(double heightmapGridX MEMBER m_heightmapGridX NOTIFY changed)
Q_PROPERTY(double heightmapGridY MEMBER m_heightmapGridY NOTIFY changed)
Q_PROPERTY(double heightmapGridZTop MEMBER m_heightmapGridZTop NOTIFY changed)
Q_PROPERTY(double heightmapGridZBottom MEMBER m_heightmapGridZBottom NOTIFY changed)
Q_PROPERTY(double heightmapProbeFeed MEMBER m_heightmapProbeFeed NOTIFY changed)
Q_PROPERTY(bool heightmapGridShow MEMBER m_heightmapGridShow NOTIFY changed)
Q_PROPERTY(double heightmapInterpolationStepX MEMBER m_heightmapInterpolationStepX NOTIFY changed)
Q_PROPERTY(double heightmapInterpolationStepY MEMBER m_heightmapInterpolationStepY NOTIFY changed)
Q_PROPERTY(int heightmapInterpolationType MEMBER m_heightmapInterpolationType NOTIFY changed)
Q_PROPERTY(bool heightmapInterpolationShow MEMBER m_heightmapInterpolationShow NOTIFY changed)
