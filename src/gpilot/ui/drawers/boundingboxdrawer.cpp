#include "boundingboxdrawer.h"
#include "ui/utils/thememanager.h"
#include <QPainter>

BoundingBoxDrawer::BoundingBoxDrawer() : QObject()
{
    m_pointSize = 20;
    m_flatShading = true;
}

bool BoundingBoxDrawer::updateData(GLPalette &palette)
{
    m_lines.clear();
    if (m_viewParser == nullptr) {
        return false;
    }

    QVector3D rawMin = minimumExtremes();
    QVector3D rawMax = maximumExtremes();

    QVector3D border = QVector3D(0.3 * m_scale, 0.3 * m_scale, 0.3 * m_scale);
    QVector3D min = rawMin - border;
    QVector3D max = rawMax + border;

    GLfloat lineColor = palette.color(QColor(Qt::yellow));
    GLfloat pointColor = palette.color(QColor(Qt::blue));

    // generate 12 lines

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

    // Billboards at min and max XY corners
    m_billboardDrawable.clearBillboards();
    const float scale = ThemeManager::instance().scaleF();
    m_billboardDrawable.addBillboard(
        QVector3D(rawMin.x(), rawMin.y(), rawMin.z()),
        new BoundingBoxBillboardContentData("Min", rawMin.x(), rawMin.y(), rawMin.z()),
        20.0f * scale
    );
    m_billboardDrawable.addBillboard(
        QVector3D(rawMax.x(), rawMax.y(), rawMax.z()),
        new BoundingBoxBillboardContentData("Max", rawMax.x(), rawMax.y(), rawMax.z()),
        20.0f * scale
    );

    if (m_billboardDrawable.needsUpdateGeometry()) {
        m_billboardDrawable.updateData(palette);
    }

    return true;
}

void BoundingBoxDrawer::setVisible(bool visible)
{
    ShaderDrawable::setVisible(visible);
    m_billboardDrawable.setVisible(visible);
}

void BoundingBoxDrawer::toggleVisible()
{
    ShaderDrawable::toggleVisible();
    m_billboardDrawable.setVisible(m_visible);
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

    return v;
}

QVector3D BoundingBoxDrawer::maximumExtremes()
{
    QVector3D v = m_viewParser->getMaximumExtremes();

    return v;
}

BoundingBoxBillboardDrawer::BoundingBoxBillboardDrawer() : BillboardDrawable()
{
    m_scaleWithDistance = false;
    m_depthTestEnabled = false;
}

QSize BoundingBoxBillboardDrawer::measureBillboard(const BillboardContentData *data_)
{
    BoundingBoxBillboardContentData const* data = dynamic_cast<BoundingBoxBillboardContentData const*>(data_);
    assert(data != nullptr);

    QFont labelFont;
    labelFont.setPointSize(12);
    QFont valueFont;
    valueFont.setPointSize(16);

    QFontMetrics fmLabel(labelFont);
    QFontMetrics fmValue(valueFont);

    QString valueLine = QString("%1,%2,%3")
        .arg(data->x, 0, 'f', 2)
        .arg(data->y, 0, 'f', 2)
        .arg(data->z, 0, 'f', 2);

    int w = qMax(fmLabel.horizontalAdvance(data->label), fmValue.horizontalAdvance(valueLine)) + 8;
    int h = fmLabel.height() * 0.8 + fmValue.height() * 0.8;

    return QSize(w, h);
}

QString BoundingBoxBillboardDrawer::buildCacheKey(const BillboardContentData *data_)
{
    BoundingBoxBillboardContentData const* data = dynamic_cast<BoundingBoxBillboardContentData const*>(data_);
    assert(data != nullptr);

    return QString("%1_%2_%3_%4").arg(data->label).arg(data->x, 0, 'f', 2).arg(data->y, 0, 'f', 2).arg(data->z, 0, 'f', 2);
}

void BoundingBoxBillboardDrawer::drawBillboard(QPainter &painter, const QRect &rect, const BillboardContentData *data_)
{
    BoundingBoxBillboardContentData const* data = dynamic_cast<BoundingBoxBillboardContentData const*>(data_);
    assert(data != nullptr);

    QFont labelFont;
    labelFont.setPointSize(12);
    QFont valueFont;
    valueFont.setPointSize(16);

    const QColor bgColor(20, 20, 40, 210);
    const QColor textColor(Qt::white);

    painter.setPen(textColor);
    painter.setBrush(bgColor);
    painter.drawRoundedRect(rect, 5, 5);

    int yPos = rect.y() - 2;

    painter.setFont(labelFont);
    painter.drawText(
        QRect(rect.x(), yPos, rect.width(), rect.height() * 0.4),
        Qt::AlignCenter, data->label);

    yPos += rect.height() * 0.4;

    QString valueLine = QString("%1,%2,%3")
        .arg(data->x, 0, 'f', 2)
        .arg(data->y, 0, 'f', 2)
        .arg(data->z, 0, 'f', 2);

    painter.setFont(valueFont);
    painter.drawText(
        QRect(rect.x(), yPos, rect.width(), rect.height() * 0.6),
        Qt::AlignCenter, valueLine);
}
