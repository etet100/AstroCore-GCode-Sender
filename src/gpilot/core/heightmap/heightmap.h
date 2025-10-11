// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <QSize>
#include <QPointF>
#include <utility>

class Heightmap
{
    public:
        Heightmap();
        Heightmap(QSize size);

        // Size of the heightmap grid, not a physical size
        QSize gridSize() const;
        int gridWidth() const;
        int gridHeight() const;
        bool isInside(QPointF ptMM) const;
        QPointF startPos() const { return m_startPos; }
        QSize stepSize() const { return m_stepSize; }
        QPair<int, int> gridIndices(const QPointF& pt_mm) const;
        double valueAt(QPoint pt) const;

    private :
        QSize m_size;
        QPointF m_startPos;
        QPointF m_endPos;
        QSize m_stepSize;
        // m_size.x * m_size.y of z values
        double** m_data;
};

#endif // HEIGHTMAP_H
