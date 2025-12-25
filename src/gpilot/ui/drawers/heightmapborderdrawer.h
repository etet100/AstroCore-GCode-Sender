// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef HEIGHTMAPBORDERDRAWER_H
#define HEIGHTMAPBORDERDRAWER_H

#include <QObject>
#include "shaderdrawable.h"
#include "core/heightmap/heightmap.h"

class HeightMapBorderDrawer : public ShaderDrawable
{
public:
    HeightMapBorderDrawer();

    QRectF borderRect() const;
    void setBorderRect(const QRectF &borderRect);
    void setModel(Heightmap &model);

protected:
    bool updateData(GLPalette &palette) override;

private:
    QRectF m_borderRect;
    Heightmap& m_model;
};

#endif // HEIGHTMAPBORDERDRAWER_H
