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
        HeightMapGridBillboardContentData(const QPoint& pos, float height, const QString& text, const QColor& bgColor, const QColor& textColor)
            : pos(pos)
            , height(height)
            , text(text)
            , textColor(textColor)
            , bgColor(bgColor)
        {}

        QPoint pos;
        float height;
        QString text;
        QColor textColor;
        QColor bgColor;
};

class HeightMapGridBillboardDrawer : public BillboardDrawable
{
    public:
        explicit HeightMapGridBillboardDrawer();

    protected:
        void drawBillboard(QPainter& painter, const QRect& rect, const BillboardContentData* data) override;
        QSize measureBillboard(const BillboardContentData *data) override;
        QString buildCacheKey(const BillboardContentData *data) override;
};

class HeightMapGridDrawer : public ShaderDrawable
{
    public:
        explicit HeightMapGridDrawer();
        void setModel(Heightmap &model);
        BillboardDrawable* billboardDrawable() { return &m_billboardDrawable; }
        void setVisible(bool visible);
        void toggleVisible();

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
