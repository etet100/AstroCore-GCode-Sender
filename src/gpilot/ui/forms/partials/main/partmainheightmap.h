// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PARTMAINHEIGHTMAP_H
#define PARTMAINHEIGHTMAP_H

#include <QWidget>
#include "core/config/module/configurationheightmap.h"
#include "core/heightmap/interpolator/heightmapinterpolator.h"

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
        void resetOpenFile();
        void resetUseHeighmap();
        void updateControlsState(bool mainState, bool heightmapMode);
        void setOpenFile(QString filePath);
        QRectF areaRectFromTextboxes();
        void setHeightmapAreaRect(QRectF area);
        void setHeightmap(Heightmap* heightmap);

        struct VisualizationDrawers {
            bool border;
            bool grid;
            bool interpolation;
        };

        struct MinMax {
            double min;
            double max;
        };

    protected:
        void resizeEvent(QResizeEvent *event) override;

    signals:
        void extremesRequired();
        void areaChanged(QRectF area);
        void showVisualizationChanged(PartMainHeightmap::VisualizationDrawers drawers);
        void newHeightmapRequested();
        void loadHeightmapRequested();
        void useHeightmapToggled(bool checked);
        void heightmapModeToggled(bool checked);
        void interpolationModeChanged(Heightmap::InterpolationMode mode);
        void gridParametersChanged(QPoint gridStart, PartMainHeightmap::MinMax zMinMax, int probeFeed, QPoint interpolationStep);

    private slots:
        void on_chkShowArea_toggled(bool checked);
        void on_chkShowProbeGrid_toggled(bool checked);
        void on_chkUseHeightmap_toggled(bool checked);
        void on_chkShowInterpolation_toggled(bool checked);
        void on_cmdHeightMapMode_toggled(bool checked);
        void on_txtInterpolationStepX_valueChanged(double arg1);
        void on_txtGridZTop_valueChanged(double arg1);
        void on_txtGridZBottom_valueChanged(double arg1);
        void on_txtGridX_valueChanged(double arg1);
        void on_txtGridY_valueChanged(double arg1);
        void on_cmdAreaFromGCode_clicked();
        void on_txtInterpolationStepY_valueChanged(double arg1);
        void on_cmdNew_clicked();
        void onAreaChanged();
        void onGridParametersChanged();

    private:
        Ui::partMainHeightmap *ui;
        Heightmap* m_heightmap = nullptr;
        void updateHeightmapGrid(double);
        void updateControlsState();
        void emitAreaChanged();
        void emitShowVisualizationChanged();
        void emitGridParametersChanged();
};

#endif // PARTMAINHEIGHTMAP_H
