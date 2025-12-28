// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "heightmap.h"
#include <QDebug>
#include <cmath>

Heightmap::Heightmap() : Heightmap(QSize(100, 100))
{
}

Heightmap::Heightmap(const Heightmap &other)
{
    m_size = other.m_size;
    m_startPos = other.m_startPos;
    m_stepSize = other.m_stepSize;
    m_endPos = other.m_endPos;
    m_data = other.m_data;
    m_minMax = other.m_minMax;
}

Heightmap::Heightmap(QSize size, QPointF startPos, QSizeF stepSize, const QList<double>& data)
{
    m_size = size;
    m_startPos = startPos;
    m_stepSize = stepSize;
    m_endPos = QPointF(startPos.x() + stepSize.width() * (size.width() - 1),
                      startPos.y() + stepSize.height() * (size.height() - 1));
    m_data = data;
    updateMinMax();
}

Heightmap::Heightmap(QSize size) : m_size(size)
{
    m_startPos = QPointF(0.0, 0.0);
    m_endPos = QPointF(0.0, 0.0);
    m_stepSize = QSize(5, 5);
    m_data.resize(m_size.width() * m_size.height());
    generateRandom();
    updateMinMax();
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

QPair<int, int> Heightmap::gridIndices(const QPointF &ptMm) const {
    int i = static_cast<int>((ptMm.x() - m_startPos.x()) / m_stepSize.width());
    int j = static_cast<int>((ptMm.y() - m_startPos.y()) / m_stepSize.height());

    return QPair<int, int>(i, j);
}

double Heightmap::valueAt(QPoint pt) const
{
    // row, col
    return at(pt.y(), pt.x());
}

double& Heightmap::at(int row, int col)
{
    return m_data[row * m_size.width() + col];
}

double Heightmap::at(int row, int col) const
{
    return m_data[row * m_size.width() + col];
}

void Heightmap::setSize(QSize size)
{
    m_size = size;
    m_data.resize(m_size.width() * m_size.height());
}

void Heightmap::generateRandom()
{
    for (int i = 0; i < m_size.height(); i++) {
        for (int j = 0; j < m_size.width(); j++) {
            at(i, j) = (rand() % 300 / 30.0) - 5.0;
        }
    }
}

void Heightmap::generateSinCos()
{
    for (int i = 0; i < m_size.height(); i++) {
        for (int j = 0; j < m_size.width(); j++) {
            at(i, j) = sin(0.1 * i) * cos(0.1 * j) * 3.0;
        }
    }
}

void Heightmap::updateMinMax()
{
    m_minMax = {NAN, NAN};
    for (auto& value : m_data) {
        if (qIsNaN(m_minMax.min) || value < m_minMax.min) {
            m_minMax.min = value;
        }
        if (qIsNaN(m_minMax.max) || value > m_minMax.max) {
            m_minMax.max = value;
        }
    }
}
