// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "heightmapgriddrawer.h"

HeightMapGridDrawer::HeightMapGridDrawer(Heightmap &model) : m_model(model)
{
    m_pointSize = 4;
}

#define STEPS(x) (trunc(x * 10.0) / 10.0)

bool HeightMapGridDrawer::updateData(GLPalette &palette)
{
    // Clear data
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

    // Horizontal grid lines
    vertex.color = palette.color(0.0, 0.0, 1.0);
    for (int i = 0; i < gridPointsX; i++) {
        for (int j = 1; j < gridPointsY; j++) {
            double value = m_model.valueAt(QPoint(i, j));
            if (qIsNaN(value)) continue;

            vertex.position = QVector3D(startPos.x() + gridStepX * (j - 1), startPos.y() + gridStepY * i, m_model.valueAt(QPoint(i, j - 1)));
            vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - m_model.valueAt(QPoint(i, j - 1))) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);

            vertex.position = QVector3D(startPos.x() + gridStepX * j, startPos.y() + gridStepY * i, value);
            vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - value) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);
        }
    }

    // Vertical grid lines
    vertex.color = palette.color(0.0, 0.0, 1.0);
    for (int j = 0; j < gridPointsY; j++) {
        for (int i = 1; i < gridPointsX; i++) {
            double value = m_model.valueAt(QPoint(i, j));
            if (qIsNaN(value)) continue;

            vertex.position = QVector3D(startPos.x() + gridStepX * j, startPos.y() + gridStepY * (i - 1), m_model.valueAt(QPoint(i - 1, j)));
            vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - m_model.valueAt(QPoint(i - 1, j))) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);

            vertex.position = QVector3D(startPos.x() + gridStepX * j, startPos.y() + gridStepY * i, value);
            vertex.color = palette.color(QColor::fromHsvF(0.67 * STEPS((max - value) / (max - min)), 1.0, 1.0));
            m_lines.append(vertex);
        }
    }

    return true;
}

// QPointF HeightMapGridDrawer::gridSize() const
// {
//     return m_gridSize;
// }

// void HeightMapGridDrawer::setGridSize(const QPointF &gridSize)
// {
//     m_gridSize = gridSize;
//     update();
// }
// QRectF HeightMapGridDrawer::borderRect() const
// {
//     return m_borderRect;
// }

// void HeightMapGridDrawer::setBorderRect(const QRectF &borderRect)
// {
//     m_borderRect = borderRect;
//     update();
// }

// double HeightMapGridDrawer::zTop() const
// {
//     return m_zTop;
// }

void HeightMapGridDrawer::setZTop(double zTop)
{
    m_zTop = zTop;
    update();
}

// double HeightMapGridDrawer::zBottom() const
// {
//     return m_zBottom;
// }

void HeightMapGridDrawer::setZBottom(double zBottom)
{
    m_zBottom = zBottom;
    update();
}

// QAbstractTableModel *HeightMapGridDrawer::model() const
// {
//     return m_model;
// }

// void HeightMapGridDrawer::setModel(QAbstractTableModel *model)
// {
//     m_model = model;
//     update();
// }





