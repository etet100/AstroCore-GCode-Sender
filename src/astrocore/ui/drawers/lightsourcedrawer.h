// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2024 BTS

#ifndef LIGHTSOURCEDRAWER_H
#define LIGHTSOURCEDRAWER_H

#include <QVector3D>
#include <QColor>
#include <cmath>
#include "shaderdrawable.h"

class LightSourceDrawer : public ShaderDrawable
{
public:
    explicit LightSourceDrawer();

    void setPosition(const QVector3D &position);
    void setColor(const QColor &color);

    bool sort(QMatrix4x4 viewMatrix) override;

protected:
    bool updateData(GLPalette &palette) override;

private:
    static constexpr double DIAMETER = 10.0;

    QVector3D m_position;
    QColor m_color;
};

#endif // LIGHTSOURCEDRAWER_H
