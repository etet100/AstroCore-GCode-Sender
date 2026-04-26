// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <QObject>
#include <QSize>
#include <QPointF>
#include <QSizeF>
#include <QRectF>
#include <QList>

class Heightmap : public QObject
{
    Q_OBJECT
    public:
        enum InterpolationMode {
            NearestNeighbour = 0,
            Linear,
            Bilinear,
            Bicubic,
        };

        enum class ScanMode {
            Rows,
            Columns,
        };

        Heightmap(
            QSize size = QSize(5, 5),
            QPointF startPos = QPointF(0.0, 0.0),
            QSizeF stepSize = QSizeF(10.0, 10.0),
            InterpolationMode interpolationMode = InterpolationMode::Bicubic,
            QObject* parent = nullptr
        );
        Heightmap(QSize size, QPointF startPos, QSizeF stepSize, InterpolationMode interpolationMode, const QList<double>& data, QObject* parent = nullptr);
        // Populate from loaded data (replaces move assignment, emits changed() once)
        void assign(QSize size, QPointF startPos, QSizeF stepSize, InterpolationMode interpolationMode, const QList<double>& data);

        struct MinMax {
            double min;
            double max;
        };

        struct BottomTop {
            double bottom;
            double top;
        };

        // Size of the heightmap grid, not a physical size
        QSize gridSize() const;
        int gridWidth() const;
        int gridHeight() const;
        bool isInside(QPointF ptMM) const;
        QPointF startPos() const { return m_startPos; }
        QPointF endPos() const { return m_endPos; }
        QSizeF stepSize() const { return m_stepSize; }
        double stepWidth() const { return m_stepSize.width(); }
        double stepHeight() const { return m_stepSize.height(); }
        QSizeF interpolationStepSize() const { return m_interpolationStepSize; }
        void setInterpolationStepSize(QSizeF stepSize);
        InterpolationMode interpolationMode() const { return m_interpolationMode; }
        void setInterpolationMode(InterpolationMode mode);
        QRectF area() const { return QRectF(m_startPos, m_endPos); }
        void setArea(QRectF area);
        // Resize grid keeping the current area; recomputes stepSize and resets data to NaN.
        void setGridSize(QSize size);
        MinMax valuesMinMax() const { return m_valuesMinMax; }
        BottomTop zBottomTop() const { return m_zBottomTop; }
        void setZBottomTop(BottomTop zBottomTop);
        int probeFeed() const { return m_probeFeed; }
        void setProbeFeed(int probeFeed);
        QPair<int, int> gridIndices(const QPointF& pt_mm) const;
        QList<QPointF> probePoints(QPointF currentPos, ScanMode mode) const;
        double& at(int x, int y);
        double at(QPoint pt) const;
        double at(int x, int y) const;
        void setHeightAt(QPoint point, double height);
        // Sets all values to NAN
        void reset();
        bool anyHeightSet();
        // Sets the point at (x, y) as zero reference and shifts all heights accordingly
        void setZeroReference(int x, int y);
        // Shifts all heights by the given offset
        void offsetAllPoints(double offset);

        void minMax(double value);

        // Suppress changed() signal until endUpdate() is called. Nested calls are counted.
        void beginUpdate();
        // Resume signal emission; emits changed() once if anything changed inside the block.
        void endUpdate();

    signals:
        void changed();

    private:
        QSize m_size;
        // left-bottom and right-top corners in mm
        QPointF m_startPos;
        QPointF m_endPos;
        QSizeF m_stepSize;
        QSizeF m_interpolationStepSize;
        InterpolationMode m_interpolationMode = InterpolationMode::Bicubic;
        // QRectF m_mapArea;
        BottomTop m_zBottomTop = {NAN, NAN};
        MinMax m_valuesMinMax = {NAN, NAN};
        int m_probeFeed = 100;
        // array m_size.x * m_size.y
        // row-major order: rows -> cols = [height][width]
        QList<double> m_data;
        int m_updateDepth = 0;
        bool m_pendingChange = false;
        void notifyChanged();
        void setSize(QSize size);
        void updateEndPos();

        // just for testing
        void generateRandom();
        void generateSinCos();
        void updateMinMax();
};

#endif // HEIGHTMAP_H


// void frmMain::loadHeightmap(QString fileName)
// {
//     QFile file(fileName);

