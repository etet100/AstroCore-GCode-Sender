// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "tooldrawer.h"

ToolDrawer::ToolDrawer()
{
    m_toolDiameter = 3;
    m_toolLength = 15;
    m_toolAngle = 35;
    m_toolPosition = QVector3D(0, 0, 0);
    m_rotationAngle = 0;
}

bool ToolDrawer::updateData(GLPalette &palette)
{
    const int arcs = 4;

    // Clear data
    m_lines.clear();
    m_points.clear();
    m_triangles.clear();

    // Prepare vertex
    VertexData vertex;
    m_color.setAlphaF(0.7);
    vertex.color = palette.color(m_color);
    vertex.start = QVector3D(sNan, sNan, sNan);

    // Draw tool
    switch (m_mode) {
        case ConfigurationVisualizer::ToolType::Modern:
            createTriangles(arcs * 3, vertex);
            break;
        case ConfigurationVisualizer::ToolType::Flat:
            createLines(arcs, vertex);
            break;
        case ConfigurationVisualizer::ToolType::Conic:
            createLines(arcs, vertex);
            break;
    }

    return true;
}

void ToolDrawer::setColor(const QColor &color)
{
    m_color = color;
}

void ToolDrawer::setMode(ConfigurationVisualizer::ToolType mode)
{
    m_mode = mode;
    updateEndLength();
    update();
}

bool ToolDrawer::sort(QMatrix4x4 viewMatrix)
{
    struct TriangleInfo {
        int index;
        float z;
    };
    QVector<TriangleInfo> infos;
    int triangleCount = m_triangles.size() / 3;
    for (int i = 0; i < triangleCount; ++i) {
        QVector3D center = (m_triangles[i*3].position + m_triangles[i*3+1].position + m_triangles[i*3+2].position) / 3.0f;
        QVector3D camCenter = viewMatrix.map(center);
        infos.append({i, camCenter.z()});
    }
    std::sort(infos.begin(), infos.end(), [](const TriangleInfo &a, const TriangleInfo &b) {
        // from farthest to nearest
        return a.z > b.z;
    });
    QVector<VertexData> sorted;
    for (const auto &info : infos) {
        sorted.append(m_triangles[info.index*3]);
        sorted.append(m_triangles[info.index*3+1]);
        sorted.append(m_triangles[info.index*3+2]);
    }
    m_triangles = sorted;

    return true;
}

void ToolDrawer::createLines(const int arcs, VertexData &vertex)
{
    for (int i = 0; i < arcs; i++) {
        double x = m_toolPosition.x() + m_toolDiameter / 2 * cos(m_rotationAngle / 180 * M_PI + (2 * M_PI / arcs) * i);
        double y = m_toolPosition.y() + m_toolDiameter / 2 * sin(m_rotationAngle / 180 * M_PI + (2 * M_PI / arcs) * i);

        // Side lines
        vertex.position = QVector3D(x, y, m_toolPosition.z() + m_endLength);
        m_lines.append(vertex);
        vertex.position = QVector3D(x, y, m_toolPosition.z() + m_toolLength);
        m_lines.append(vertex);

        // Bottom lines
        vertex.position = QVector3D(m_toolPosition.x(), m_toolPosition.y(), m_toolPosition.z());
        m_lines.append(vertex);
        vertex.position = QVector3D(x, y, m_toolPosition.z() + m_endLength);
        m_lines.append(vertex);

        // Top lines
        vertex.position = QVector3D(m_toolPosition.x(), m_toolPosition.y(), m_toolPosition.z() + m_toolLength);
        m_lines.append(vertex);
        vertex.position = QVector3D(x, y, m_toolPosition.z() + m_toolLength);
        m_lines.append(vertex);

        // Zero Z lines
        vertex.position = QVector3D(m_toolPosition.x(), m_toolPosition.y(), 0);
        m_lines.append(vertex);
        vertex.position = QVector3D(x, y, 0);
        m_lines.append(vertex);
    }

    // Draw circles
    // Bottom
    m_lines += createCircle(QVector3D(m_toolPosition.x(), m_toolPosition.y(), m_toolPosition.z() + m_endLength),
                            m_toolDiameter / 2, 20, vertex.color);

    // Top
    m_lines += createCircle(QVector3D(m_toolPosition.x(), m_toolPosition.y(), m_toolPosition.z() + m_toolLength),
                            m_toolDiameter / 2, 20, vertex.color);

    // Zero Z circle
    if (m_endLength == 0) {
        m_lines += createCircle(QVector3D(m_toolPosition.x(), m_toolPosition.y(), 0),
                                m_toolDiameter / 2, 20, vertex.color);
    }
}

