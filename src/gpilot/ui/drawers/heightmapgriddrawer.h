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
        explicit HeightMapGridDrawer();
        void setModel(Heightmap &model);

    protected:
        bool updateData(GLPalette &palette) override;

    private:
        Heightmap &m_model;
        void generateLines(QSize gridSize, Heightmap::MinMax minMax, QPointF startPos, QSizeF stepSize, VertexData vertex, GLPalette &palette);
        void generateTriangles(QSize gridSize, Heightmap::MinMax minMax, QPointF startPos, QSizeF stepSize, VertexData vertex, GLPalette &palette);
};

#endif // HEIGHTMAPGRIDDRAWER_H
