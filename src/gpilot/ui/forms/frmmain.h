// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QMainWindow>
#include <QSettings>
#include <QTimer>
#include <QBasicTimer>
#include <QStringList>
#include <QList>
#include <QTime>
#include <QMenu>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QProgressDialog>
#include <QGroupBox>
#include <exception>
#include <QWinTaskBar/qwintaskbar.h>

#include "io/connection/connection.h"
#include "core/communicator/communicator.h"
#include "io/connection/connectionmanager.h"
#include "ui/drawers/cursordrawer.h"
#include "ui/drawers/tablesurfacedrawer.h"
#include "ui/forms/partials/main/partmainvirtualsettings.h"
#include "core/gcode/gcode.h"
#include "core/globals.h"
#include "core/gcode/loader/gcodeloader.h"

#include "io/connection/connection.h"
#include "ui/forms/partials/main/partmainjog.h"
#include "ui/forms/partials/main/partmainstate.h"
#include "ui/forms/partials/main/partmainconsole.h"
#include "ui/forms/frmgrblconfigurator.h"
#include "core/gcode/parser/gcodeviewparser.h"

#include "ui/drawers/origindrawer.h"
#include "ui/drawers/gcodedrawer.h"
#include "ui/drawers/tooldrawer.h"
#include "ui/drawers/heightmapborderdrawer.h"
#include "ui/drawers/heightmapgriddrawer.h"
#include "ui/drawers/heightmapinterpolationdrawer.h"
#include "ui/drawers/shaderdrawable.h"
#include "ui/drawers/selectiondrawer.h"
#include "ui/drawers/machineboundsdrawer.h"

#include "ui/tables/gcodetablemodel.h"
#include "ui/tables/heightmaptablemodel.h"
#include "ui/tables/gcodeitemdelegate.h"

#include "utils/interpolation.h"

#include "styledtoolbutton.h"
#include "sliderbox.h"

#include "ui/forms/frmsettings.h"
#include "ui/forms/frmabout.h"

#include "scripting/scriptvars.h"
#include "ui/widgets/dropwidget.h"

#ifdef WINDOWS
    // #include <QtWinExtras/QtWinExtras>
    #include "shobjidl.h"
#endif

namespace Ui {
class frmMain;
class frmProgram;
}

class CancelException : public std::exception {
public:
#ifdef Q_OS_MAC
#undef _GLIBCXX_USE_NOEXCEPT
#define _GLIBCXX_USE_NOEXCEPT _NOEXCEPT
#endif

    const char* what() const throw()
    {
        return "Operation was cancelled by user";
    }
};

class frmMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit frmMain(Configuration &configuration, QWidget *parent = 0);
    ~frmMain();

    //void writeConsole(QString command);
    void initializeCommunicator();

signals:
    void responseReceived(QString command, int tableIndex, QString response);
    void statusReceived(QString status);
    void senderStateChanged(int state);
    void machineStateChanged(int state);
    void settingsAboutToLoad();
    void settingsLoaded();
    void settingsAboutToSave();
    void settingsSaved();
    void settingsAboutToShow();
    void settingsAccepted();
    void settingsRejected();
    void settingsSetToDefault();
    void pluginsLoaded();

