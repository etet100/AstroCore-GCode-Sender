// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PARTMAINHEIGHTMAP_H
#define PARTMAINHEIGHTMAP_H

#include <QWidget>
#include "core/heightmap/configurationheightmap.h"
#include "core/heightmap/interpolator/abstractheightmapinterpolator.h"
#include "ui/utils/uistate.h"

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
        void updateControlsState(const UiState& state);
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
        void newClicked();
        void useHeightmapToggled(bool checked);
        void heightmapModeToggled(bool checked);
        void interpolationModeChanged(Heightmap::InterpolationMode mode);
        void gridParametersChanged(QSize gridSize, PartMainHeightmap::MinMax zMinMax, int probeFeed, QSizeF interpolationStep);
        void openClicked();
        void saveClicked();

    private slots:
        // void newClicked();
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
