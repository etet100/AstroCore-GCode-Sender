// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcodeexporter.h"
#include <QIODevice>
#include <QTextStream>
#include <QFile>

GCodeExporter::GCodeExporter()
{
}

void GCodeExporter::exportToFile(GCode &gcode, const QString fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        //        QMessageBox::critical(this, this->windowTitle(), tr("Can't open file:\n") + fileName);
        return;
    }

    QTextStream stream(&file);

    file.close();
}
