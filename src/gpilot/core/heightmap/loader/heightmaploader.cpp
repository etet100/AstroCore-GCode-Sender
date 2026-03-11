// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "heightmaploader.h"
#include <QFile>
#include <QTextStream>

HeightmapLoader::HeightmapLoader() {}

Heightmap HeightmapLoader::loadFromFile(const QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Cannot open file for reading");
    }

    QTextStream in(&file);
    QString line;
    QSize size;
    QPointF startPos;
    QSizeF stepSize;
    int row = 0;
    QList<double> data;
    Heightmap::InterpolationMode interpolationMode = Heightmap::InterpolationMode::Bicubic;

    line = in.readLine().trimmed();
    validateHeader(line);

    while (!in.atEnd()) {
        line = in.readLine().trimmed();

        if (line.startsWith("size:")) {
            size = parseSize(line.mid(5).trimmed());
            data.resize(size.width() * size.height());
        } else if (line.startsWith("startPos:")) {
            startPos = parseStartPos(line.mid(9).trimmed());
        } else if (line.startsWith("stepSize:")) {
            stepSize = parseStepSize(line.mid(9).trimmed());
        } else if (line.startsWith("interpolationMode:")) {
            interpolationMode = parseInterpolationMode(line.mid(18).trimmed());
        } else if (line.startsWith("row:")) {
            parseRowData(line.mid(4).trimmed(), row, size, data);
            row++;
        }
    }

    if (row != size.height()) {
        throw std::runtime_error("Insufficient data rows in heightmap file");
    }

    return Heightmap(size, startPos, stepSize, interpolationMode, data);
}

void HeightmapLoader::validateHeader(const QString &line)
{
    if (line != "# g-pilot heightmap") {
        throw std::runtime_error("Invalid heightmap file format");
    }
}

QSize HeightmapLoader::parseSize(const QString &line)
{
    QStringList parts = line.split(" ");
    if (parts.size() != 2) {
        throw std::runtime_error("Invalid size parameters in heightmap file");
    }

    QSize size(parts[0].toInt(), parts[1].toInt());
    if (size.width() > 1000 || size.height() > 1000) {
        throw std::runtime_error("Heightmap size too large");
    }

    return size;
}

QPointF HeightmapLoader::parseStartPos(const QString &line)
{
    QStringList parts = line.split(" ");
    if (parts.size() != 2) {
        throw std::runtime_error("Invalid startPos parameters in heightmap file");
    }

    return QPointF(parts[0].toDouble(), parts[1].toDouble());
}

QSizeF HeightmapLoader::parseStepSize(const QString &line)
{
    QStringList parts = line.split(" ");
    if (parts.size() != 2) {
        throw std::runtime_error("Invalid stepSize parameters in heightmap file");
    }

    return QSizeF(parts[0].toDouble(), parts[1].toDouble());
}

Heightmap::InterpolationMode HeightmapLoader::parseInterpolationMode(const QString &line)
{
    QString modeStr = line.toLower();
    if (modeStr == "linear") {
        return Heightmap::InterpolationMode::Linear;
    } else if (modeStr == "bilinear") {
        return Heightmap::InterpolationMode::Bilinear;
    } else if (modeStr == "bicubic") {
        return Heightmap::InterpolationMode::Bicubic;
    } else if (modeStr == "nearest") {
        return Heightmap::InterpolationMode::NearestNeighbour;
    } else {
        throw std::runtime_error("Invalid interpolation mode in heightmap file");
    }
}

void HeightmapLoader::parseRowData(const QString &line, int row, const QSize &size, QList<double> &data)
{
    if (row > size.height() - 1) {
        throw std::runtime_error("Too many data rows in heightmap file");
    }

    if (data.size() != size.width() * size.height()) {
        throw std::runtime_error("Heightmap parameters not fully specified before data");
    }

    QStringList parts = line.split(" ");
    if (parts.size() != size.width()) {
        throw std::runtime_error("Data row width does not match specified heightmap size");
    }

    for (int col = 0; col < parts.size() && col < size.width(); ++col) {
        data[row * size.width() + col] = parts[col].toDouble();
    }
}
