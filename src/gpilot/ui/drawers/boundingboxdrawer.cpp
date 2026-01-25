#include "boundingboxdrawer.h"

BoundingBoxDrawer::BoundingBoxDrawer() : QObject()
{
    m_pointSize = 20;
    m_flatShading = true;
}

bool BoundingBoxDrawer::updateData(GLPalette &palette)
{
    QVector3D border = QVector3D(0.3 * m_scale, 0.3 * m_scale, 0.3 * m_scale);
    QVector3D min = minimumExtremes() - border;
    QVector3D max = maximumExtremes() + border;

    GLfloat lineColor = palette.color(QColor(Qt::yellow));
    GLfloat pointColor = palette.color(QColor(Qt::blue));

    // generate 12 lines

    m_lines.clear();

    m_lines.append(VertexData(QVector3D(min.x(), min.y(), min.z()), lineColor));
    m_lines.append(VertexData(QVector3D(max.x(), min.y(), min.z()), lineColor));

    m_lines.append(VertexData(QVector3D(max.x(), min.y(), min.z()), lineColor));
    m_lines.append(VertexData(QVector3D(max.x(), max.y(), min.z()), lineColor));

    m_lines.append(VertexData(QVector3D(max.x(), max.y(), min.z()), lineColor));
    m_lines.append(VertexData(QVector3D(min.x(), max.y(), min.z()), lineColor));

    m_lines.append(VertexData(QVector3D(min.x(), max.y(), min.z()), lineColor));
    m_lines.append(VertexData(QVector3D(min.x(), min.y(), min.z()), lineColor));

    m_lines.append(VertexData(QVector3D(min.x(), min.y(), min.z()), lineColor));
    m_lines.append(VertexData(QVector3D(min.x(), min.y(), max.z()), lineColor));

    m_lines.append(VertexData(QVector3D(max.x(), min.y(), min.z()), lineColor));
    m_lines.append(VertexData(QVector3D(max.x(), min.y(), max.z()), lineColor));

    m_lines.append(VertexData(QVector3D(max.x(), max.y(), min.z()), lineColor));
    m_lines.append(VertexData(QVector3D(max.x(), max.y(), max.z()), lineColor));

    m_lines.append(VertexData(QVector3D(min.x(), max.y(), min.z()), lineColor));
    m_lines.append(VertexData(QVector3D(min.x(), max.y(), max.z()), lineColor));

    m_lines.append(VertexData(QVector3D(min.x(), min.y(), max.z()), lineColor));
    m_lines.append(VertexData(QVector3D(max.x(), min.y(), max.z()), lineColor));

    m_lines.append(VertexData(QVector3D(max.x(), min.y(), max.z()), lineColor));
    m_lines.append(VertexData(QVector3D(max.x(), max.y(), max.z()), lineColor));

    m_lines.append(VertexData(QVector3D(max.x(), max.y(), max.z()), lineColor));
    m_lines.append(VertexData(QVector3D(min.x(), max.y(), max.z()), lineColor));

    m_lines.append(VertexData(QVector3D(min.x(), max.y(), max.z()), lineColor));
    m_lines.append(VertexData(QVector3D(min.x(), min.y(), max.z()), lineColor));

    // generate 8 points

    m_points.clear();
    m_points.append(VertexData(QVector3D(min.x(), min.y(), min.z()), pointColor));
    m_points.append(VertexData(QVector3D(max.x(), min.y(), min.z()), pointColor));
    m_points.append(VertexData(QVector3D(max.x(), max.y(), min.z()), pointColor));
    m_points.append(VertexData(QVector3D(min.x(), max.y(), min.z()), pointColor));
    m_points.append(VertexData(QVector3D(min.x(), min.y(), max.z()), pointColor));
    m_points.append(VertexData(QVector3D(max.x(), min.y(), max.z()), pointColor));
    m_points.append(VertexData(QVector3D(max.x(), max.y(), max.z()), pointColor));
    m_points.append(VertexData(QVector3D(min.x(), max.y(), max.z()), pointColor));

    return true;
}

void BoundingBoxDrawer::setViewParser(GCodeViewParser *viewParser)
{
    // do not delete old parser!
    m_viewParser = viewParser;
}

void BoundingBoxDrawer::setZoom(double zoom)
{
    m_scale = zoom;
    update();
}

QVector3D BoundingBoxDrawer::minimumExtremes()
{
    QVector3D v = m_viewParser->getMinimumExtremes();
    // if (m_ignoreZ) {
    //     if (m_ignoreZ) v.setZ(0);
    // }

    return v;
}

QVector3D BoundingBoxDrawer::maximumExtremes()
{
    QVector3D v = m_viewParser->getMaximumExtremes();
    // if (m_ignoreZ) {
    //     v.setZ(0);
    // }

    return v;
}
