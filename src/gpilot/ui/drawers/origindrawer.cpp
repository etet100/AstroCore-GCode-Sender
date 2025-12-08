#include "origindrawer.h"

OriginDrawer::OriginDrawer()
{
    m_scale = 1.0;
}

void OriginDrawer::setZoom(double zoom)
{
    m_scale = zoom;
    qDebug() << "[OriginDrawer] Set zoom to" << m_scale;
    update();
}

bool OriginDrawer::updateData(GLPalette &palette)
{
    int cx = palette.color(1.0, 0.0, 0.0);
    int cy = palette.color(0.0, 1.0, 0.0);
    int cz = palette.color(0.0, 0.0, 1.0);
    int crect = cx;

    m_lines = QVector<VertexData>()
        // X-axis
        << VertexData(QVector3D(0, 0, 0) * m_scale, cx)
        << VertexData(QVector3D(9, 0, 0) * m_scale, cx)
        << VertexData(QVector3D(10, 0, 0) * m_scale, cx)
        << VertexData(QVector3D(8, 0.5, 0) * m_scale, cx)
        << VertexData(QVector3D(8, 0.5, 0) * m_scale, cx)
        << VertexData(QVector3D(8, -0.5, 0) * m_scale, cx)
        << VertexData(QVector3D(8, -0.5, 0) * m_scale, cx)
        << VertexData(QVector3D(10, 0, 0) * m_scale, cx)

        // Y-axis
        << VertexData(QVector3D(0, 0, 0) * m_scale, cy)
        << VertexData(QVector3D(0, 9, 0) * m_scale, cy)
        << VertexData(QVector3D(0, 10, 0) * m_scale, cy)
        << VertexData(QVector3D(0.5, 8, 0) * m_scale, cy)
        << VertexData(QVector3D(0.5, 8, 0) * m_scale, cy)
        << VertexData(QVector3D(-0.5, 8, 0) * m_scale, cy)
        << VertexData(QVector3D(-0.5, 8, 0) * m_scale, cy)
        << VertexData(QVector3D(0, 10, 0) * m_scale, cy)

        // Z-axis
        << VertexData(QVector3D(0, 0, 0) * m_scale, cz)
        << VertexData(QVector3D(0, 0, 9) * m_scale, cz)
        << VertexData(QVector3D(0, 0, 10) * m_scale, cz)
        << VertexData(QVector3D(0.5, 0, 8) * m_scale, cz)
        << VertexData(QVector3D(0.5, 0, 8) * m_scale, cz)
        << VertexData(QVector3D(-0.5, 0, 8) * m_scale, cz)
        << VertexData(QVector3D(-0.5, 0, 8) * m_scale, cz)
        << VertexData(QVector3D(0, 0, 10) * m_scale, cz)

        // 2x2 rect
        << VertexData(QVector3D(1, 1, 0) * m_scale, crect)
        << VertexData(QVector3D(-1, 1, 0) * m_scale, crect)
        << VertexData(QVector3D(-1, 1, 0) * m_scale, crect)
        << VertexData(QVector3D(-1, -1, 0) * m_scale, crect)
        << VertexData(QVector3D(-1, -1, 0) * m_scale, crect)
        << VertexData(QVector3D(1, -1, 0) * m_scale, crect)
        << VertexData(QVector3D(1, -1, 0) * m_scale, crect)
        << VertexData(QVector3D(1, 1, 0) * m_scale, crect)
        ;

    return true;
}
