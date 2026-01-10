// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "heightmapgriddrawer.h"
#include "core/heightmap/interpolator/heightmapbilinearinterpolator.h"
#include "core/heightmap/interpolator/heightmaplinearinterpolator.h"
#include "core/heightmap/interpolator/heightmapbicubicinterpolator.h"
#include <QPainter>

HeightMapGridDrawer::HeightMapGridDrawer() : m_model(new Heightmap())
{
    m_pointSize = 4;
}

#define STEPS 25
#define QUANTIZE(x) (floor(x * STEPS) / STEPS)

void HeightMapGridDrawer::generateLines(QSize gridSize, Heightmap::MinMax minMax, QPointF startPos, QSizeF stepSize, VertexData vertex, GLPalette &palette)
{
    const float zOffset = 0.0f;

    // Horizontal grid lines
    vertex.color = palette.color(1.0, 0.0, 1.0);// palette.color(0.0, 0.0, 1.0);
    for (int x = 0; x < gridSize.width(); x++) {
        for (int y = 1; y < gridSize.height(); y++) {
            double value = m_model->at(x, y);
            if (qIsNaN(value)) continue;

            vertex.position = QVector3D(startPos.x() + stepSize.width() * x, startPos.y() + stepSize.height() * (y - 1), m_model->at(x, y - 1) + zOffset);
            // vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - m_model.valueAt(QPoint(i, j - 1))) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);

            vertex.position = QVector3D(startPos.x() + stepSize.width() * x, startPos.y() + stepSize.height() * y, value + zOffset);
            // vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - value) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);
        }
    }

    // Vertical grid lines
    // vertex.color = palette.color(0.0, 0.0, 1.0);
    for (int y = 0; y < gridSize.height(); y++) {
        for (int x = 1; x < gridSize.width(); x++) {
            double value = m_model->at(x, y);
            if (qIsNaN(value)) continue;

            vertex.position = QVector3D(startPos.x() + stepSize.width() * (x - 1), startPos.y() + stepSize.height() * y, m_model->at(x - 1, y) + zOffset);
            // vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - m_model.at(QPoint(i - 1, j))) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);

            vertex.position = QVector3D(startPos.x() + stepSize.width() * x, startPos.y() + stepSize.height() * y, value + zOffset);
            // vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - value) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);
        }
    }
}

void HeightMapGridDrawer::generateTriangles(QSize gridSize, Heightmap::MinMax minMax, QPointF startPos, QSizeF stepSize, VertexData vertex, GLPalette &palette)
{
    float alpha = 1.0;
    HeightmapBicubicInterpolator interpolator(*m_model);

    auto setTriangleNormal = [](VertexData &a, VertexData &b, VertexData &c) {
        QVector3D normal = QVector3D::normal(a.position, b.position, c.position);
        a.start = normal;
        b.start = normal;
        c.start = normal;
    };

    double minMaxRange = minMax.max - minMax.min;

    for (int x = 0; x < gridSize.width() - 1; x++) {
        for (int y = 0; y < gridSize.height() - 1; y++) {
            // Let's split every cell info 5x5 subcells to test interpolator
            const double substep = 0.2;
            for (double x2 = x; x2 < x + 0.9; x2 += substep) {
                for (double y2 = y; y2 < y + 0.9; y2 += substep) {
                    double v00 = qBound(minMax.min, interpolator.interpolate(QPointF(x2, y2)), minMax.max);
                    double v10 = qBound(minMax.min, interpolator.interpolate(QPointF(x2 + substep, y2)), minMax.max);
                    double v01 = qBound(minMax.min, interpolator.interpolate(QPointF(x2, y2 + substep)), minMax.max);
                    double v11 = qBound(minMax.min, interpolator.interpolate(QPointF(x2 + substep, y2 + substep)), minMax.max);

                    if (qIsNaN(v00) || qIsNaN(v10) || qIsNaN(v01) || qIsNaN(v11)) {
                        continue;
                    }

                    QVector3D p00(startPos.x() + stepSize.width() * x2, startPos.y() + stepSize.height() * y2, v00);
                    QVector3D p10(startPos.x() + stepSize.width() * (x2 + substep), startPos.y() + stepSize.height() * y2, v10);
                    QVector3D p01(startPos.x() + stepSize.width() * x2, startPos.y() + stepSize.height() * (y2 + substep), v01);
                    QVector3D p11(startPos.x() + stepSize.width() * (x2 + substep), startPos.y() + stepSize.height() * (y2 + substep), v11);

                    GLuint c00 = palette.color(QColor::fromHsvF(0.67 * QUANTIZE((minMax.max - v00) / minMaxRange), 1.0, 1.0, alpha));
                    GLuint c10 = palette.color(QColor::fromHsvF(0.67 * QUANTIZE((minMax.max - v10) / minMaxRange), 1.0, 1.0, alpha));
                    GLuint c01 = palette.color(QColor::fromHsvF(0.67 * QUANTIZE((minMax.max - v01) / minMaxRange), 1.0, 1.0, alpha));
                    GLuint c11 = palette.color(QColor::fromHsvF(0.67 * QUANTIZE((minMax.max - v11) / minMaxRange), 1.0, 1.0, alpha));

                    VertexData vA, vB, vC;
                    // Triangle 1
                    vA.position = p00; vA.color = c00;
                    vB.position = p10; vB.color = c10;
                    vC.position = p11; vC.color = c11;
                    setTriangleNormal(vA, vB, vC);
                    m_triangles.append(vA);
                    m_triangles.append(vB);
                    m_triangles.append(vC);

                    // Triangle 2
                    vA.position = p00; vA.color = c00;
                    vB.position = p11; vB.color = c11;
                    vC.position = p01; vC.color = c01;
                    setTriangleNormal(vA, vB, vC);
                    m_triangles.append(vA);
                    m_triangles.append(vB);
                    m_triangles.append(vC);
                }
            }
        }
    }
}

