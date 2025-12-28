// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef HEIGHTMAPLOADER_H
#define HEIGHTMAPLOADER_H

#include <QString>
#include "core/heightmap/heightmap.h"

class HeightmapLoader
{
    public:
        HeightmapLoader();

        static Heightmap loadFromFile(const QString fileName);
};

#endif // HEIGHTMAPLOADER_H
