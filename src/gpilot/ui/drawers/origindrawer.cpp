#include "origindrawer.h"

OriginDrawer::OriginDrawer()
{
    m_scale = 1.0;
    m_depthTestEnabled = false;
}

void OriginDrawer::setZoom(double zoom)
{
    m_scale = zoom;
    update();
}

void OriginDrawer::setVisible(bool visible)
{
    ShaderDrawable::setVisible(visible);
    m_billboardDrawable.setVisible(visible);
}

void OriginDrawer::toggleVisible()
{
    ShaderDrawable::toggleVisible();
    m_billboardDrawable.setVisible(m_visible);
}

bool OriginDrawer::updateData(GLPalette &palette)
{
    QColor cx = QColor(255, 0, 0);
    QColor cy = QColor(0, 255, 0);
    QColor cz = QColor(0, 0, 255);

    int cxint = palette.color(cx);
    int cyint = palette.color(cy);
    int czint = palette.color(cz);
    int crect = cxint;

    // Axis labels
    m_billboardDrawable.clearBillboards();
    const float ofsFromEnd = 3.0f;
    m_billboardDrawable.addBillboard((QVector3D(10 + ofsFromEnd, 0, 0)) * m_scale, new OriginBillboardContentData("X", cx), 10.0f);
    m_billboardDrawable.addBillboard((QVector3D(0, 10 + ofsFromEnd, 0)) * m_scale, new OriginBillboardContentData("Y", cy), 10.0f);
    m_billboardDrawable.addBillboard((QVector3D(0, 0, 10 + ofsFromEnd)) * m_scale, new OriginBillboardContentData("Z", cz), 10.0f);

    // Axis lines
    m_lines = QVector<VertexData>()
        // X-axis
        << VertexData(QVector3D(0, 0, 0) * m_scale, cxint)
        << VertexData(QVector3D(9, 0, 0) * m_scale, cxint)
        << VertexData(QVector3D(10, 0, 0) * m_scale, cxint)
        << VertexData(QVector3D(8, 0.5, 0) * m_scale, cxint)
        << VertexData(QVector3D(8, 0.5, 0) * m_scale, cxint)
        << VertexData(QVector3D(8, -0.5, 0) * m_scale, cxint)
        << VertexData(QVector3D(8, -0.5, 0) * m_scale, cxint)
        << VertexData(QVector3D(10, 0, 0) * m_scale, cxint)

        // Y-axis
        << VertexData(QVector3D(0, 0, 0) * m_scale, cyint)
        << VertexData(QVector3D(0, 9, 0) * m_scale, cyint)
        << VertexData(QVector3D(0, 10, 0) * m_scale, cyint)
        << VertexData(QVector3D(0.5, 8, 0) * m_scale, cyint)
        << VertexData(QVector3D(0.5, 8, 0) * m_scale, cyint)
        << VertexData(QVector3D(-0.5, 8, 0) * m_scale, cyint)
        << VertexData(QVector3D(-0.5, 8, 0) * m_scale, cyint)
        << VertexData(QVector3D(0, 10, 0) * m_scale, cyint)

        // Z-axis
        << VertexData(QVector3D(0, 0, 0) * m_scale, czint)
        << VertexData(QVector3D(0, 0, 9) * m_scale, czint)
        << VertexData(QVector3D(0, 0, 10) * m_scale, czint)
        << VertexData(QVector3D(0.5, 0, 8) * m_scale, czint)
        << VertexData(QVector3D(0.5, 0, 8) * m_scale, czint)
        << VertexData(QVector3D(-0.5, 0, 8) * m_scale, czint)
        << VertexData(QVector3D(-0.5, 0, 8) * m_scale, czint)
        << VertexData(QVector3D(0, 0, 10) * m_scale, czint)

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
