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

    private:
        static QSize parseSize(const QString &line);
        static QPointF parseStartPos(const QString &line);
        static QSizeF parseStepSize(const QString &line);
        static Heightmap::InterpolationMode parseInterpolationMode(const QString &line);
        static void parseRowData(const QString &line, int row, const QSize &size, QList<double> &data);
        static void validateHeader(const QString &line);
};

#endif // HEIGHTMAPLOADER_H
