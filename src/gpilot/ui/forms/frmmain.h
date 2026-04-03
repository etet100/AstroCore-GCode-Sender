// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QMainWindow>
#include <QTimer>
#include <QStringList>
#include <QList>
#include <QTime>
#include <QMenu>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QProgressDialog>
#include <QGroupBox>
#include <exception>
#ifdef WINDOWS
#include <windows.h>
#endif
#include "io/connection/connection.h"
#include "core/communicator/communicator.h"
#include "io/connection/connectionmanager.h"
#include "ui/forms/partials/main/partmainvirtualsettings.h"
#include "core/gcode/gcode.h"
#include "core/globals.h"
#include "core/gcode/loader/gcodeloader.h"
#include "core/gcode/loader/gcodethreadedloader.h"
#include "io/connection/connection.h"
#include "ui/forms/partials/main/partmainjog.h"
#include "ui/forms/partials/main/partmainprogram.h"
#include "ui/forms/partials/main/partmainstate.h"
#include "ui/forms/partials/main/partmainconsole.h"
#include "ui/forms/partials/main/partmainvisualizer.h"
#include "ui/forms/partials/main/partmainoverride.h"
#include "ui/forms/frmgrblconfigurator.h"
#include "ui/forms/frmlog.h"
#include "core/gcode/parser/gcodeviewparser.h"

#include "utils/interpolation.h"
#include "styledtoolbutton.h"
#include "ui/forms/frmsettings.h"
#include "ui/forms/frmabout.h"
#include "core/scripting/scriptvars.h"
#include "ui/widgets/dropwidget.h"
#include "ui/utils/windowstaskbar.h"
#include "ui/widgets/filedropoverlay.h"
#include "core/utils/programtimeestimator.h"
#include "core/utils/timer.h"

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

struct CentralWidgetConfig {
    QWidget* widget;
    QDockWidget* dock;
    QAction* action;
    QString name;
    QString title;
};

class FrmMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit FrmMain(Configuration& configuration, QWidget *parent = 0);
    ~FrmMain();

    void initializeCommunicator();
    void setLogFormWindow(FrmLog* logForm);

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
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void fileSaveTransformedAs();
    void fileExit();
    void fileSettings();
    void serviceConfigureGRBL();
    void serviceResetGRBLConfiguration();
    void aboutShow();
    void viewLockWindowsToggled(bool checked);
    void viewDarkModeToggled(bool checked);
    void viewCentralProgramToggled(bool checked);
    void viewCentralVisualizerToggled(bool checked);
    void on_actHeightmapOpen2_triggered();
    void on_actHeightmapSave_triggered();
    void clearRecentFiles();
    // UI Scale
    void decreaseUiScale();
    void increaseUiScale();
    void resetUiScale();
    //
    void onFileOpen(QString filePath = "");
    void onFileSend();
    void onFilePause(bool checked);
    void onFileAbort();
    void onFileReset();
    // void on_cmdSpindle_toggled(bool checked);
    void on_cmdSpindle_clicked(bool checked);
    void on_grpOverriding_toggled(bool checked);
    void on_grpSpindle_toggled(bool checked);
    void on_grpJog_toggled(bool checked);
    void on_grpHeightmap_toggled(bool checked);
    void on_chkKeyboardControl_toggled(bool checked);
    void on_menuViewWindows_aboutToShow();
    void on_menuViewPanels_aboutToShow();
    void on_dockVisualizer_visibilityChanged(bool visible);
    void useHeightmapToggled(bool checked);
    void heightmapModeToggled(bool checked);
    void onLoadHeightmapRequested();
    void onMachinePosChanged(QVector3D pos);
    void onWorkPosChanged(QVector3D pos);
    void onMachineStateChanged(MachineState state);
    void onMachineStateReceived(MachineState state);
    void onSenderStateReceived(SenderState state);
    void onSpindleStateReceived(bool state);
    void onFloodStateReceived(bool state);
    void onParserStateReceived(QString state);
    void onPinStateReceived(PinState state);
    void onFeedSpindleSpeedReceived(int feedRate, int spindleSpeed);
    void onSpindleSpeedReceived(int spindleSpeed);
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
    void onConsoleNewCommand(QString command, bool isInternal);
    void updateOnStateBehaviorChanged(StateBehavior *sb);

    void programInsertLines(int current, bool before);
    void programDeleteLines(int from, int to);
    void programEditLines(int from, int to);
    void onTableCellChanged(QModelIndex i1, QModelIndex i2);
    void onTableCurrentChanged(QModelIndex idx1, QModelIndex idx2);
    void onActRecentFileTriggered();
    // void onActSendFromLineTriggered();
    void onSlbSpindleValueUserChanged();
    void onSlbSpindleValueChanged();