void HeightMapGridDrawer::setModel(Heightmap &model)
{
    m_model = &model;
    update();
}

void HeightMapGridDrawer::setVisible(bool visible)
{
    ShaderDrawable::setVisible(visible);
    m_billboardDrawable.setVisible(visible);
}

void HeightMapGridDrawer::toggleVisible()
{
    ShaderDrawable::toggleVisible();
    m_billboardDrawable.setVisible(m_visible);
}

bool HeightMapGridDrawer::updateData(GLPalette &palette)
{
    // Clear data
    m_triangles.clear();
    m_lines.clear();
    m_points.clear();

    // Prepare vertex
    VertexData vertex;
    vertex.start = QVector3D(sNan, sNan, m_pointSize);

    // Calculate grid parameters
    // int gridSize.width() = m_model.gridWidth();
    // int gridSize.height() = m_model.gridHeight();

    // QPointF startPos = m_model.startPos();

    // double max = m_model.maxValue();
    // double min = m_model.minValue();

    // Probe path / dots
//     for (int i = 0; i < gridSize.width(); i++) {
//         for (int j = 0; j < gridSize.height(); j++) {
//             double value = m_model.valueAt(QPoint(i, j));
//             if (qIsNaN(value)) {
//                 vertex.color = palette.color(1.0f, 0.6f, 0.0f);
//                 vertex.position = QVector3D(startPos.x() + stepSize.width() * j, startPos.y() + stepSize.height() * i, m_zTop);
//                 m_lines.append(vertex);
//                 vertex.position = QVector3D(startPos.x() + stepSize.width() * j, startPos.y() + stepSize.height() * i, m_zBottom);
//                 m_lines.append(vertex);
//             } else {
// //                vertex.color = palette.color(0.0, 0.0, 1.0);\
//                 vertex.color = palette.color(
//                 color.setHsvF(0.67 * (max - m_data->at(i).at(j - 1)) / (max - min), 1.0, 1.0);

//                 vertex.position = QVector3D(startPos.x() + stepSize.width() * j, startPos.y() + stepSize.height() * i, value);
//                 m_points.append(vertex);
//             }
//         }
//     }

    generateTriangles(m_model->gridSize(), m_model->valuesMinMax(), m_model->startPos(), m_model->stepSize(), vertex, palette);



    generateLines(m_model->gridSize(), m_model->valuesMinMax(), m_model->startPos(), m_model->stepSize(), vertex, palette);
    generatePlates(m_model->gridSize(), m_model->valuesMinMax(), m_model->startPos(), m_model->stepSize(), vertex, palette);

    // Update billboard drawable
    if (m_billboardDrawable.needsUpdateGeometry()) {
        m_billboardDrawable.updateData(palette);
    }

    return true;
}

