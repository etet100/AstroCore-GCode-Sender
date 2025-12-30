// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef HEIGHTMAPGRIDDRAWER_H
#define HEIGHTMAPGRIDDRAWER_H

#include <QObject>
#include "shaderdrawable.h"
#include "billboarddrawable.h"
#include "core/heightmap/heightmap.h"

struct HeightMapGridBillboardContentData : public BillboardContentData
{
        HeightMapGridBillboardContentData(const QString& text, const QColor& bgColor, const QColor& textColor)
            : text(text)
            , textColor(textColor)
            , bgColor(bgColor)
        {}

        QString text;
        QColor textColor;
        QColor bgColor;
};

class HeightMapGridBillboardDrawer : public BillboardDrawable
{
    public:
        explicit HeightMapGridBillboardDrawer();

    protected:
        void drawBillboard(QPainter& painter, const QRect& rect, const BillboardContentData* data, const QString& text, const QColor& textColor) override;
};

class HeightMapGridDrawer : public ShaderDrawable
{
    public:
        explicit HeightMapGridDrawer();
        void setModel(Heightmap &model);
        BillboardDrawable* billboardDrawable() { return &m_billboardDrawable; }

    protected:
        bool updateData(GLPalette &palette) override;

    private:
        Heightmap &m_model;
        HeightMapGridBillboardDrawer m_billboardDrawable;
        void generateLines(QSize gridSize, Heightmap::MinMax minMax, QPointF startPos, QSizeF stepSize, VertexData vertex, GLPalette &palette);
        void generatePlates(QSize gridSize, Heightmap::MinMax minMax, QPointF startPos, QSizeF stepSize, VertexData vertex, GLPalette &palette);
        void generateTriangles(QSize gridSize, Heightmap::MinMax minMax, QPointF startPos, QSizeF stepSize, VertexData vertex, GLPalette &palette);
};

#endif // HEIGHTMAPGRIDDRAWER_H
