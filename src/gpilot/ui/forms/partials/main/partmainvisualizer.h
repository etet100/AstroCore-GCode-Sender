#ifndef PARTMAINVISUALIZER_H
#define PARTMAINVISUALIZER_H

#include <QWidget>
#include "ui/drawers/origindrawer.h"
#include "ui/drawers/gcodedrawer.h"
#include "ui/drawers/tooldrawer.h"
#include "ui/drawers/heightmapborderdrawer.h"
#include "ui/drawers/heightmapgriddrawer.h"
#include "ui/drawers/heightmapinterpolationdrawer.h"
#include "ui/drawers/selectiondrawer.h"
#include "ui/drawers/machineboundsdrawer.h"
#include "ui/drawers/tablesurfacedrawer.h"
#include "ui/drawers/cursordrawer.h"
#include "core/gcode/parser/gcodeviewparser.h"

class ConfigurationVisualizer;
class ConfigurationMachine;

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
        void applyVisualizerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
        void applyCodeDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration, ConfigurationMachine &machineConfiguration);
        void applyToolDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
        void applyCursorDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
        void applyTableSurfaceDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
        void applyHeightmapDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
        void applyOriginDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
        void applySelectionDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
        void updateGCodeExtremes();
        void fitDrawable();
        void fitCodeDrawer();

        void initDrawables();
        void setCodeParser(GCodeViewParser* parser);
        void setProbeParser(GCodeViewParser* parser);

        void updateCodeDrawer(const QList<int>& indexes);
        void updateCodeDrawer();
        void updateCurrentDrawer(const QList<int>& indexes);

        void setToolPosition(QVector3D pos);

        void setEstimatedTime(QTime t);
        void setSpendTime(QTime t);
        QTime spendTime() const;

        void setParserState(QString state);
        void setPinState(QString state);
        void setSpeedState(QString state);

        void reset();

        void setHeightmapMode(bool enabled);
        void updateHeightmapGrid();
        // void updateHeightmapGrid(QRectF rect, int x, int y, double zBottom, double zTop);
        void updateHeightmapInterpolation(bool reset = false);
        void setInterpolationData(QVector<QVector<double>> *data, QRectF borderRect);
        void setInterpolationVisible(bool visible);
        void setSelectionVisible(bool visible);
        // method has the same name as QWidget::setUpdatesEnabled!
        void setUpdatesEnabled2(bool updatesEnabled);

        void useCodeDrawer();
        void useProbeDrawer();

        // High-level API for program operations
        void loadNewProgram();
        void resetVisualization();
        void updateToolpathHighlighting(int currentRow, int previousRow, GCode& program);
        void updateToolTracking(QVector3D toolPosition, int processedLineIndex, GCode& program);
        void resetLastDrawnLine();
        void finalizeTransfer();

        // High-level API for heightmap operations
        void setHeightmapBorderRect(QRectF rect);
        QRectF getCodeDrawerBounds() const;

        // Configuration
        bool isIgnoreZ() const;

        // Parser operations
        GCodeViewParser* getCurrentParser();
        bool isCurrentDrawerProbeMode() const;
        void updateCurrentDrawerGeometry();

        // Export/Debug
        void exportCodeDrawerToFile(const QString& filename);

        // Line commands generation helper
        struct SegmentInfo {
            LineSegment* firstSegment;
            LineSegment* lastSegment;
            LineSegment* feedSegment;
            LineSegment* plungeSegment;
        };
        SegmentInfo getSegmentInfoForLine(int lineNumber);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    signals:
        void goToCursor(QPointF pos);

    private slots:
        void cursorPosChanged(QPointF);
        void topClicked();
        void frontClicked();
        void leftClicked();
        void rightClicked();
        void isometricClicked();
        void rotationCubeClicked();
        void heightmapClicked();
        void toggleProjectionClicked();
        void fitClicked();

    private:
        Ui::partMainVisualizer* ui;

        // TODO: Add machine table visualizer
        TableSurfaceDrawer m_tableSurfaceDrawer;
        OriginDrawer m_originDrawer;
        GcodeDrawer *m_codeDrawer;
        GcodeDrawer *m_probeDrawer;
        GcodeDrawer *m_currentDrawer;
        ToolDrawer m_toolDrawer;
        CursorDrawer m_cursorDrawer;
        HeightMapBorderDrawer m_heightmapBorderDrawer;
        HeightMapGridDrawer m_heightmapGridDrawer;
        HeightMapInterpolationDrawer m_heightmapInterpolationDrawer;
        SelectionDrawer m_selectionDrawer;
        MachineBoundsDrawer m_machineBoundsDrawer;
        Heightmap& m_heightmap;
        GCode& m_program;

        bool m_ignoreZ;
        int m_lastDrawnLineIndex;

        void placeVisualizerButtons();

        // Internal selection management
        void setSelectionEndPosition(QVector3D pos);
        void updateSelection();
};

#endif // PARTMAINVISUALIZER_H
