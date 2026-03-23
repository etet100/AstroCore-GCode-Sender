// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "heightmap.h"
#include <QDebug>
#include <cmath>

Heightmap::Heightmap() : Heightmap(QSize(20, 20))
{
}

// Heightmap::Heightmap(const Heightmap &other)
// {
//     m_size = other.m_size;
//     m_startPos = other.m_startPos;
//     m_stepSize = other.m_stepSize;
//     m_endPos = other.m_endPos;
//     m_data = other.m_data;
//     m_valuesMinMax = other.m_valuesMinMax;
// }

Heightmap::Heightmap(
    QSize size,
    QPointF startPos,
    QSizeF stepSize,
    InterpolationMode interpolationMode,
    const QList<double>& data
) : m_size(size),
    m_startPos(startPos),
    m_stepSize(stepSize),
    m_interpolationMode(interpolationMode),
    m_data(data)
{
    updateEndPos();
    updateMinMax();
}

Heightmap::Heightmap(QSize size) : m_size(size)
{
    m_startPos = QPointF(0.0, 0.0);
    m_stepSize = QSizeF(10.0, 10.0); // 10 mm steps
    m_data.resize(m_size.width() * m_size.height());
    updateEndPos();
    generateRandom();
    updateMinMax();
}

void Heightmap::updateEndPos()
{
    m_endPos = m_startPos + QPointF(m_stepSize.width() * (m_size.width() - 1), m_stepSize.height() * (m_size.height() - 1));
}

QSize Heightmap::gridSize() const
{
    return m_size;
}

int Heightmap::gridWidth() const
{
    return m_size.width();
}

int Heightmap::gridHeight() const
{
    return m_size.height();
}

bool Heightmap::isInside(QPointF ptMm) const
{
    return ptMm.x() >= m_startPos.x() && ptMm.x() <= m_endPos.x() &&
           ptMm.y() >= m_startPos.y() && ptMm.y() <= m_endPos.y();
}

// Change area, keep grid size the same
void Heightmap::setArea(QRectF area)
{
    m_startPos = area.topLeft();
    m_stepSize = QSizeF(area.width() / (float) (m_size.width() - 1), area.height() / (float) (m_size.height() - 1));
    updateEndPos();
    // reset();
}

QPair<int, int> Heightmap::gridIndices(const QPointF &ptMm) const
{
    int i = static_cast<int>((ptMm.x() - m_startPos.x()) / m_stepSize.width());
    int j = static_cast<int>((ptMm.y() - m_startPos.y()) / m_stepSize.height());

    return QPair<int, int>(i, j);
}

QList<QPointF> Heightmap::probePoints(QPointF currentPos, ScanMode mode) const
{
    // Find nearest grid vertex to current position
    int sx = qBound(0, qRound((currentPos.x() - m_startPos.x()) / m_stepSize.width()),  m_size.width()  - 1);
    int sy = qBound(0, qRound((currentPos.y() - m_startPos.y()) / m_stepSize.height()), m_size.height() - 1);

    auto gridPt = [&](int x, int y) {
        return QPointF(m_startPos.x() + x * m_stepSize.width(),
                       m_startPos.y() + y * m_stepSize.height());
    };

    QList<QPointF> points;
    points.reserve(m_size.width() * m_size.height());

    if (mode == ScanMode::Rows) {
        // Step 1: rest of starting row, going right from sx
        for (int x = sx; x < m_size.width(); x++)
            points << gridPt(x, sy);

        // Step 2: rows after sy, full serpentine (next row goes left)
        bool goRight = false;
        for (int y = sy + 1; y < m_size.height(); y++) {
            for (int x = goRight ? 0 : m_size.width() - 1;
                 goRight ? x < m_size.width() : x >= 0;
                 goRight ? x++ : x--)
                points << gridPt(x, y);
            goRight = !goRight;
        }

        // Step 3: rows before sy, continuing serpentine
        for (int y = sy - 1; y >= 0; y--) {
            for (int x = goRight ? 0 : m_size.width() - 1;
                 goRight ? x < m_size.width() : x >= 0;
                 goRight ? x++ : x--)
                points << gridPt(x, y);
            goRight = !goRight;
        }

        // Step 4: remaining points in starting row (0 to sx-1), continuing direction
        for (int x = goRight ? 0 : sx - 1;
             goRight ? x < sx : x >= 0;
             goRight ? x++ : x--)
            points << gridPt(x, sy);

    } else { // ScanMode::Columns
        // Step 1: rest of starting column, going up from sy
        for (int y = sy; y < m_size.height(); y++)
            points << gridPt(sx, y);

        // Step 2: columns after sx, full serpentine (next column goes down)
        bool goUp = false;
        for (int x = sx + 1; x < m_size.width(); x++) {
            for (int y = goUp ? 0 : m_size.height() - 1;
                 goUp ? y < m_size.height() : y >= 0;
                 goUp ? y++ : y--)
                points << gridPt(x, y);
            goUp = !goUp;
        }

        // Step 3: columns before sx, continuing serpentine
        for (int x = sx - 1; x >= 0; x--) {
            for (int y = goUp ? 0 : m_size.height() - 1;
                 goUp ? y < m_size.height() : y >= 0;
                 goUp ? y++ : y--)
                points << gridPt(x, y);
            goUp = !goUp;
        }

        // Step 4: remaining points in starting column (0 to sy-1), continuing direction
        for (int y = goUp ? 0 : sy - 1;
             goUp ? y < sy : y >= 0;
             goUp ? y++ : y--)
            points << gridPt(sx, y);
    }

    return points;
}

