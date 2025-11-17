// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "heightmapgriddrawer.h"

HeightMapGridDrawer::HeightMapGridDrawer(Heightmap &model) : m_model(model)
{
    m_pointSize = 4;
}

#define STEPS(x) (trunc(x * 5.0) / 5.0)

void HeightMapGridDrawer::generateLines(int gridPointsY, double min, QPointF startPos, double gridStepX, VertexData vertex, double max, GLPalette &palette, int gridPointsX, double gridStepY)
{
    // Horizontal grid lines
    vertex.color = palette.color(1.0, 0.0, 1.0);// palette.color(0.0, 0.0, 1.0);
    for (int i = 0; i < gridPointsX; i++) {
        for (int j = 1; j < gridPointsY; j++) {
            double value = m_model.valueAt(QPoint(i, j));
            if (qIsNaN(value)) continue;

            vertex.position = QVector3D(startPos.x() + gridStepX * (j - 1), startPos.y() + gridStepY * i, m_model.valueAt(QPoint(i, j - 1)) + 0.1);
            // vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - m_model.valueAt(QPoint(i, j - 1))) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);

            vertex.position = QVector3D(startPos.x() + gridStepX * j, startPos.y() + gridStepY * i, value + 0.1);
            // vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - value) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);
        }
    }

    // Vertical grid lines
    // vertex.color = palette.color(0.0, 0.0, 1.0);
    for (int j = 0; j < gridPointsY; j++) {
        for (int i = 1; i < gridPointsX; i++) {
            double value = m_model.valueAt(QPoint(i, j));
            if (qIsNaN(value)) continue;

            vertex.position = QVector3D(startPos.x() + gridStepX * j, startPos.y() + gridStepY * (i - 1), m_model.valueAt(QPoint(i - 1, j)) + 0.1);
            // vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - m_model.valueAt(QPoint(i - 1, j))) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);

            vertex.position = QVector3D(startPos.x() + gridStepX * j, startPos.y() + gridStepY * i, value + 0.1);
            // vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - value) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);
        }
    }
}

void HeightMapGridDrawer::generateTriangles(int gridPointsY, double min, QPointF startPos, double gridStepX, VertexData vertex, double max, GLPalette &palette, int gridPointsX, double gridStepY)
{
    float alpha = 1.0;

    auto setTriangleNormal = [](VertexData &a, VertexData &b, VertexData &c) {
        QVector3D normal = QVector3D::normal(a.position, b.position, c.position);
        a.start = normal;
        b.start = normal;
        c.start = normal;
    };

    for (int i = 0; i < gridPointsX - 1; i++) {
        for (int j = 0; j < gridPointsY - 1; j++) {
            double v00 = m_model.valueAt(QPoint(i, j));
            double v10 = m_model.valueAt(QPoint(i + 1, j));
            double v01 = m_model.valueAt(QPoint(i, j + 1));
            double v11 = m_model.valueAt(QPoint(i + 1, j + 1));

            if (qIsNaN(v00) || qIsNaN(v10) || qIsNaN(v01) || qIsNaN(v11)) continue;

            QVector3D p00(startPos.x() + gridStepX * j,     startPos.y() + gridStepY * i,     v00);
            QVector3D p10(startPos.x() + gridStepX * j,     startPos.y() + gridStepY * (i+1), v10);
            QVector3D p01(startPos.x() + gridStepX * (j+1), startPos.y() + gridStepY * i,     v01);
            QVector3D p11(startPos.x() + gridStepX * (j+1), startPos.y() + gridStepY * (i+1), v11);

            GLuint c00 = palette.color(QColor::fromHsvF(0.67 * STEPS((max - v00) / (max - min)), 1.0, 1.0, alpha));
            GLuint c10 = palette.color(QColor::fromHsvF(0.67 * STEPS((max - v10) / (max - min)), 1.0, 1.0, alpha));
            GLuint c01 = palette.color(QColor::fromHsvF(0.67 * STEPS((max - v01) / (max - min)), 1.0, 1.0, alpha));
            GLuint c11 = palette.color(QColor::fromHsvF(0.67 * STEPS((max - v11) / (max - min)), 1.0, 1.0, alpha));

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
    int gridPointsX = m_model.gridWidth();
    int gridPointsY = m_model.gridHeight();

    // double gridStepX = gridPointsX > 1 ? m_borderRect.width() / (gridPointsX - 1) : 0;
    // double gridStepY = gridPointsY > 1 ? m_borderRect.height() / (gridPointsY - 1) : 0;
    double gridStepX = m_model.stepWidth();
    double gridStepY = m_model.stepHeight();

    QPointF startPos = m_model.startPos();

    double max = 3;
    double min = -3;

    // Probe path / dots
//     for (int i = 0; i < gridPointsX; i++) {
//         for (int j = 0; j < gridPointsY; j++) {
//             double value = m_model.valueAt(QPoint(i, j));
//             if (qIsNaN(value)) {
//                 vertex.color = palette.color(1.0f, 0.6f, 0.0f);
//                 vertex.position = QVector3D(startPos.x() + gridStepX * j, startPos.y() + gridStepY * i, m_zTop);
//                 m_lines.append(vertex);
//                 vertex.position = QVector3D(startPos.x() + gridStepX * j, startPos.y() + gridStepY * i, m_zBottom);
//                 m_lines.append(vertex);
//             } else {
// //                vertex.color = palette.color(0.0, 0.0, 1.0);\
//                 vertex.color = palette.color(
//                 color.setHsvF(0.67 * (max - m_data->at(i).at(j - 1)) / (max - min), 1.0, 1.0);

//                 vertex.position = QVector3D(startPos.x() + gridStepX * j, startPos.y() + gridStepY * i, value);
//                 m_points.append(vertex);
//             }
//         }
//     }

    generateTriangles(gridPointsY, min, startPos, gridStepX, vertex, max, palette, gridPointsX, gridStepY);
    generateLines(gridPointsY, min, startPos, gridStepX, vertex, max, palette, gridPointsX, gridStepY);

    return true;
}

