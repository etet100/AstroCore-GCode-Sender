// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "heightmapborderdrawer.h"

HeightMapBorderDrawer::HeightMapBorderDrawer() : m_model(nullptr)
{
}

void HeightMapBorderDrawer::setModel(Heightmap& model)
{
    m_model = &model;
    update();
}

bool HeightMapBorderDrawer::updateData(GLPalette& palette)
{
    if (!m_model) return false;

    const QPointF p1 = m_model->startPos();
    const QPointF p2 = m_model->endPos();
    const GLuint color = palette.color(1.0, 0.0, 0.0);

    m_lines = QVector<VertexData>()
        << VertexData(QVector3D(p1.x(), p1.y(), 0), color)
        << VertexData(QVector3D(p1.x(), p2.y(), 0), color)
        << VertexData(QVector3D(p1.x(), p2.y(), 0), color)
        << VertexData(QVector3D(p2.x(), p2.y(), 0), color)
        << VertexData(QVector3D(p2.x(), p2.y(), 0), color)
        << VertexData(QVector3D(p2.x(), p1.y(), 0), color)
        << VertexData(QVector3D(p2.x(), p1.y(), 0), color)
        << VertexData(QVector3D(p1.x(), p1.y(), 0), color);

    return true;
}


