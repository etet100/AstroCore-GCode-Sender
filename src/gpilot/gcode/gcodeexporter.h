// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef GCODEEXPORTER_H
#define GCODEEXPORTER_H

#include "gcode.h"

class GCodeExporter
{
    public:
        GCodeExporter();
        void exportToFile(GCode &gcode, const QString fileName);
};

#endif // GCODEEXPORTER_H
