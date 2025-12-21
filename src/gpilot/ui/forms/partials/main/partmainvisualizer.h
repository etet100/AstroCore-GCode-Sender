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

namespace Ui {
class partMainVisualizer;
}

class PartMainVisualizer : public QWidget
{
        Q_OBJECT

    public:
        explicit PartMainVisualizer(
            GCode& m_program,
            Heightmap& m_heightmap,
            QWidget* parent = nullptr
        );
        ~PartMainVisualizer();

    private slots:
        void placeVisualizerButtons();
        void onVisualizerCursorPosChanged(QPointF);

    private:
        Ui::partMainVisualizer* ui;
        void addDrawables();

        // Visualizer drawers
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
};

#endif // PARTMAINVISUALIZER_H
