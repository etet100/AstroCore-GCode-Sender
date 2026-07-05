// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef CURSORDRAWER_H
#define CURSORDRAWER_H

#include <memory>
#include <QColor>
#include <QPointF>
#include <QPropertyAnimation>
#include "compositedrawable.h"
#include "shaderdrawable.h"

// Animated 3D cursor cone.
class CursorDrawer : public QObject, public ShaderDrawable
{
    Q_OBJECT
    Q_PROPERTY(float animation WRITE setAnimation)

public:
    explicit CursorDrawer();

    void setPosition(QPointF position);
    void setVisible(bool visible) override;
    void setColor(const QColor &color);

protected:
    bool updateData(GLPalette &palette) override;
    double m_toolDiameter;
    double m_toolLength;
    double m_endLength;
    QVector3D m_position;
    double m_tipAngle;
    QColor m_color;

    QVector<VertexData> createCircle(QVector3D center, double radius, int arcs, GLuint color);

private:
    QPropertyAnimation *m_animator;

    void startAnimator();
    void setAnimation(float value);
    void createLines(const float z, const int arcs, VertexData &vertex);
    void createTriangles(const float z, const int arcs, VertexData &vertex);
};

// Flat Z=0 circle - the cursor shadow projected on the XY plane.
class CursorShadowDrawer : public ShaderDrawable
{
public:
    explicit CursorShadowDrawer();

    void setPosition(QPointF position);
    void setColor(const QColor &color);
    void setToolDiameter(double diameter);

protected:
    bool updateData(GLPalette &palette) override;

private:
    QVector3D m_position;
    QColor m_color;
    double m_toolDiameter;

    QVector<VertexData> createCircle(QVector3D center, double radius, int arcs, GLuint color);
};

// Groups CursorDrawer and CursorShadowDrawer into one logical object.
class CursorCompositeDrawer : public CompositeDrawable
{
public:
    explicit CursorCompositeDrawer();

    void setPosition(QPointF position);
    void setColor(const QColor &color);
    void setVisible(bool visible) override;
    void update() override;

private:
    CursorDrawer *m_cursor;
    CursorShadowDrawer *m_shadow;
};

#endif // CURSORDRAWER_H
