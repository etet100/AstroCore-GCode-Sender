// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef HEIGHTMAPEXPORTER_H
#define HEIGHTMAPEXPORTER_H

#include <QString>
#include "core/heightmap/heightmap.h"

class HeightmapExporter
{
    public:
        HeightmapExporter();

        static void exportToFile(const Heightmap& heightmap, const QString fileName);

    private:
        static QString interpolationModeToString(Heightmap::InterpolationMode mode);
};

#endif // HEIGHTMAPEXPORTER_H
