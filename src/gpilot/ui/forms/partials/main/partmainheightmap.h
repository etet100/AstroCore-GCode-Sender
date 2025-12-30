// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PARTMAINHEIGHTMAP_H
#define PARTMAINHEIGHTMAP_H

#include <QWidget>
#include "core/config/module/configurationheightmap.h"

namespace Ui {
class partMainHeightmap;
}

class PartMainHeightmap : public QWidget
{
    Q_OBJECT

    public:
        explicit PartMainHeightmap(QWidget *parent = nullptr);
        ~PartMainHeightmap();

        void applyHeightmapConfiguration(ConfigurationHeightmap &configurationHeightmap);
        bool heightmapMode(); // heightmap editing mode??
        bool useMap();
        bool showInterpolationGrid();
        void setGridUpdateEnabled();
        void fileClosed();
        void resetUseHeighmap();
        void updateControlsState(bool mainState, bool heightmapMode);
        void setOpenFile(QString filePath);
        QRectF borderRectFromTextboxes();
        void setHeightmapBorderRect(QRectF);

        struct VisualizationDrawers {
            bool border;
            bool grid;
            bool interpolation;
        };

        struct MinMax {
            double min;
            double max;
        };

    signals:
        void extremesRequired();
        void borderChanged(QRectF);
        void showVisualizationChanged(VisualizationDrawers drawers);
        void newHeightmapRequested();
        void loadHeightmapRequested();
        void useHeightmapToggled(bool checked);
        void heightmapModeToggled(bool checked);
        void gridParametersChanged(QPoint gridStart, MinMax zMinMax, int probeFeed, QPoint interpolationStep);

    private slots:
        void on_chkShowBorder_toggled(bool checked);
        void on_chkShowProbeGrid_toggled(bool checked);
        void on_chkUseHeightmap_toggled(bool checked);
        void on_chkShowInterpolation_toggled(bool checked);
        void on_cmdHeightMapMode_toggled(bool checked);
        void on_txtInterpolationStepX_valueChanged(double arg1);
        void on_txtGridZTop_valueChanged(double arg1);
        void on_txtGridZBottom_valueChanged(double arg1);
        void on_txtGridX_valueChanged(double arg1);
        void on_txtGridY_valueChanged(double arg1);
        void on_cmdAutoBorder_clicked();
        void on_txtBorderX_valueChanged(double arg1);
        void on_txtBorderWidth_valueChanged(double arg1);
        void on_txtBorderY_valueChanged(double arg1);
        void on_txtBorderHeight_valueChanged(double arg1);
        void on_txtInterpolationStepY_valueChanged(double arg1);
        void on_cmdNew_clicked();

    private:
        Ui::partMainHeightmap *ui;
        void updateHeightmapGrid(double);
        void updateControlsState();
        void emitBorderChanged();
        void emitShowVisualizationChanged();
        void emitGridParametersChanged();
};

#endif // PARTMAINHEIGHTMAP_H
