// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef TABLESURFACEDRAWER_H
#define TABLESURFACEDRAWER_H

#include "shaderdrawable.h"

class TableSurfaceDrawer : public ShaderDrawable
{
    public:
        TableSurfaceDrawer();
        void setGridColor(QColor color) { m_gridColor = color; }

    protected:
        bool updateData(GLPalette &palette) override;

    private:
        QColor m_gridColor;
};

#endif // TABLESURFACEDRAWER_H
