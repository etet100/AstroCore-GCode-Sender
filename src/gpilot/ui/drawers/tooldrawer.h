// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef TOOLDRAWER_H
#define TOOLDRAWER_H

#include <QVector3D>
#include <QTimer>
#include <QColor>
#include <cmath>
#include "shaderdrawable.h"
#include "ui/config/configurationvisualizer.h"

class ToolDrawer : public ShaderDrawable
{
public:
    explicit ToolDrawer();
    ~ToolDrawer();

    void setToolDiameter(double toolDiameter);
    void setToolLength(double toolLength);
    void setToolPosition(const QVector3D &toolPosition);
    void setRotationSpeed(double speed);
    void setToolAngle(double toolAngle);
    void setColor(const QColor &color);
    void setMode(ConfigurationVisualizer::ToolType);
    bool sort(QMatrix4x4 viewMatrix) override;

protected:
    bool updateData(GLPalette &palette) override;

private:
    double m_toolDiameter;
    double m_toolLength;
    double m_endLength;
    double m_rotationAngle;
    double m_rotationSpeed;
    double m_toolAngle;
    QColor m_color;
    ConfigurationVisualizer::ToolType m_mode;
    QTimer *m_rotationTimer;

    void setRotationAngle(double rotationAngle);
    void rotate(double angle);
    double normalizeAngle(double angle);
    QVector<VertexData> createCircle(QVector3D center, double radius, int arcs, uint color);
    void createLines(const int arcs, VertexData &vertex);
    void createTriangles(const int arcs, VertexData &vertex);
    void updateEndLength();
};

#endif // TOOLDRAWER_H