private slots:
    void on_actFileNew_triggered();
    void on_actFileOpen_triggered();
    void on_actFileSave_triggered();
    void on_actFileSaveAs_triggered();
    void on_actFileSaveTransformedAs_triggered();
    void onActRecentClearTriggered();
    void on_actFileExit_triggered();
    void on_actServiceSettings_triggered();
    void on_actServiceConfigureGRBL_triggered();
    void on_actAbout_triggered();
    // void on_actJogStepNext_triggered();
    // void on_actJogStepPrevious_triggered();
    // void on_actJogFeedNext_triggered();
    // void on_actJogFeedPrevious_triggered();
    void on_actSpindleSpeedPlus_triggered();
    void on_actSpindleSpeedMinus_triggered();
    void on_actViewLockWindows_toggled(bool checked);
    void on_cmdFileOpen_clicked();
    void on_cmdFileSend_clicked();
    void on_cmdFilePause_clicked(bool checked);
    void on_cmdFileAbort_clicked();
    void on_cmdFileReset_clicked();
    // void on_cmdSpindle_toggled(bool checked);
    void on_cmdSpindle_clicked(bool checked);
    void on_cmdTop_clicked();
    void on_cmdFront_clicked();
    void on_cmdLeft_clicked();
    void on_cmdIsometric_clicked();
    void on_cmdRotationCube_clicked();
    void on_cmdToggleProjection_clicked();
    void on_cmdFit_clicked();
    void on_grpOverriding_toggled(bool checked);
    void on_grpSpindle_toggled(bool checked);
    void on_grpJog_toggled(bool checked);
    void on_grpHeightMap_toggled(bool arg1);
    void on_chkKeyboardControl_toggled(bool checked);
    void on_chkHeightMapBorderShow_toggled(bool checked);
    void on_chkHeightMapInterpolationShow_toggled(bool checked);
    void on_chkHeightMapUse_clicked(bool checked);
    void on_chkHeightMapGridShow_toggled(bool checked);
    void on_txtHeightMapBorderX_valueChanged(double arg1);
    void on_txtHeightMapBorderWidth_valueChanged(double arg1);
    void on_txtHeightMapBorderY_valueChanged(double arg1);
    void on_txtHeightMapBorderHeight_valueChanged(double arg1);
    void on_txtHeightMapGridX_valueChanged(double arg1);
    void on_txtHeightMapGridY_valueChanged(double arg1);
    void on_txtHeightMapGridZBottom_valueChanged(double arg1);
    void on_txtHeightMapGridZTop_valueChanged(double arg1);
    void on_txtHeightMapInterpolationStepX_valueChanged(double arg1);
    void on_txtHeightMapInterpolationStepY_valueChanged(double arg1);
    void on_cmdHeightMapMode_toggled(bool checked);
    void on_cmdHeightMapCreate_clicked();
    void on_cmdHeightMapLoad_clicked();
    void on_cmdHeightMapBorderAuto_clicked();
    void on_tblProgram_customContextMenuRequested(const QPoint &pos);
    void on_menuViewWindows_aboutToShow();
    void on_menuViewPanels_aboutToShow();
    void on_dockVisualizer_visibilityChanged(bool visible);

    void onMachinePosChanged(QVector3D pos);
    void onWorkPosChanged(QVector3D pos);
    void onMachineStateChanged(MachineState state);
    void onMachineStateReceived(MachineState state);
    void onSenderStateReceived(SenderState state);
    void onSpindleStateReceived(bool state);
    void onFloodStateReceived(bool state);
    void onParserStateReceived(QString state);
    void onPinStateReceived(QString state);
    void onFeedSpindleSpeedReceived(int feedRate, int spindleSpeed);
    void onSpindleSpeedReceived(int spindleSpeed);
    void onOverridesReceived(int feedOverride, int spindleOverride, int rapidOverride);
    void onAborted();
    void onResponseReceived(QString command, int tableIndex, QString response);
    void onCommandResponseReceived(CommandAttributes commandAttributes);
    void onCommandSent(CommandAttributes commandAttributes);
    // @TODO signal does not make sense, it has to done in other way
    // void onCommandProcessed(int tableIndex, QString response);
    void onConfigurationReceived(PhysicalMachineConfiguration);
    void onToolPositionReceived(QVector3D pos);
    void onTransferCompleted();
    void onConnectionError(QString error);
    void onConsoleNewCommand(QString command);
    void onStateBehaviorChanged(StateBehavior *sb);

    void onTimerConnection();
    void onTableInsertLine();
    void onTableDeleteLines();
    void onTableCellChanged(QModelIndex i1, QModelIndex i2);
    void onTableCurrentChanged(QModelIndex idx1, QModelIndex idx2);
    void onOverridingToggled(bool checked);
    void onOverrideChanged();
    void onActRecentFileTriggered();
    void onActSendFromLineTriggered();
    void onSlbSpindleValueUserChanged();
    void onSlbSpindleValueChanged();
//    void onCboCommandReturnPressed();
    void onDockTopLevelChanged(bool topLevel);
    void onScroolBarAction(int action);
    void onVisualizerCursorPosChanged(QPointF);
    // void onProgramLinesUpdated(int from, int to);
    void updateHeightMapInterpolationDrawer(bool reset = false);
    void placeVisualizerButtons();

protected:
    void showEvent(QShowEvent *se) override;
    void hideEvent(QHideEvent *he) override;
    void resizeEvent(QResizeEvent *re) override;
    void timerEvent(QTimerEvent *) override;
    void closeEvent(QCloseEvent *ce) override;
    void dragEnterEvent(QDragEnterEvent *dee) override;
    void dropEvent(QDropEvent *de) override;
    void changeEvent(QEvent *ce) override;
    void moveEvent(QMoveEvent *me) override;
    QMenu *createPopupMenu() override;

private:
    static const int PROGRESSMINLINES = 10000;
    static const int PROGRESSSTEP = 1000;

    // Ui
    Ui::frmMain *ui;

    QMenu *m_tableMenu;
    QMessageBox* m_senderErrorBox;
#ifdef WINDOWS
    // QWinTaskbarButton *m_taskBarButton;
    // QWinTaskbarProgress *m_taskBarProgress;
    QWinTaskBar m_taskBar;
