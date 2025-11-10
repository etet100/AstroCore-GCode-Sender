// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef HEIGHTMAPGRIDDRAWER_H
#define HEIGHTMAPGRIDDRAWER_H

#include <QObject>
#include "shaderdrawable.h"
#include "core/heightmap/heightmap.h"

class HeightMapGridDrawer : public ShaderDrawable
{
public:
    HeightMapGridDrawer(Heightmap &model);

    // QPointF gridSize() const;
    // void setGridSize(const QPointF &gridSize);

    // QRectF borderRect() const;
    // void setBorderRect(const QRectF &borderRect);

    // double zTop() const;
    void setZTop(double zTop);

    // double zBottom() const;
    void setZBottom(double zBottom);

    // QAbstractTableModel *model() const;
    // void setModel(QAbstractTableModel *model);

protected:
    bool updateData(GLPalette &palette) override;

private:
    // QPointF m_gridSize;
    // QRectF m_borderRect;
    double m_zTop;
    double m_zBottom;
    // QAbstractTableModel *m_model;
    Heightmap &m_model;
    void generateLines(int gridPointsY, double min, QPointF startPos, double gridStepX, VertexData vertex, double max, GLPalette &palette, int gridPointsX, double gridStepY);
    void generateTriangles(int gridPointsY, double min, QPointF startPos, double gridStepX, VertexData vertex, double max, GLPalette &palette, int gridPointsX, double gridStepY);
};

#endif // HEIGHTMAPGRIDDRAWER_H