void ToolDrawer::createTriangles(const int arcs, VertexData &vertex)
{
    // Prepare circles (top and bottom)
    QVector<QVector3D> bottomCircle;
    QVector<QVector3D> topCircle;
    double angleStep = 2 * M_PI / arcs;

    for (int i = 0; i < arcs; ++i) {
        double angle = m_rotationAngle / 180 * M_PI + angleStep * i;
        double x = m_toolPosition.x() + m_toolDiameter / 2 * cos(angle);
        double y = m_toolPosition.y() + m_toolDiameter / 2 * sin(angle);
        bottomCircle.append(QVector3D(x, y, m_toolPosition.z() + m_endLength));
        topCircle.append(QVector3D(x, y, m_toolPosition.z() + m_toolLength + 0.1));
    }

    auto setTriangleNormal = [](VertexData &a, VertexData &b, VertexData &c) {
        QVector3D normal = QVector3D::normal(a.position, b.position, c.position);
        a.start = normal;
        b.start = normal;
        c.start = normal;
    };

    // Side triangles
    if (m_toolLength > m_endLength) {
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
    }

    // Top cap (fan from center)
    QVector3D topCenter(m_toolPosition.x(), m_toolPosition.y(), m_toolPosition.z() + m_toolLength);
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
        QVector3D tip(m_toolPosition.x(), m_toolPosition.y(), m_toolPosition.z());
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
    } else {
        // Bottom cap (fan from center)
        QVector3D bottomCenter(m_toolPosition.x(), m_toolPosition.y(), m_toolPosition.z() + m_endLength);
        for (int i = 0; i < arcs; ++i) {
            int next = (i + 1) % arcs;
            VertexData v1 = vertex; v1.position = bottomCenter;
            VertexData v2 = vertex; v2.position = bottomCircle[next];
            VertexData v3 = vertex; v3.position = bottomCircle[i];
            setTriangleNormal(v1, v2, v3);
            m_triangles.append(v1); m_triangles.append(v2); m_triangles.append(v3);
        }
    }
}

QVector<VertexData> ToolDrawer::createCircle(QVector3D center, double radius, int arcs, uint color)
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

void ToolDrawer::setToolDiameter(double toolDiameter)
{
    if (m_toolDiameter != toolDiameter) {
        m_toolDiameter = toolDiameter;
        update();
    }
}

void ToolDrawer::setToolLength(double toolLength)
{
    if (m_toolLength != toolLength) {
        m_toolLength = toolLength;
        // Call to update end length in case tool length is less than end length
        updateEndLength();
        update();
    }
}

void ToolDrawer::setToolPosition(const QVector3D &toolPosition)
{
    if (m_toolPosition != toolPosition) {
        m_toolPosition = toolPosition;
        update();
    }
}

void ToolDrawer::setRotationAngle(double rotationAngle)
{
    if (m_rotationAngle != rotationAngle) {
        m_rotationAngle = rotationAngle;
        update();
    }
}

void ToolDrawer::rotate(double angle)
{
    setRotationAngle(normalizeAngle(m_rotationAngle + angle));
}

void ToolDrawer::updateEndLength()
{
    if (m_mode == ConfigurationVisualizer::ToolType::Flat) {
        m_toolAngle = 180;
    }

    m_endLength = m_toolAngle > 0 && m_toolAngle < 180 ? m_toolDiameter / 2 / tan(m_toolAngle / 180 * M_PI / 2) : 0;
    assert(!qIsInf(m_endLength));
    if (m_toolLength < m_endLength) {
        m_toolLength = m_endLength;
    }
}

void ToolDrawer::setToolAngle(double toolAngle)
{
    if (m_toolAngle != toolAngle) {
        m_toolAngle = toolAngle;
        updateEndLength();
        update();
    }
}

double ToolDrawer::normalizeAngle(double angle)
{
    while (angle < 0) { angle += 360; }
    while (angle > 360) { angle -= 360; }

    return angle;
}