//     if (!file.open(QIODevice::ReadOnly)) {
//         QMessageBox::critical(this, this->windowTitle(), tr("Can't open file:\n") + fileName);
//         return;
//     }
//     QTextStream textStream(&file);

//     m_settingsLoading = true;

//     // Storing previous values
//     ui->txtHeightMapBorderX->setValue(qQNaN());
//     ui->txtHeightMapBorderY->setValue(qQNaN());
//     ui->txtHeightMapBorderWidth->setValue(qQNaN());
//     ui->txtHeightMapBorderHeight->setValue(qQNaN());

//     ui->txtHeightMapGridX->setValue(qQNaN());
//     ui->txtHeightMapGridY->setValue(qQNaN());
//     ui->txtHeightMapGridZBottom->setValue(qQNaN());
//     ui->txtHeightMapGridZTop->setValue(qQNaN());

//     QList<QString> list = textStream.readLine().split(";");
//     ui->txtHeightMapBorderX->setValue(list[0].toDouble());
//     ui->txtHeightMapBorderY->setValue(list[1].toDouble());
//     ui->txtHeightMapBorderWidth->setValue(list[2].toDouble());
//     ui->txtHeightMapBorderHeight->setValue(list[3].toDouble());

//     list = textStream.readLine().split(";");
//     ui->txtHeightMapGridX->setValue(list[0].toDouble());
//     ui->txtHeightMapGridY->setValue(list[1].toDouble());
//     ui->txtHeightMapGridZBottom->setValue(list[2].toDouble());
//     ui->txtHeightMapGridZTop->setValue(list[3].toDouble());

//     m_settingsLoading = false;

//     updateHeightmapBorderDrawer();

//     m_heightmapModel.clear();   // To avoid probe data wipe message
//     updateHeightmapGrid();

//     list = textStream.readLine().split(";");

//     for (int i = 0; i < m_heightmapModel.rowCount(); i++) {
//         QList<QString> row = textStream.readLine().split(";");
//         for (int j = 0; j < m_heightmapModel.columnCount(); j++) {
//             m_heightmapModel.setData(m_heightmapModel.index(i, j), row[j].toDouble(), Qt::UserRole);
//         }
//     }

//     file.close();

//     ui->txtHeightMap->setText(fileName.mid(fileName.lastIndexOf("/") + 1));
//     m_heightmapFileName = fileName;
//     m_heightmapChanged = false;

//     ui->cboHeightMapInterpolationType->setCurrentIndex(list[0].toInt());
//     ui->txtHeightMapInterpolationStepX->setValue(list[1].toDouble());
//     ui->txtHeightMapInterpolationStepY->setValue(list[2].toDouble());

//     updateHeightMapInterpolationDrawer();
// }

// bool frmMain::saveHeightmap(QString fileName)
// {
//     QFile file(fileName);
//     QDir dir;

//     if (file.exists()) dir.remove(file.fileName());
//     if (!file.open(QIODevice::WriteOnly)) return false;

//     QTextStream textStream(&file);
//     textStream << ui->txtHeightMapBorderX->text() << ";"
//                << ui->txtHeightMapBorderY->text() << ";"
//                << ui->txtHeightMapBorderWidth->text() << ";"
//                << ui->txtHeightMapBorderHeight->text() << "\r\n";
//     textStream << ui->txtHeightMapGridX->text() << ";"
//                << ui->txtHeightMapGridY->text() << ";"
//                << ui->txtHeightMapGridZBottom->text() << ";"
//                << ui->txtHeightMapGridZTop->text() << "\r\n";
//     textStream << ui->cboHeightMapInterpolationType->currentIndex() << ";"
//                << ui->txtHeightMapInterpolationStepX->text() << ";"
//                << ui->txtHeightMapInterpolationStepY->text() << "\r\n";

//     for (int i = 0; i < m_heightmapModel.rowCount(); i++) {
//         for (int j = 0; j < m_heightmapModel.columnCount(); j++) {
//             textStream << m_heightmapModel.data(m_heightmapModel.index(i, j), Qt::UserRole).toString() << ((j == m_heightmapModel.columnCount() - 1) ? "" : ";");
//         }
//         textStream << "\r\n";
//     }

//     file.close();

//     m_heightmapChanged = false;

//     return true;
// }
