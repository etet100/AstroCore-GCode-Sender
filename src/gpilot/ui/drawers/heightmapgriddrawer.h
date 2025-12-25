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
        void generateLines(int gridPointsY, double min, QPointF startPos, double gridStepX, VertexData vertex, double max, GLPalette &palette, int gridPointsX, double gridStepY);
        void generateTriangles(int gridPointsY, double min, QPointF startPos, double gridStepX, VertexData vertex, double max, GLPalette &palette, int gridPointsX, double gridStepY);
};

#endif // HEIGHTMAPGRIDDRAWER_H
