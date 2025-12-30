// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "heightmapgriddrawer.h"

HeightMapGridDrawer::HeightMapGridDrawer() : m_model(*(new Heightmap()))
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
    for (int i = 0; i < gridSize.width(); i++) {
        for (int j = 1; j < gridSize.height(); j++) {
            double value = m_model.valueAt(QPoint(i, j));
            if (qIsNaN(value)) continue;

            vertex.position = QVector3D(startPos.x() + stepSize.width() * (j - 1), startPos.y() + stepSize.height() * i, m_model.valueAt(QPoint(i, j - 1)) + zOffset);
            // vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - m_model.valueAt(QPoint(i, j - 1))) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);

            vertex.position = QVector3D(startPos.x() + stepSize.width() * j, startPos.y() + stepSize.height() * i, value + zOffset);
            // vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - value) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);
        }
    }

    // Vertical grid lines
    // vertex.color = palette.color(0.0, 0.0, 1.0);
    for (int j = 0; j < gridSize.height(); j++) {
        for (int i = 1; i < gridSize.width(); i++) {
            double value = m_model.valueAt(QPoint(i, j));
            if (qIsNaN(value)) continue;

            vertex.position = QVector3D(startPos.x() + stepSize.width() * j, startPos.y() + stepSize.height() * (i - 1), m_model.valueAt(QPoint(i - 1, j)) + zOffset);
            // vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - m_model.valueAt(QPoint(i - 1, j))) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);

            vertex.position = QVector3D(startPos.x() + stepSize.width() * j, startPos.y() + stepSize.height() * i, value + zOffset);
            // vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - value) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);
        }
    }
}

void HeightMapGridDrawer::generateTriangles(QSize gridSize, Heightmap::MinMax minMax, QPointF startPos, QSizeF stepSize, VertexData vertex, GLPalette &palette)
{
    float alpha = 1.0;

    auto setTriangleNormal = [](VertexData &a, VertexData &b, VertexData &c) {
        QVector3D normal = QVector3D::normal(a.position, b.position, c.position);
        a.start = normal;
        b.start = normal;
        c.start = normal;
    };

    double minMaxRange = minMax.max - minMax.min;

    for (int i = 0; i < gridSize.width() - 1; i++) {
        for (int j = 0; j < gridSize.height() - 1; j++) {
            double v00 = m_model.valueAt(QPoint(i, j));
            double v10 = m_model.valueAt(QPoint(i + 1, j));
            double v01 = m_model.valueAt(QPoint(i, j + 1));
            double v11 = m_model.valueAt(QPoint(i + 1, j + 1));

            if (qIsNaN(v00) || qIsNaN(v10) || qIsNaN(v01) || qIsNaN(v11)) continue;

            QVector3D p00(startPos.x() + stepSize.width() * j,     startPos.y() + stepSize.height() * i,     v00);
            QVector3D p10(startPos.x() + stepSize.width() * j,     startPos.y() + stepSize.height() * (i+1), v10);
            QVector3D p01(startPos.x() + stepSize.width() * (j+1), startPos.y() + stepSize.height() * i,     v01);
            QVector3D p11(startPos.x() + stepSize.width() * (j+1), startPos.y() + stepSize.height() * (i+1), v11);

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

void HeightMapGridDrawer::setModel(Heightmap &model)
{
    m_model = model;
    update();
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

    generateTriangles(m_model.gridSize(), m_model.valuesMinMax(), m_model.startPos(), m_model.stepSize(), vertex, palette);
    generateLines(m_model.gridSize(), m_model.valuesMinMax(), m_model.startPos(), m_model.stepSize(), vertex, palette);
    generatePlates(m_model.gridSize(), m_model.valuesMinMax(), m_model.startPos(), m_model.stepSize(), vertex, palette);

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

    for (int j = 0; j < gridSize.height(); j++) {
        double y = startPos.y() + stepSize.height() * j;
        double x = startPos.x();
        for (int i = 1; i < gridSize.width(); i++) {
            double value = m_model.valueAt(QPoint(i, j));

            if (qIsNaN(value)) {
                x += stepSize.width();
                continue;
            }

            // Draw vertical line from surface to label position
            vertex.position = QVector3D(x, y, value);
            m_lines.append(vertex);

            vertex.position = QVector3D(x, y, value + 20.0);
            m_lines.append(vertex);

            // Add billboard label at elevated position
            QString labelText = QString("%1, %2\n%3")
                .arg(i).arg(j).arg(value, 0, 'f', 2);

            // m_billboardDrawable.addBillboard(
            //     QVector3D(x, y, value + 20.0),
            //     labelText,
            //     Qt::white,
            //     25.0f  // Billboard size in pixels
            // );

            if (i == 1 && j == 0) {
                qDebug() << "[HeightMapGridDrawer] First billboard added at" << QVector3D(x, y, value + 20.0) << "label:" << labelText;
            }

            x += stepSize.width();
        }
    }
}
