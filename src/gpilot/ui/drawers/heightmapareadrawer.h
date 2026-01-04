// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef HEIGHTMAPAREADRAWER_H
#define HEIGHTMAPAREADRAWER_H

#include <QObject>
#include "shaderdrawable.h"
#include "billboarddrawable.h"
#include "core/heightmap/heightmap.h"

struct HeightMapAreaBillboardContentData : public BillboardContentData
{
    HeightMapAreaBillboardContentData(QString id, const QPointF& pos)
        : pos(pos)
        , id(id)
    {}

    QPointF pos;
    QString id;
};


class HeightMapAreaBillboardDrawer : public BillboardDrawable
{
    public:
        explicit HeightMapAreaBillboardDrawer();

    protected:
        void drawBillboard(QPainter& painter, const QRect& rect, const BillboardContentData* data) override;
        QSize measureBillboard(const BillboardContentData *data) override;
        QString buildCacheKey(const BillboardContentData *data) override;
        void atlasReady(QImage &atlasImage);

    private:
        QFont m_font;
        QFontMetrics m_fm;
};


class HeightMapAreaDrawer : public ShaderDrawable
{
    public:
        HeightMapAreaDrawer();

        QRectF borderRect() const;
        void setModel(Heightmap &model);
        BillboardDrawable* billboardDrawable() { return &m_billboardDrawable; }

    protected:
        bool updateData(GLPalette &palette) override;

    private:
        Heightmap* m_model;
        HeightMapAreaBillboardDrawer m_billboardDrawable;
        void generateStartEndMarkers();
};

#endif // HEIGHTMAPAREADRAWER_H