void HeightMapGridDrawer::generatePlates(QSize gridSize, Heightmap::MinMax minMax, QPointF startPos, QSizeF stepSize, VertexData vertex, GLPalette &palette)
{
    vertex.color = palette.color(QColor::fromString("yellow"));

    // Clear billboards from previous generation
    m_billboardDrawable.clearBillboards();

    double y = startPos.y();
    for (int y_ = 0; y_ < gridSize.height(); y_++) {
        double x = startPos.x();
        for (int x_ = 0; x_ < gridSize.width(); x_++) {
            double value = m_model->at(x_, y_);

            if (qIsNaN(value)) {
                x += stepSize.width();
                continue;
            }

            // Draw line from surface to label position
            // vertex.position = QVector3D(x, y, value);
            // m_lines.append(vertex);
            // vertex.position = QVector3D(x, y, value + 4.0);
            // m_lines.append(vertex);

            m_billboardDrawable.addBillboard(
                QVector3D(x, y, value + 6.0),
                new HeightMapGridBillboardContentData(
                    QPoint(x_, y_), value,
                    QString("%1, %2\n%3").arg(x_).arg(y_).arg(value, 0, 'f', 2),
                    QColor(11, 22, 17, 200), Qt::white
                ),
                20.0f  // Billboard size in pixels
            );

            x += stepSize.width();
        }
        y += stepSize.height();
    }
}

HeightMapGridBillboardDrawer::HeightMapGridBillboardDrawer() : BillboardDrawable()
{
    m_scaleWithDistance = false;
}

QSize HeightMapGridBillboardDrawer::measureBillboard(const BillboardContentData *data_)
{
    HeightMapGridBillboardContentData const* data = dynamic_cast<HeightMapGridBillboardContentData const*>(data_);
    assert(data != nullptr);

    // Split text into two lines
    QStringList lines = data->text.split('\n');
    if (lines.size() != 2) {
       return QSize(0, 0);
    }

    // Two fonts: smaller for coordinates, larger for value
    QFont smallFont;
    smallFont.setPointSize(14);
    QFont largeFont;
    largeFont.setPointSize(30);

    QFontMetrics fmSmall(smallFont);
    QFontMetrics fmLarge(largeFont);

    // Calculate dimensions
    int maxWidth = 0;
    int totalHeight = 0;

    // Line 1
    maxWidth = qMax(maxWidth, fmSmall.horizontalAdvance(lines[0]));
    totalHeight += fmSmall.height() * 0.8;
    // Line 2
    maxWidth = qMax(maxWidth, fmLarge.horizontalAdvance(lines[1]));
    totalHeight += fmLarge.height() * 0.65;

    return QSize(maxWidth + 8, totalHeight);
}

QString HeightMapGridBillboardDrawer::buildCacheKey(const BillboardContentData *data)
{
    HeightMapGridBillboardContentData const* cdata = dynamic_cast<HeightMapGridBillboardContentData const*>(data);
    assert(cdata != nullptr);

    return cdata->text + "_" + cdata->bgColor.name() + "_" + cdata->textColor.name();
}

void HeightMapGridBillboardDrawer::drawBillboard(QPainter &painter, const QRect &rect, const BillboardContentData *data_)
{
    HeightMapGridBillboardContentData const* data = dynamic_cast<HeightMapGridBillboardContentData const*>(data_);
    assert(data != nullptr);

    // Split text into lines
    QStringList lines = data->text.split('\n');
    if (lines.size() != 2) {
        return;
    }

    // Two fonts: smaller for coordinates, larger for value
    QFont smallFont;
    smallFont.setPointSize(14);
    QFont largeFont;
    largeFont.setPointSize(22);

    painter.setPen(data->textColor);
    painter.setBrush(data->bgColor);
    painter.drawRoundedRect(rect, 5, 5);

    // Draw text centered
    int yPos = rect.y() - 2;

    // Line 1
    painter.setPen(data->textColor.darker(200));
    painter.setFont(smallFont);

    painter.drawText(
        QRect(rect.x(), yPos, rect.width(), rect.height() * 0.4),
        Qt::AlignCenter, lines[0]);

    yPos += rect.height() * 0.4;

    // Line 2
    painter.setPen(data->textColor);
    painter.setFont(largeFont);

    painter.drawText(
        QRect(rect.x(), yPos, rect.width(), rect.height() * 0.6),
        Qt::AlignCenter, lines[1]);
}
