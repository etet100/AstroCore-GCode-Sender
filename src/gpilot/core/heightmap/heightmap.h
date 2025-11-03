// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <QSize>
#include <QPointF>

class Heightmap
{
    Q_DISABLE_COPY(Heightmap)

    public:
        Heightmap();
        Heightmap(QSize size);

        // Size of the heightmap grid, not a physical size
        QSize gridSize() const;
        int gridWidth() const;
        int gridHeight() const;

        bool isInside(QPointF ptMM) const;
        QPointF startPos() const { return m_startPos; }
        QSizeF stepSize() const { return m_stepSize; }
        double stepWidth() const { return m_stepSize.width(); }
        double stepHeight() const { return m_stepSize.height(); }
        QPair<int, int> gridIndices(const QPointF& pt_mm) const;
        double valueAt(QPoint pt) const;

    private :
        QSize m_size;
        QPointF m_startPos;
        QPointF m_endPos;
        QSizeF m_stepSize;
        // m_size.x * m_size.y of z values
        double** m_data;
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
