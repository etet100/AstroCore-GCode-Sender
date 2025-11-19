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

void CursorDrawer::setColor(const QColor &color)
{
    m_color = color;
}

void CursorDrawer::createLines(const float z, const int arcs, VertexData &vertex)
{
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
}

void CursorDrawer::createTriangles(const float z, const int arcs, VertexData &vertex)
{
    // Prepare circles (top and bottom)
    QVector<QVector3D> bottomCircle;
    QVector<QVector3D> topCircle;
    double angleStep = 2 * M_PI / arcs;

    for (int i = 0; i < arcs; ++i) {
        double angle = angleStep * i;
        double x = m_position.x() + m_toolDiameter / 2 * cos(angle);
        double y = m_position.y() + m_toolDiameter / 2 * sin(angle);
        bottomCircle.append(QVector3D(x, y, z + m_endLength));
        topCircle.append(QVector3D(x, y, z + m_toolLength + 0.1));
    }

    auto setTriangleNormal = [](VertexData &a, VertexData &b, VertexData &c) {
        QVector3D normal = QVector3D::normal(a.position, b.position, c.position);
        a.start = normal;
        b.start = normal;
        c.start = normal;
    };

    // Side triangles
    for (int i = 0; i < arcs; ++i) {
        int next = (i + 1) % arcs;
        VertexData v1 = vertex; v1.position = bottomCircle[i];
        VertexData v2 = vertex; v2.position = topCircle[i];
        VertexData v3 = vertex; v3.position = topCircle[next];
        setTriangleNormal(v1, v2, v3);
        m_triangles.append(v1); m_triangles.append(v2); m_triangles.append(v3);
        VertexData v4 = vertex; v4.position = bottomCircle[i];
        VertexData v5 = vertex; v5.position = topCircle[next];
        VertexData v6 = vertex; v6.position = bottomCircle[next];
        setTriangleNormal(v4, v5, v6);
        m_triangles.append(v4); m_triangles.append(v5); m_triangles.append(v6);
    }

    // Top cap (fan from center)
    QVector3D topCenter(m_position.x(), m_position.y(), z + m_toolLength);
    for (int i = 0; i < arcs; ++i) {
        int next = (i + 1) % arcs;
        VertexData v1 = vertex; v1.position = topCenter;
        VertexData v2 = vertex; v2.position = topCircle[i];
        VertexData v3 = vertex; v3.position = topCircle[next];
        setTriangleNormal(v1, v2, v3);
        m_triangles.append(v1); m_triangles.append(v2); m_triangles.append(v3);
    }

    // Sharp tip triangles (if m_endLength > 0)
    if (m_endLength > 0) {
        QVector3D tip(m_position.x(), m_position.y(), z);
        for (int i = 0; i < arcs; ++i) {
            int next = (i + 1) % arcs;
            VertexData v1 = vertex; v1.position = tip;
            VertexData v2 = vertex; v2.position = bottomCircle[i];
            VertexData v3 = vertex; v3.position = bottomCircle[next];
            QVector3D normal = QVector3D::normal(v1.position, v2.position, v3.position);
            v1.start = normal;
            v2.start = normal;
            v3.start = normal;
            m_triangles.append(v1); m_triangles.append(v2); m_triangles.append(v3);
        }
    }
}

bool CursorDrawer::updateData(GLPalette &palette)
{
    const int arcs = 5;
    const float z = m_position.z() + m_animation;

    // Clear data
    m_lines.clear();
    m_triangles.clear();
    m_points.clear();

    // Prepare vertex
    VertexData vertex;
    m_color.setAlphaF(0.5);
    vertex.color = palette.color(m_color);

    // createLines(z, arcs, vertex);
    createTriangles(z, arcs, vertex);

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

QVector<VertexData> CursorDrawer::createCircle(QVector3D center, double radius, int arcs, GLuint color)
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


