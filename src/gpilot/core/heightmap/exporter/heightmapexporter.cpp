// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "heightmapexporter.h"
#include <QFile>
#include <QTextStream>

HeightmapExporter::HeightmapExporter() {}

// # Heightmap
// width  100
//     height 80
//     startPos 0.0 0.0
//     stepSize 1.0 1.0

// # data (row = Y, col = X)
//     0.12 0.15 0.18 0.20
//     0.10 0.13 0.17 0.19

void HeightmapExporter::exportToFile(const Heightmap& heightmap, const QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error("Cannot open file for writing");
    }

    QTextStream out(&file);
    out << "# g-pilot heightmap\n";
    out << "version: 1";
    out << "# params\n";
    out << "size: " << heightmap.gridWidth() << " " << heightmap.gridHeight() << "\n";
    out << "startPos: " << heightmap.startPos().x() << " " << heightmap.startPos().y() << "\n";
    out << "stepSize: " << heightmap.stepWidth() << " " << heightmap.stepHeight() << "\n";
    out << "interpolationMode: " << interpolationModeToString(heightmap.interpolationMode()) << "\n";
    out << "# data (row = Y, col = X)\n";
    for (int row = 0; row < heightmap.gridHeight(); ++row) {
        out << "row: ";
        for (int col = 0; col < heightmap.gridWidth(); ++col) {
            out << heightmap.at(col, row);
            if (col < heightmap.gridWidth() - 1) {
                out << " ";
            }
        }
        out << "\n";
    }

    file.close();
}

QString HeightmapExporter::interpolationModeToString(Heightmap::InterpolationMode mode)
{
    switch (mode) {
        case Heightmap::InterpolationMode::Linear:
            return "linear";
        case Heightmap::InterpolationMode::Bilinear:
            return "bilinear";
        case Heightmap::InterpolationMode::Bicubic:
            return "bicubic";
        case Heightmap::InterpolationMode::NearestNeighbour:
            return "nearest";
        default:
            throw std::runtime_error("Unknown interpolation mode");
    }
}
