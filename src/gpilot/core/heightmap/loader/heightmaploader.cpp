// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "heightmaploader.h"
#include <QFile>
#include <QTextStream>

HeightmapLoader::HeightmapLoader() {}

// # g-pilot heightmap
// version: 1
// # params
// size: 100 100
// startPos: 0 0
// stepSize: 5 5
// # data (row = Y, col = X)
// row: -3.63333 -3.9 1.96667 0.866667 -2.56667 -2.86667 -3.2 4 -3.8 4.63333 3.9 2.26667 -4.7 -4.13333 -4.66667 2.2 -3.5 -0.1 -2.6 -2.96667 -1.4 1.5 -0.566667 -4.93333 -2.76667 3.36667 -0.866667 -4.73333 -0.9 0.366667 -3 -0.7 -2.4 -2.66667 2.73333 0.2 -2.06667 -0.266667 -0.366667 -3.16667 0.533333 1.63333 -2.23333 4.86667 -2.4 2.1 4.86667 1.86667 -5 0.533333 3.03333 -2.33333 -1.1 2.16667 -2.7 3.86667 -0.933333 -3.13333 3.33333 1.43333 4.1 4.43333 -2.56667 -2.43333 -3.03333 0.0666667 -2.76667 -0.3 4.7 -3.5 -0.433333 -0.0333333 -0.333333 0.4 2.43333 -3.96667 -4.1 3.56667 -0.6 3.33333 -1.16667 2.56667 0.433333 3.66667 1.1 -4.73333 -3.6 -2.53333 -4.76667 2 -1.33333 1.33333 -1.5 -0.3 -0.766667 2.16667 1.16667 -3.7 3.8 -4.73333
// row: 0.566667 2.16667 -1.13333 -0.7 0.666667 0.333333 2.03333 -4.06667 -2.23333 -3.4 -4.06667 -1.76667 -5 4.03333 1.63333 -0.6 0.666667 1.33333 -4.8 4.36667 -3.2 -0.366667 4.76667 -2.93333 1.43333 -1.36667 1.03333 -0.6 2.63333 1.36667 -1.4 2.03333 -3.5 -2 -3.3 2.23333 -4.5 -3.76667 -0.8 4.56667 -4.46667 -1.7 3.23333 4.53333 1.86667 -3.93333 -4.13333 4.43333 3.93333 1.16667 -1.76667 -2.66667 1.4 -3.26667 -3.26667 -3.76667 3.53333 -2.3 2.6 1.16667 2.7 4.5 3.26667 -2.63333 1.76667 -0.7 -1.6 -3.03333 -2.4 -4.9 0.266667 -0.9 2.7 3.76667 -1.33333 -4.86667 -1 -3.06667 3.6 3.23333 0.433333 -0.833333 2.1 -4.46667 3.23333 -2.13333 -4.33333 0.933333 1.1 0.9 1.56667 0.733333 -4.66667 2.96667 4.46667 -1.7 -1.3 1.46667 -2.1 0.4
// row: -3.86667 -0.366667 -3.83333 3.93333 3.76667 1.73333 3.03333 -0.766667 -3.83333 -2.26667 4.3 -4.06667 1.5 1.56667 -0.433333 3.4 4.16667 0.7 -3.13333 -3.26667 2.4 -2.73333 4.66667 2.33333 -1.8 -2.1 -1.66667 0.933333 1.3 3.7 4.4 -4.63333 3 4.1 -3.76667 2.56667 -4.73333 -1.46667 1.5 2.46667 -2.06667 2.33333 -3.66667 -4.63333 -4.1 4.73333 1.06667 -0.166667 -0.9 -3.7 -0.166667 4.66667 -0.666667 -3.2 -0.5 3.26667 -2.76667 -1.83333 -0.733333 -4.46667 -1.53333 4.53333 -1.33333 -4.26667 -4.8 3 -2.83333 2.33333 3.1 -4 -1.46667 4.43333 -2.66667 4.26667 -2.63333 1.5 -4.83333 2.3 4.6 3.6 -4.43333 -1.66667 -3.96667 1.03333 -4.23333 2.9 -0.833333 -0.433333 -4.5 -2.2 1.56667 -0.4 -4.06667 -2.06667 3.5 2.4 0.533333 0.333333 -4.03333 3.33333
// row: -1.66667 3.6 -3.3 1.4 3.93333 -4.46667 -2.5 1.13333 -0.8 0.1 -2.86667 -3.3 4.1 2.06667 4.73333 -3.63333 4.63333 -1.5 -3.56667 -3.23333 -4.76667 1.6 1.76667 4 3.7 4.43333 0.366667 -1.2 0.533333 -4 3.73333 -3.2 2.76667 -0.766667 -1.2 3.5 -1.33333 0.566667 -4.8 2.4 -0.833333 -1 1.16667 0.866667 -1.23333 -2.26667 -4.93333 4.13333 -0.1 -4.4 0.133333 2.06667 -4.4 1.06667 0.7 3.26667 3.66667 1.2 -4.26667 -1.4 1.1 -1 1.53333 3.76667 -1.96667 3.66667 4.36667 2.16667 3.7 -4.33333 -1.3 -4.83333 -3.2 -4.36667 4.26667 4.8 -4.1 3.16667 -0.2 -0.366667 -1.1 3.26667 -2.13333 4.23333 4.36667 -1.56667 4.36667 -5 4.23333 -2.5 -1.5 0.7 -4.56667 -0.366667 -1.83333 -0.566667 -3.2 2.7 2.26667 2.46667
// ...

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

    line = in.readLine().trimmed();
    if (line != "# g-pilot heightmap") {
        throw std::runtime_error("Invalid heightmap file format");
    }

    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (line.startsWith("size:")) {
            QStringList parts = line.mid(5).trimmed().split(" ");
            if (parts.size() == 2) {
                size = QSize(parts[0].toInt(), parts[1].toInt());
                if (size.width() > 1000 || size.height() > 1000) {
                    throw std::runtime_error("Heightmap size too large");
                }
                data.resize(size.width() * size.height());
            } else {
                throw std::runtime_error("Invalid size parameters in heightmap file");
            }
        } else if (line.startsWith("startPos:")) {
            QStringList parts = line.mid(9).trimmed().split(" ");
            if (parts.size() == 2) {
                startPos = QPointF(parts[0].toDouble(), parts[1].toDouble());
            } else {
                throw std::runtime_error("Invalid startPos parameters in heightmap file");
            }
        } else if (line.startsWith("stepSize:")) {
            QStringList parts = line.mid(9).trimmed().split(" ");
            if (parts.size() == 2) {
                stepSize = QSizeF(parts[0].toDouble(), parts[1].toDouble());
            } else {
                throw std::runtime_error("Invalid stepSize parameters in heightmap file");
            }
        } else if (line.startsWith("row:")) {
            if (row > size.height() - 1) {
                throw std::runtime_error("Too many data rows in heightmap file");
            }
            if (data.size() != size.width() * size.height()) {
                throw std::runtime_error("Heightmap parameters not fully specified before data");
            }

            QStringList parts = line.mid(4).trimmed().split(" ");
            if (parts.size() != size.width()) {
                throw std::runtime_error("Data row width does not match specified heightmap size");
            }

            // Convert row
            for (int col = 0; col < parts.size() && col < size.width(); ++col) {
                data[row * size.width() + col] = parts[col].toDouble();
            }

            row++;
        }
    }

    if (row != size.height()) {
        throw std::runtime_error("Insufficient data rows in heightmap file");
    }

    return Heightmap(size, startPos, stepSize, data);
}
