// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef CURSORDRAWER_H
#define CURSORDRAWER_H

#include <QVector3D>
#include <QTimer>
#include <QColor>
#include "shaderdrawable.h"
#include <chrono>
#include "../widgets/glwidget.h"

using namespace std::chrono;

class CursorDrawer : public ShaderDrawable
{
    public:
        explicit CursorDrawer();

        void setPosition(QPointF position);
        void setColor(const QColor &color);
        void rotate();

    protected:
        bool updateData(GLPalette &palette) override;

    private:
        static const int DIAMETER = 5;
        static const int LENGTH = 25;
        double m_distanceFromSurface = 0;
        double m_endLength = 10;
        QPointF m_position = {0, 0};
        QColor m_color = {255, 255, 0};
        double m_rotationAngle = 0;
        high_resolution_clock m_clock;
        double normalizeAngle(double angle);
        QVector<VertexData> createCircle(QVector3D center, double radius, int arcs, int color);
};

#endif // CURSORDRAWER_H
