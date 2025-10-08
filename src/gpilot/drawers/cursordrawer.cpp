// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "cursordrawer.h"
#include <cmath>
#include <QTime>

using namespace std::chrono;

CursorDrawer::CursorDrawer() : ShaderDrawable()
{
}

bool CursorDrawer::updateData(GLPalette &palette)
{
    const int arcs = 5;

    // Clear data
    m_lines.clear();
    m_points.clear();

    // Prepare vertex
    VertexData vertex;
    vertex.color = palette.color(m_color);
    vertex.start = QVector3D(sNan, sNan, sNan);

    // Draw lines
    for (int i = 0; i < arcs; i++) {
        double x = m_position.x() + DIAMETER / 2 * cos(m_rotationAngle / 180 * M_PI + (2 * M_PI / arcs) * i);
        double y = m_position.y() + DIAMETER / 2 * sin(m_rotationAngle / 180 * M_PI + (2 * M_PI / arcs) * i);

        // Side lines
        vertex.position = QVector3D(x, y, m_distanceFromSurface + m_endLength);
        m_lines.append(vertex);
        vertex.position = QVector3D(x, y, m_distanceFromSurface + LENGTH);
        m_lines.append(vertex);

        // Bottom lines
        vertex.position = QVector3D(m_position.x(), m_position.y(), m_distanceFromSurface);
        m_lines.append(vertex);
        vertex.position = QVector3D(x, y, m_distanceFromSurface + m_endLength);
        m_lines.append(vertex);

        // Top lines
        vertex.position = QVector3D(m_position.x(), m_position.y(), m_distanceFromSurface + LENGTH);
        m_lines.append(vertex);
        vertex.position = QVector3D(x, y, m_distanceFromSurface + LENGTH);
        m_lines.append(vertex);

        // Zero Z lines
        if (m_endLength == 0) {
            vertex.position = QVector3D(m_position.x(), m_position.y(), 0);
            m_lines.append(vertex);
            vertex.position = QVector3D(x, y, 0);
            m_lines.append(vertex);
        }
    }

    // Draw circles
    // Bottom
    m_lines += createCircle(QVector3D(m_position.x(), m_position.y(), m_distanceFromSurface + m_endLength),
                            DIAMETER / 2, 20, vertex.color);

    // Top
    m_lines += createCircle(QVector3D(m_position.x(), m_position.y(), m_distanceFromSurface + LENGTH),
                            DIAMETER / 2, 20, vertex.color);

    // Zero Z circle
    if (m_endLength == 0) {
        m_lines += createCircle(QVector3D(m_position.x(), m_position.y(), 0),
                                DIAMETER / 2, 20, vertex.color);
    }

    return true;
}

void CursorDrawer::setColor(const QColor &color)
{
    m_color = color;
}

void CursorDrawer::rotate()
{
    uint64_t ms = duration_cast<milliseconds>(m_clock.now().time_since_epoch()).count();

    double newAngle = (ms / 10) % 360;
    if (m_rotationAngle != newAngle) {
        m_rotationAngle = newAngle;
        update();
    }
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
    position = QPointF(
        trunc(position.x()),
        trunc(position.y())
    );
    if (m_position != position) {
        m_position = position;
        update();
    }
}