//    void onCboCommandReturnPressed();
    void onDockTopLevelChanged(bool topLevel);
    // void onVisualizerCursorPosChanged(QPointF);
    // void onProgramLinesUpdated(int from, int to);
    // void updateHeightmapInterpolationDrawer(bool reset = false);
    void onHeightmapDataChangedByUser();
    void centralWidgetActionTriggered(bool checked);

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    void showEvent(QShowEvent *se) override;
    void hideEvent(QHideEvent *he) override;
    void resizeEvent(QResizeEvent *re) override;
    void closeEvent(QCloseEvent *ce) override;
    void dragEnterEvent(QDragEnterEvent *dee) override;
    void dragLeaveEvent(QDragLeaveEvent *dle) override;
    void dropEvent(QDropEvent *de) override;
    void changeEvent(QEvent *ce) override;
    void moveEvent(QMoveEvent *me) override;
    QMenu *createPopupMenu() override;

private:
    static const int PROGRESSMINLINES = 10000;
    static const int PROGRESSSTEP = 1000;

    // Ui
    Ui::frmMain *ui;

    QMessageBox* m_senderErrorBox;
#ifdef WINDOWS
    UINT m_taskbarButtonCreatedMessageId;
    WindowsTaskbar m_taskBar;
#endif

    // Parsers
    GCodeViewParser m_viewParser;
    GCodeViewParser m_probeParser;

    // Heightmap
    bool m_heightmapMode;
    Heightmap m_heightmap;

    bool m_firstShow = true;

    // @TODO to be moved to separate core class
    ConnectionManager m_connectionManager;
    Connection *m_connection = nullptr;
    Communicator *m_communicator;
    GCode m_program;
    FileDropOverlay *m_fileDropOverlay = nullptr;

    // Partials/Panels
    PartMainVirtualSettings *m_partMainVirtualSettings;
    FrmLog* m_logForm = nullptr;

    // Time estimation
    Timer m_timer;
    ProgramTimeEstimator m_timeEstimator;

    // Flags
    bool m_programLoading;
    bool m_settingsLoading;

    // bool m_updateSpindleSpeed;
    // bool m_updateParserStatus;

    // Keyboard
    bool m_absoluteCoordinates;
    //bool m_storedKeyboardControl;

    Configuration &m_configuration;
    ScriptVars m_scriptVars;

    // Central widget management
    QList<CentralWidgetConfig> m_centralWidgets;
    void initializeCentralWidgets();
    void switchCentralWidget(CentralWidgetConfig* config);
    void restoreCentralWidget();

    // Settings
    void preloadSettings();
    void loadSettings();
    void saveSettings();
    void applySettings();

    // Communication
    // void openPortIfNeeded();
    // QString evaluateCommand(QString command);

    // Parser
    GCodeThreadedLoader *m_visualizerUpdater = nullptr;
    void updateParser();

    // Files/models
    void loadFile(QString filePath);
    // void loadLines(QList<std::string> data);
    void applyLoaderGCode(GCodeLoaderData *data);
    void applyUpdaterGCode(GCodeLoaderData *data);
    bool saveChanges(bool heightmapMode);
    // void clearTable();
    void resetHeightmap();
    void newFile();
    void newHeightmap();

    // Ui
    void updateControlsState();
    void updateLayouts();
    void updateRecentFilesMenus();
    void updateJogTitle();
    void addRecentFile(QString fileName);
    void addRecentHeightmap(QString fileName);
    // QRectF borderRectFromExtremes();
    void updateHeightmapBorderDrawer();
    bool updateHeightmapGrid();
    void updateHeightmapGrid(double arg1);
    bool eventFilter(QObject *obj, QEvent *event) override;
    // void updateCurrentModel(GCodeTableModel *m_currentModel);
    void updateToolPositionAndToolpathShadowing(QVector3D toolPosition);
    // void updateToolpathShadowingOnCheckMode();
    QString lastUsedDirectory();

    // Utility
    QList<LineSegment *> subdivideSegment(LineSegment *segment);
    // void jogStep(QVector3D vector);
    void jogStart(QVector3D vector);
    // void jogContinuous();
    // int buttonSize();
    QString getLineInitCommands(int row);

    static bool actionLessThan(const QAction *a1, const QAction *a2);
    static bool actionTextLessThan(const QAction *a1, const QAction *a2);

    void initializeConnection(ConfigurationConnection::ConnectionMode mode);
    void initializeDockTitles();
    void initializeVisualizer();
    void initializeMainMenu();
    void initializeEventFilter();

    void applySpindleConfiguration(ConfigurationMachine &machineConfiguration);
    void applyRecentFilesConfiguration(ConfigurationUI &uiConfiguration);
    void applyHeightmapConfiguration(ConfigurationHeightmap &heightmapConfiguration);
    void applyOverridesConfiguration(ConfigurationMachine &machineConfiguration);
    void applyUIConfiguration(ConfigurationUI &uiConfiguration);
    void applyJoggingConfiguration(ConfigurationJogging &joggingConfiguration);
    void appendPanel(DropWidget *dockPanel, const QString name, const QString title, QWidget *panel);
    void appendSpacer(DropWidget *dockPanel);
    void addDockableWindow(const QString title, const QString name, QWidget *window, Qt::DockWidgetArea area, Qt::Orientation orientation);
    void restoreDockableLayoutState();
    void initializeUiScaleMenu();
    void setHeightmapPoint(QPoint point, double height);
    void updateUiScaleMenu();
};

typedef QMap<QString, QList<QKeySequence>> ShortcutsMap;

#endif // FRMMAIN_H
