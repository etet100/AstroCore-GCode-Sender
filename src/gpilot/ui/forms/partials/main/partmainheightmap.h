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
        void requestNewHeightmap();
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
