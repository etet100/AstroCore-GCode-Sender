// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef HEIGHTMAPGRIDDRAWER_H
#define HEIGHTMAPGRIDDRAWER_H

#include <QObject>
#include "shaderdrawable.h"
#include "billboarddrawable.h"
#include "core/heightmap/heightmap.h"
#include "core/heightmap/interpolator/heightmapinterpolator.h"

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
        void setModel(Heightmap& model);
        BillboardDrawable* billboardDrawable() { return &m_billboardDrawable; }
        void setVisible(bool visible);
        void toggleVisible();
        void setInterpolationMode(Heightmap::InterpolationMode mode);

    protected:
        bool updateData(GLPalette &palette) override;

    private:
        static constexpr int SUBDIVISIONS_PER_CELL = 5;

        Heightmap* m_model;
        HeightMapGridBillboardDrawer m_billboardDrawable;
        Heightmap::InterpolationMode m_interpolationMode;
        void generateLines(QSize gridSize, Heightmap::MinMax minMax, QPointF startPos, QSizeF stepSize, VertexData vertex, GLPalette& palette);
        void generatePlates(QSize gridSize, Heightmap::MinMax minMax, QPointF startPos, QSizeF stepSize, VertexData vertex, GLPalette& palette);
        void generateTriangles(QSize gridSize, Heightmap::MinMax minMax, QPointF startPos, QSizeF stepSize, VertexData vertex, GLPalette& palette);
        HeightmapInterpolator* createInterpolator();
};

#endif // HEIGHTMAPGRIDDRAWER_H
