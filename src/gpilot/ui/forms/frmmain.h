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
#include "io/connection/abstractconnection.h"
#include "core/communicator/communicator.h"
#include "io/connection/connectionmanager.h"
#include "ui/forms/partials/main/partmainvirtualsettings.h"
#include "core/gcode/gcode.h"
#include "core/globals.h"
#include "core/gcode/loader/abstractgcodeloader.h"
#include "core/gcode/loader/gcodethreadedloader.h"
#include "io/connection/abstractconnection.h"
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
#include "ui/widgets/dropwidget.h"
#include "ui/utils/windowstaskbar.h"
#include "ui/widgets/filedropoverlay.h"
#include "core/utils/programtimeestimator.h"
#include "core/utils/timer.h"
#include "core/core.h"

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
    explicit FrmMain(QWidget *parent = 0);
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
    void openHeightmap();
    void saveHeightmap();
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
    void toggleSpindle(bool checked);
    void overridingGroupToggled(bool checked);
    void spindleGroupToggled(bool checked);
    void jogGroupToggled(bool checked);
    void heightmapGroupToggled(bool checked);
    void keyboardControlToggled(bool checked);
    void populateViewWindowsMenu();
    void populateViewPanelsMenu();
    void visualizerVisibilityChanged(bool visible);
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
    void updateOnStateBehaviorChanged(AbstractStateBehavior *sb);

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
    static const int PROGRESS_MIN_LINES = 10000;
    static const int PROGRESS_STEP = 1000;

    // Ui
    Ui::frmMain *ui;

    QMessageBox* m_senderErrorBox;
#ifdef WINDOWS
    UINT m_taskbarButtonCreatedMessageId;
    WindowsTaskbar m_taskBar;
#endif

    // Heightmap mode flag is a UI concern — the heightmap itself lives in Core.
    bool m_heightmapMode;

    bool m_firstShow = true;

    FileDropOverlay *m_fileDropOverlay = nullptr;

    // Shortcuts to domain objects owned by Core. All return stable refs/
    // pointers so `&program()`, `&heightmap()`, `&viewParser()` can be safely
    // stored by drawers, models and state behaviors for the whole app life.
    GCode& program() { return Core::instance().program(); }
    Heightmap& heightmap() { return Core::instance().heightmap(); }
    GCodeViewParser& viewParser() { return Core::instance().viewParser(); }
    GCodeViewParser& probeParser() { return Core::instance().probeParser(); }
    Timer& timer() { return Core::instance().timer(); }
    ProgramTimeEstimator& timeEstimator() { return Core::instance().timeEstimator(); }
    ConnectionManager& connectionManager() { return Core::instance().connectionManager(); }
    Communicator* communicator() { return Core::instance().communicator(); }
    AbstractConnection* connection() { return Core::instance().connection(); }

    // Partials/Panels
    PartMainVirtualSettings *m_partMainVirtualSettings;
    FrmLog* m_logForm = nullptr;

    // bool m_updateSpindleSpeed;
    // bool m_updateParserStatus;

    // Keyboard
    bool m_absoluteCoordinates;
    //bool m_storedKeyboardControl;

    Configuration &m_configuration;

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
    void testConverter(int converterIndex = 0);
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

    void initializeLogMenu();
    void initializeConsolePanel();
    void initializeJogPanel();
    void initializeControlPanel();
    void initializeStatePanel();
    void initializeSpindlePanel();
    void initializeProgramPanel();
    void initializeHeightmapPanel();
    void initializeOverridesPanel();
    void initializeVisualizerPanel();
    void initializeVirtualSettingsPanel();
    void initializeDockCorners();
    void connectWindowTitleUpdater();

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
    void initializeGCodeLoaderConfiguration();
};

#endif // FRMMAIN_H
