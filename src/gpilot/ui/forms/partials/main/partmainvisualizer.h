#ifndef PARTMAINVISUALIZER_H
#define PARTMAINVISUALIZER_H

#include <QWidget>
#include <QGraphicsOpacityEffect>
#include "ui/drawers/origindrawer.h"
#include "ui/drawers/gcodedrawer.h"
#include "ui/drawers/tooldrawer.h"
#include "ui/drawers/heightmapareadrawer.h"
#include "ui/drawers/heightmapgriddrawer.h"
#include "ui/drawers/heightmapinterpolationdrawer.h"
#include "ui/drawers/lightsourcedrawer.h"
#include "ui/drawers/selectiondrawer.h"
#include "ui/drawers/machineboundsdrawer.h"
#include "ui/drawers/tablesurfacedrawer.h"
#include "ui/drawers/cursordrawer.h"
#include "ui/drawers/boundingboxdrawer.h"
#include "ui/drawers/nogcodedefaultdrawer.h"
#include "core/gcode/parser/gcodeviewparser.h"
#include "core/heightmap/interpolator/abstractheightmapinterpolator.h"
#include "ui/widgets/glwidget.h"

class ConfigurationVisualizer;
class ConfigurationMachine;
class ProgramTimeEstimator;

namespace Ui {
class partMainVisualizer;
}

class PartMainVisualizer : public QWidget
{
    Q_OBJECT

    public:
        explicit PartMainVisualizer(
            QWidget* parent = nullptr
        );
        ~PartMainVisualizer();
        void applyVisualizerConfiguration(ConfigurationVisualizer &visualizerConfiguration, ConfigurationMachine &machineConfiguration);
        void updateColors();
        void updateGCodeExtremes();
        void fitDrawable();
        void fitCodeDrawer();

        void initDrawables();
        void setProgram(GCode* program, GCodeViewParser* parser);

        void updateCodeDrawer(const QList<int>& indexes);
        void updateCodeDrawer();

        void setToolPosition(QVector3D pos);

        void setParserState(QString state);
        void setPinState(QString state);
        void setSpeedState(QString state);

        void close();

        void setInterpolationData(QVector<QVector<double>> *data, QRectF borderRect);
        void setHeightmapInterpolationVisible(bool visible);
        void setSelectionVisible(bool visible);
        // method has the same name as QWidget::setUpdatesEnabled!
        void setUpdatesEnabled2(bool updatesEnabled);

        // High-level API for program operations
        void loadNewProgram();
        void resetVisualization();
        void updateToolpathHighlighting(int currentRow, int previousRow);
        void updateToolTracking(QVector3D toolPosition, int processedLineIndex);
        void resetLastDrawnLine();
        void finalizeTransfer();

        // High-level API for heightmap operations
        QRectF getCodeDrawerBounds() const;

        // Configuration
        bool isIgnoreZ() const;

        // Export/Debug
        void exportCodeDrawerToFile(const QString& filename);

        // Heighmap drawers manipulation
        void setHeightmap(Heightmap& heightmap);
        void setHeightmapMode(bool enabled);
        void updateHeightmapGrid();
        void updateHeightmapInterpolation(bool reset = false);
        void showHeightmapBorder(bool show);
        void showHeightmapProbeGrid(bool show);
        void showHeightmapInterpolationGrid(bool show);
        void setHeightmapInterpolationMode(Heightmap::InterpolationMode mode);
        void updateHeightmap();

        // Line commands generation helper
        struct SegmentInfo {
            LineSegment* firstSegment;
            LineSegment* lastSegment;
            LineSegment* feedSegment;
            LineSegment* plungeSegment;
        };
        SegmentInfo getSegmentInfoForLine(int lineNumber);

        void showInfoBar(QString text);
        void hideInfoBar();

        void setTimeEstimation(ProgramTimeEstimator& estimator);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    signals:
        void goToCursor(QPointF pos);
        void viewModeChanged(GLWidget::ViewMode mode);
        void editHeightmapPoint(QPoint point);

    private slots:
        void updateCursorDrawer(QPointF);
        void topClicked();
        void frontClicked();
        void leftClicked();
        void rightClicked();
        void isometricClicked();
        void rotationCubeClicked();
        void heightmapClicked();
        void heightmapMarkersClicked();
        void toggleProjectionClicked();
        void toggleOriginClicked();
        void fitClicked();
        void _2dClicked();
        void showButtonInfo(bool hovered);
        void updateBillboardsScreenPositions();
        void toggleToolClicked();
        void toggleLightClicked();
        void toggleBoundingBoxClicked();
        void toggleToolpathClicked();

    private:
        Ui::partMainVisualizer* ui;

        // TODO: Add machine table visualizer
        TableSurfaceDrawer m_tableSurfaceDrawer;
        OriginDrawer m_originDrawer;
        GcodeDrawer *m_codeDrawer = nullptr;
        BoundingBoxDrawer m_boundingBoxDrawer;
        ToolDrawer m_toolDrawer;
        CursorCompositeDrawer m_cursorDrawer;
        HeightMapAreaDrawer m_heightmapBorderDrawer;
        HeightMapGridDrawer m_heightmapGridDrawer;
        HeightMapGridBillboardContentData* m_lastHMGBContentData = nullptr;
        HeightMapInterpolationDrawer m_heightmapInterpolationDrawer;
        SelectionDrawer m_selectionDrawer;
        MachineBoundsDrawer m_machineBoundsDrawer;
        NoGcodeDefaultDrawer m_noGcodeDefaultDrawer;
        LightSourceDrawer m_lightSourceDrawer;
        Heightmap* m_heightmap = nullptr;
        GCode* m_program = nullptr;
        bool m_ignoreZ;
        int m_lastDrawnLineIndex;
        ConfigurationVisualizer::ColorGroups m_colors;

        QGraphicsOpacityEffect* m_infoOpacityEffect;
        QPropertyAnimation* m_infoAnimation;
        QTimer m_lightPosTimer;

        void placeButtons();
        void applyCodeDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration, ConfigurationMachine &machineConfiguration);
        void applyToolDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
        void applyCursorDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
        void applyTableSurfaceDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
        void applyHeightmapDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
        void applyOriginDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
        void applySelectionDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
        void setSelectionEndPosition(QVector3D pos);
        void updateSelection();
        void placeInfoBar();
        void initializeInfoBar();
        void initializeButtons();
};

#endif // PARTMAINVISUALIZER_H
