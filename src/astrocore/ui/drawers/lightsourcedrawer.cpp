// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2024 BTS

#include "lightsourcedrawer.h"

LightSourceDrawer::LightSourceDrawer()
    : m_position(0, 0, 0)
    , m_color(Qt::yellow)
{
}

bool LightSourceDrawer::updateData(GLPalette &palette)
{
    m_lines.clear();
    m_points.clear();
    m_triangles.clear();

    const int rings = 6;
    const int segments = 11;
    const double radius = DIAMETER / 2.0;

    VertexData vertex;
    m_color.setAlphaF(0.7f);
    vertex.color = palette.color(m_color);

    for (int ring = 0; ring < rings; ++ring) {
        double lat0 = M_PI * (-0.5 + (double)ring / rings);
        double lat1 = M_PI * (-0.5 + (double)(ring + 1) / rings);
        double sinLat0 = sin(lat0), cosLat0 = cos(lat0);
        double sinLat1 = sin(lat1), cosLat1 = cos(lat1);

        for (int seg = 0; seg < segments; ++seg) {
            double lon0 = 2.0 * M_PI * (double)seg / segments;
            double lon1 = 2.0 * M_PI * (double)(seg + 1) / segments;

            QVector3D p00(m_position.x() + radius * cosLat0 * cos(lon0),
                          m_position.y() + radius * cosLat0 * sin(lon0),
                          m_position.z() + radius * sinLat0);
            QVector3D p10(m_position.x() + radius * cosLat1 * cos(lon0),
                          m_position.y() + radius * cosLat1 * sin(lon0),
                          m_position.z() + radius * sinLat1);
            QVector3D p01(m_position.x() + radius * cosLat0 * cos(lon1),
                          m_position.y() + radius * cosLat0 * sin(lon1),
                          m_position.z() + radius * sinLat0);
            QVector3D p11(m_position.x() + radius * cosLat1 * cos(lon1),
                          m_position.y() + radius * cosLat1 * sin(lon1),
                          m_position.z() + radius * sinLat1);

            QVector3D n1 = QVector3D::normal(p00, p10, p11);
            VertexData v1 = vertex; v1.position = p00; v1.start = n1;
            VertexData v2 = vertex; v2.position = p10; v2.start = n1;
            VertexData v3 = vertex; v3.position = p11; v3.start = n1;
            m_triangles.append(v1); m_triangles.append(v2); m_triangles.append(v3);

            QVector3D n2 = QVector3D::normal(p00, p11, p01);
            VertexData v4 = vertex; v4.position = p00; v4.start = n2;
            VertexData v5 = vertex; v5.position = p11; v5.start = n2;
            VertexData v6 = vertex; v6.position = p01; v6.start = n2;
            m_triangles.append(v4); m_triangles.append(v5); m_triangles.append(v6);
        }
    }

    return true;
}

void LightSourceDrawer::setPosition(const QVector3D &position)
{
    if (m_position != position) {
        m_position = position;
        update();
    }
}

void LightSourceDrawer::setColor(const QColor &color)
{
    m_color = color;
}

bool LightSourceDrawer::sort(QMatrix4x4 viewMatrix)
{
    struct TriangleInfo {
        int index;
        float z;
    };

    int triangleCount = m_triangles.size() / 3;
    QVector<TriangleInfo> infos;
    infos.reserve(triangleCount);

    for (int i = 0; i < triangleCount; ++i) {
        QVector3D center = (m_triangles[i*3].position + m_triangles[i*3+1].position + m_triangles[i*3+2].position) / 3.0f;
        infos.append({i, viewMatrix.map(center).z()});
    }

    std::sort(infos.begin(), infos.end(), [](const TriangleInfo &a, const TriangleInfo &b) {
        return a.z > b.z;
    });

    QVector<VertexData> sorted;
    sorted.reserve(m_triangles.size());
    for (const auto &info : infos) {
        sorted.append(m_triangles[info.index*3]);
        sorted.append(m_triangles[info.index*3+1]);
        sorted.append(m_triangles[info.index*3+2]);
    }
    m_triangles = sorted;

    return true;
}