#endif

    // Parsers
    GCodeViewParser m_viewParser;
    GCodeViewParser m_probeParser;

    // Heightmap
    bool m_heightmapMode;
    Heightmap m_heightmap;

    // Visualizer drawers
    // TODO: Add machine table visualizer
    TableSurfaceDrawer m_tableSurfaceDrawer;
    OriginDrawer *m_originDrawer;
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

    bool m_firstShow = true;

    // @TODO to be moved to separate core class
    ConnectionManager m_connectionManager;
    Connection *m_connection;
    Communicator *m_communicator;
    GCode m_program;
    GCode *m_currentProgram = &m_program;

    // Table models
    GCodeTableModel m_programModel;
    GCodeTableModel m_probeModel;
    GCodeTableModel m_programHeightmapModel;
    GCodeTableModel *m_currentModel;
    HeightmapTableModel m_heightmapModel;
    GCodeItemDelegate m_programItemDelegate;

    // Partials/Panels
    PartMainVirtualSettings *m_partMainVirtualSettings;

    // Filenames
    QString m_settingsFileName;
    QString m_programFileName;
    QString m_heightmapFileName;

    // Timers
    QTimer m_timerConnection;
    QBasicTimer m_timerToolAnimation;
    qint64 m_startTime;

    // Flags
    bool m_programLoading;
    bool m_settingsLoading;
    bool m_fileChanged;
    bool m_heightmapChanged;

    // bool m_updateSpindleSpeed;
    // bool m_updateParserStatus;


    // Current values
    int m_lastDrawnLineIndex;

    // Keyboard
    bool m_absoluteCoordinates;
    //bool m_storedKeyboardControl;

    Configuration &m_configuration;
    ScriptVars m_scriptVars;

    // Settings
    void preloadSettings();
    void loadSettings();
    void saveSettings();
    void applySettings();

    // Communication
    // void openPortIfNeeded();
    // QString evaluateCommand(QString command);

    // Parser
    void updateParser();

    // Files/models
    void loadFile(QString fileName);
    void loadLines(QList<std::string> data);
    void applyLoaderGCode(GCodeLoaderData *data);
    bool saveChanges(bool heightmapMode);
    bool saveProgramToFile(QString fileName, GCode &data);
    void loadHeightmap(QString fileName) {};
    bool saveHeightmap(QString fileName) { return true; };
    void clearTable();
    void resetHeightmap();
    void newFile();
    void newHeightmap();

    // Ui
    void updateControlsState();
    void updateLayouts();
    void updateRecentFilesMenu();
    void updateOverride(SliderBox *slider, int value, char command);
    void updateJogTitle();
    void addRecentFile(QString fileName);
    void addRecentHeightmap(QString fileName);
    QRectF borderRectFromTextboxes();
    QRectF borderRectFromExtremes();
    void updateHeightmapBorderDrawer();
    bool updateHeightmapGrid();
    void updateHeightmapGrid(double arg1);
    void resizeTableHeightmapSections();
    bool eventFilter(QObject *obj, QEvent *event) override;
    // void updateCurrentModel(GCodeTableModel *m_currentModel);
    void updateToolPositionAndToolpathShadowing(QVector3D toolPosition);
    void updateToolpathShadowingOnCheckMode();
    QString lastWorkingDirectory();

    // Utility
    int bufferLength();
    QTime updateProgramEstimatedTime(QList<LineSegment> &lines);
    QList<LineSegment *> subdivideSegment(LineSegment *segment);
    // void jogStep(QVector3D vector);
    void jogStart(QVector3D vector);
    // void jogContinuous();
    // int buttonSize();
    QString getLineInitCommands(int row);

    static bool actionLessThan(const QAction *a1, const QAction *a2);
    static bool actionTextLessThan(const QAction *a1, const QAction *a2);

    void initializeConnection(ConfigurationConnection::ConnectionMode mode);
    void initializeVisualizer();

    void applySpindleConfiguration(ConfigurationMachine &machineConfiguration);
    void applyRecentFilesConfiguration(ConfigurationUI &uiConfiguration);
    void applyHeightmapConfiguration(ConfigurationHeightmap &heightmapConfiguration);
    void applyOverridesConfiguration(ConfigurationMachine &machineConfiguration);
    void applyVisualizerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
    void applyCodeDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
    void applyToolDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
    void applyCursorDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
    void applyTableSurfaceDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
    void applyHeightmapDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration);
    void applyUIConfiguration(ConfigurationUI &uiConfiguration);
    void applyJoggingConfiguration(ConfigurationJogging &joggingConfiguration);
    void appendPanel(DropWidget *dockPanel, const QString name, const QString title, QWidget *panel);
    void appendSpacer(DropWidget *dockPanel);
    void addWindow(const QString title, QWidget *window, Qt::DockWidgetArea area, Qt::Orientation orientation);
    void restoreDockableLayoutState();
};

typedef QMap<QString, QList<QKeySequence>> ShortcutsMap;

#endif // FRMMAIN_H
