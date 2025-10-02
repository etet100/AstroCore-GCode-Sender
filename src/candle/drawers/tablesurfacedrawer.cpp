// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "tablesurfacedrawer.h"

TableSurfaceDrawer::TableSurfaceDrawer() {
}

bool TableSurfaceDrawer::updateData(GLPalette &palette)
{
    const float SIZE = 50;

    int color = palette.color(m_gridColor.redF(), m_gridColor.greenF(), m_gridColor.blueF());

    m_lines.clear();

    for (int i = -SIZE; i < SIZE; i++) {
        m_lines
            << VertexData(QVector3D(i * 10, -SIZE * 10, 0), color, QVector3D(sNan, sNan, sNan))
            << VertexData(QVector3D(i * 10, SIZE * 10, 0), color, QVector3D(sNan, sNan, sNan))
            << VertexData(QVector3D(-SIZE * 10, i * 10, 0), color, QVector3D(sNan, sNan, sNan))
            << VertexData(QVector3D(SIZE * 10, i * 10, 0), color, QVector3D(sNan, sNan, sNan));
    }

    return true;
}
