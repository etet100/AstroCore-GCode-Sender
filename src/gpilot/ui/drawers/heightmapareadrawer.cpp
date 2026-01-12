// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "heightmapareadrawer.h"
#include <QPainter>

HeightMapAreaDrawer::HeightMapAreaDrawer() : m_model(nullptr)
{
}

void HeightMapAreaDrawer::setModel(Heightmap& model)
{
    m_model = &model;
    update();
}

bool HeightMapAreaDrawer::updateData(GLPalette& palette)
{
    if (!m_model) return false;

    const QPointF p1 = m_model->startPos();
    const QPointF p2 = m_model->endPos();
    const GLuint color = palette.color(1.0, 0.0, 0.0);

    m_lines = QVector<VertexData>()
        << VertexData(QVector3D(p1.x(), p1.y(), 0), color)
        << VertexData(QVector3D(p1.x(), p2.y(), 0), color)
        << VertexData(QVector3D(p1.x(), p2.y(), 0), color)
        << VertexData(QVector3D(p2.x(), p2.y(), 0), color)
        << VertexData(QVector3D(p2.x(), p2.y(), 0), color)
        << VertexData(QVector3D(p2.x(), p1.y(), 0), color)
        << VertexData(QVector3D(p2.x(), p1.y(), 0), color)
        << VertexData(QVector3D(p1.x(), p1.y(), 0), color);

    generateStartEndMarkers();

    if (m_billboardDrawable.needsUpdateGeometry()) {
        m_billboardDrawable.updateData(palette);
    }

    return true;
}

void HeightMapAreaDrawer::generateStartEndMarkers()
{
    m_billboardDrawable.clearBillboards();

    const QPointF p1 = m_model->startPos();
    const QPointF p2 = m_model->endPos();

    m_billboardDrawable.addBillboard(
        QVector3D(p1.x(), p1.y(), 0) - QVector3D(5, 5, 0),
        new HeightMapAreaBillboardContentData("p1", p1),
        17.0f
    );

    m_billboardDrawable.addBillboard(
        QVector3D(p2.x(), p2.y(), 0) + QVector3D(5, 5, 0),
        new HeightMapAreaBillboardContentData("p2", p2),
        17.0f
    );
}

HeightMapAreaBillboardDrawer::HeightMapAreaBillboardDrawer() : BillboardDrawable() , m_fm(m_font)
{
    m_depthTestEnabled = false;
    m_scaleWithDistance = false;
    m_font.setPixelSize(17);
    m_fm = QFontMetrics(m_font);
}

void HeightMapAreaBillboardDrawer::drawBillboard(QPainter &painter, const QRect &rect, const BillboardContentData *data)
{
    HeightMapAreaBillboardContentData const* cdata = dynamic_cast<HeightMapAreaBillboardContentData const*>(data);

    // Background
    painter.setPen(Qt::transparent);
    painter.setBrush(QColor(0, 0, 0, 200));
    painter.drawRoundedRect(rect, 4, 4);

    // Text
    painter.setFont(m_font);
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    QString text = QString("%1, %2").arg(cdata->pos.x(), 0, 'f', 1).arg(cdata->pos.y(), 0, 'f', 1);
    painter.drawText(
        rect,
        Qt::AlignHCenter | Qt::AlignVCenter,
        text
    );
}

QSize HeightMapAreaBillboardDrawer::measureBillboard(const BillboardContentData *data)
{
    HeightMapAreaBillboardContentData const* cdata = dynamic_cast<HeightMapAreaBillboardContentData const*>(data);
    QString text = QString("%1, %2").arg(cdata->pos.x(), 0, 'f', 1).arg(cdata->pos.y(), 0, 'f', 1);
    int textWidth = m_fm.horizontalAdvance(text);
    int textHeight = m_fm.height();

    return QSize(textWidth + 8, textHeight + 4);
}

QString HeightMapAreaBillboardDrawer::buildCacheKey(const BillboardContentData *data)
{
    HeightMapAreaBillboardContentData const* cdata = dynamic_cast<HeightMapAreaBillboardContentData const*>(data);

    return cdata->id;
}
