// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef TOOLDRAWER_H
#define TOOLDRAWER_H

#include <QVector3D>
#include <QTimer>
#include <QColor>
#include <cmath>
#include "shaderdrawable.h"

class ToolDrawer : public ShaderDrawable
{
public:
    explicit ToolDrawer();

    void setToolDiameter(double toolDiameter);
    void setToolLength(double toolLength);
    void setToolPosition(const QVector3D &toolPosition);
    void setRotationAngle(double rotationAngle);
    void rotate(double angle);
    void setToolAngle(double toolAngle);
    void setColor(const QColor &color);

protected:
    bool updateData(GLPalette &palette) override;

private:
    double m_toolDiameter;
    double m_toolLength;
    double m_endLength;
    QVector3D m_toolPosition;
    double m_rotationAngle;
    double m_toolAngle;
    QColor m_color;

    double normalizeAngle(double angle);
    QVector<VertexData> createCircle(QVector3D center, double radius, int arcs, uint color);
};

#endif // TOOLDRAWER_H
