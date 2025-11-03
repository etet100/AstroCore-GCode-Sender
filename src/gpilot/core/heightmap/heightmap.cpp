// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "heightmap.h"
#include <QDebug>
#include <cmath>

Heightmap::Heightmap() : Heightmap(QSize(100, 100))
{
}

Heightmap::Heightmap(QSize size) : m_size(size)
{
    m_startPos = QPointF(0.0, 0.0);
    m_endPos = QPointF(0.0, 0.0);
    m_stepSize = QSize(5, 5);
    m_data = nullptr;
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
    // return (rand() % 100) / 10.0;
    return sin(1.0 * pt.x()) * cos(1.0 * pt.y()) * 3.0;

 //   return m_data[pt.x()][pt.y()];
}
