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
    void setModel(Heightmap &model);

protected:
    bool updateData(GLPalette &palette) override;

private:
    Heightmap* m_model;
};

#endif // HEIGHTMAPBORDERDRAWER_H
