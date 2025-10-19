// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "cursordrawer.h"
#include <cmath>
#include <QTime>

using namespace std::chrono;

CursorDrawer::CursorDrawer() : ShaderDrawable()
{
    m_toolDiameter = 3;
    m_toolLength = 15;
    m_endLength = 10;
    m_position = QVector3D(0, 0, 0);
    m_color = QColor(0, 0, 0);

    startAnimator();
}

bool CursorDrawer::updateData(GLPalette &palette)
{
    const int arcs = 5;
    const float z = m_position.z() + m_animation;

    // Clear data
    m_lines.clear();
    m_points.clear();

    // Prepare vertex
    VertexData vertex;
    vertex.color = palette.color(m_color);

    // Draw circles
    // Bottom
    m_lines += createCircle(QVector3D(m_position.x(), m_position.y(), z + m_endLength),
                            m_toolDiameter / 2, 20, vertex.color);

    // Top
    m_lines += createCircle(QVector3D(m_position.x(), m_position.y(), z + m_toolLength),
                            m_toolDiameter / 2, 20, vertex.color);

    // Zero Z circle
    if (m_endLength == 0) {
        m_lines += createCircle(QVector3D(m_position.x(), m_position.y(), 0),
                                m_toolDiameter / 2, 20, vertex.color);
    }

    // Draw lines
    for (int i = 0; i < arcs; i++) {
        double x = m_position.x() + m_toolDiameter / 2 * cos((2 * M_PI / arcs) * i);
        double y = m_position.y() + m_toolDiameter / 2 * sin((2 * M_PI / arcs) * i);

        // Side lines
        vertex.position = QVector3D(x, y, z + m_endLength);
        m_lines.append(vertex);
        vertex.position = QVector3D(x, y, z + m_toolLength);
        m_lines.append(vertex);

        // Bottom lines
        vertex.position = QVector3D(m_position.x(), m_position.y(), z);
        m_lines.append(vertex);
        vertex.position = QVector3D(x, y, z + m_endLength);
        m_lines.append(vertex);

        // Top lines
        vertex.position = QVector3D(m_position.x(), m_position.y(), z + m_toolLength);
        m_lines.append(vertex);
        vertex.position = QVector3D(x, y, z + m_toolLength);
        m_lines.append(vertex);
    }

    for (int i = 0; i < arcs; i++) {
        // Zero Z lines
        double x = m_position.x() + m_toolDiameter / 2 * cos((2 * M_PI / arcs) * i);
        double y = m_position.y() + m_toolDiameter / 2 * sin((2 * M_PI / arcs) * i);

        vertex.position = QVector3D(m_position.x(), m_position.y(), 0);
        m_lines.append(vertex);
        vertex.position = QVector3D(x, y, 0);
        m_lines.append(vertex);
    }

    return true;
}

QVector<VertexData> CursorDrawer::createCircle(QVector3D center, double radius, int arcs, int color)
{
    // Vertices
    QVector<VertexData> circle;

    // Prepare vertex
    VertexData vertex;
    vertex.color = color;
    vertex.start = QVector3D(sNan, sNan, sNan);

    // Create line loop
    for (int i = 0; i <= arcs; i++) {
        double angle = 2 * M_PI * i / arcs;
        double x = center.x() + radius * cos(angle);
        double y = center.y() + radius * sin(angle);

        if (i > 1) {
            circle.append(circle.last());
        }
        else if (i == arcs) circle.append(circle.first());

        vertex.position = QVector3D(x, y, center.z());
        circle.append(vertex);
    }

    return circle;
}

void CursorDrawer::setPosition(QPointF position)
{
    QVector3D pos3d(position.x(), position.y(), 0);
    if (m_position != pos3d) {
        m_position = pos3d;
        update();
    }
}

void CursorDrawer::setAnimation(float value)
{
    m_animation = value;
    if (m_visible) {
        update();
    }
}

void CursorDrawer::setVisible(bool visible) {
    m_visible = visible;
    update();
}

void CursorDrawer::startAnimator()
{
    m_animator = new QPropertyAnimation(this, "animation");
    m_animator->setDuration(500);
    m_animator->setStartValue(0);
    m_animator->setEndValue(3);
    m_animator->setEasingCurve(QEasingCurve::InOutSine);
    QObject::connect(m_animator, &QPropertyAnimation::finished, [this]() {
        if (m_animator->direction() == QAbstractAnimation::Forward)
            m_animator->setDirection(QAbstractAnimation::Backward);
        else
            m_animator->setDirection(QAbstractAnimation::Forward);
        m_animator->start();
    });
    m_animator->start();
}


