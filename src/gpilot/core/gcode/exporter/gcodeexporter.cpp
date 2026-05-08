// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcodeexporter.h"
#include <QFile>
#include <QTextStream>
#include <stdexcept>

GCodeExporter::GCodeExporter()
{
}

void GCodeExporter::exportToFile(GCode &gcode, const QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error("Cannot open file for writing");
    }

    QTextStream out(&file);
    for (int i = 0; i < gcode.count(); i++) {
        out << gcode[i].line << "\n";
    }

    file.close();
}