double& Heightmap::at(int x, int y)
{
    return m_data[y * m_size.width() + x];
}

double Heightmap::at(QPoint pt) const
{
    return at(pt.x(), pt.y());
}

double Heightmap::at(int x, int y) const
{
    return m_data[y * m_size.width() + x];
}

void Heightmap::setHeightAt(QPoint point, double height)
{
    at(point.x(), point.y()) = height;
    updateMinMax();
}

void Heightmap::reset()
{
    for (auto& value : m_data) {
        value = NAN;
    }
}

bool Heightmap::anyHeightSet()
{
    for (const auto& value : m_data) {
        if (!qIsNaN(value)) {
            return true;
        }
    }

    return false;
}

void Heightmap::setSize(QSize size)
{
    m_size = size;
    m_data.resize(m_size.width() * m_size.height());
    updateEndPos();
}

void Heightmap::generateRandom()
{
    qDebug() << "[Heightmap] generating random heightmap data...";
    for (int y = 0; y < m_size.height(); y++) {
        for (int x = 0; x < m_size.width(); x++) {
            at(x, y) = ((rand() % (25 * 30)) / 250.0) - 3.0;
        }
    }
}

void Heightmap::generateSinCos()
{
    qDebug() << "[Heightmap] generating sin-cos heightmap data...";
    for (int y = 0; y < m_size.height(); y++) {
        for (int x = 0; x < m_size.width(); x++) {
            at(x, y) = sin(0.1 * y) * cos(0.1 * x) * 3.0;
        }
    }
}

void Heightmap::updateMinMax()
{
    m_valuesMinMax = {NAN, NAN};
    for (auto& value : m_data) {
        if (qIsNaN(m_valuesMinMax.min) || value < m_valuesMinMax.min) {
            m_valuesMinMax.min = value;
        }
        if (qIsNaN(m_valuesMinMax.max) || value > m_valuesMinMax.max) {
            m_valuesMinMax.max = value;
        }
    }
}

void Heightmap::setZeroReference(int x, int y)
{
    double referenceValue = at(x, y);

    offsetAllPoints(-referenceValue);
}

void Heightmap::offsetAllPoints(double offset)
{
    m_valuesMinMax = {NAN, NAN};

    for (auto& value : m_data) {
        if (!qIsNaN(value)) {
            value += offset;
            minMax(value);
        }
    }
}

void Heightmap::minMax(double value)
{
    if (qIsNaN(m_valuesMinMax.min) || value < m_valuesMinMax.min) {
        m_valuesMinMax.min = value;
    }
    if (qIsNaN(m_valuesMinMax.max) || value > m_valuesMinMax.max) {
        m_valuesMinMax.max = value;
    }
}
