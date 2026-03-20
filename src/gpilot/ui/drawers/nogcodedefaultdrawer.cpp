#include "nogcodedefaultdrawer.h"

NoGcodeDefaultDrawer::NoGcodeDefaultDrawer()
{
}

bool NoGcodeDefaultDrawer::updateData(GLPalette &palette)
{
    // Pentagon: 100x100 rectangle with the 0,0 corner cut off
    //
    //  (0,20)──────────────(0,100)
    //    \                    |
    //   (20,0)─────────────(100,0)
    //                         |
    //                      (100,100)

    const QVector3D vertices[] = {
        { 20,   0, 0},
        {100,   0, 0},
        {100, 100, 0},
        {  0, 100, 0},
        {  0,  20, 0},
    };

    // Outline
    const int lineColor = palette.color(QColor(Qt::darkGray));

    m_lines.clear();
    for (int i = 0; i < 5; i++) {
        m_lines << VertexData(vertices[i],     lineColor, QVector3D(sNan, sNan, sNan));
        m_lines << VertexData(vertices[(i + 1) % 5], lineColor, QVector3D(sNan, sNan, sNan));
    }

    // Filled surface — fan triangulation from vertices[0]
    const int fillColor = palette.color(QColor(60, 60, 60, 80));

    m_triangles.clear();
    for (int i = 1; i <= 3; i++) {
        m_triangles << VertexData(vertices[0],     fillColor, QVector3D(0, 0, 1));
        m_triangles << VertexData(vertices[i],     fillColor, QVector3D(0, 0, 1));
        m_triangles << VertexData(vertices[i + 1], fillColor, QVector3D(0, 0, 1));
    }

    return true;
}

QVector3D NoGcodeDefaultDrawer::maximumExtremes()
{
    return QVector3D(100, 100, 0);
}
