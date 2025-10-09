#include "origindrawer.h"

OriginDrawer::OriginDrawer()
{
}

bool OriginDrawer::updateData(GLPalette &palette)
{
    int c1 = palette.color(1.0, 0.0, 0.0);
    int c2 = palette.color(0.0, 1.0, 0.0);
    int c3 = palette.color(0.0, 0.0, 1.0);

    m_lines = QVector<VertexData>()
        // X-axis
        << VertexData(QVector3D(0, 0, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(9, 0, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(10, 0, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(8, 0.5, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(8, 0.5, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(8, -0.5, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(8, -0.5, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(10, 0, 0), c1, QVector3D(sNan, sNan, sNan))

        // Y-axis
        << VertexData(QVector3D(0, 0, 0), c2, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(0, 9, 0), c2, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(0, 10, 0), c2, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(0.5, 8, 0), c2, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(0.5, 8, 0), c2, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(-0.5, 8, 0), c2, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(-0.5, 8, 0), c2, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(0, 10, 0), c2, QVector3D(sNan, sNan, sNan))

        // Z-axis
        << VertexData(QVector3D(0, 0, 0), c3, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(0, 0, 9), c3, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(0, 0, 10), c3, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(0.5, 0, 8), c3, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(0.5, 0, 8), c3, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(-0.5, 0, 8), c3, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(-0.5, 0, 8), c3, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(0, 0, 10), c3, QVector3D(sNan, sNan, sNan))

        // 2x2 rect
        << VertexData(QVector3D(1, 1, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(-1, 1, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(-1, 1, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(-1, -1, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(-1, -1, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(1, -1, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(1, -1, 0), c1, QVector3D(sNan, sNan, sNan))
        << VertexData(QVector3D(1, 1, 0), c1, QVector3D(sNan, sNan, sNan))
        ;

    return true;
}
