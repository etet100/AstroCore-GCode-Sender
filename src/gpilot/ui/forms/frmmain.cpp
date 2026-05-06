// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024-2026 BTS

#include <QFileDialog>
#include <QTextStream>
#include <QDebug>
#include <QStringList>
#include <QTextBlock>
#include <QTextCursor>
#include <QMessageBox>
#include <QPushButton>
#include <QAbstractButton>
#include <QHash>
#include <QComboBox>
#include <QCheckBox>
#include <QScrollBar>
#include <QShortcut>
#include <QAction>
#include <QLayout>
#include <QDrag>
#include <QMimeData>
#include <QTranslator>
#include <QDockWidget>
#include <QStyleHints>
#include "core/globals.h"
#include "ui/forms/frmmain.h"
#include "ui/forms/frmclosingapp.h"
#include "ui/forms/frmconvertersettings.h"
#include "utils/utils.h"
#include "ui/forms/partials/main/partmainjog.h"
#include "ui/forms/partials/main/partmaincontrol.h"
#include "ui/forms/partials/main/partmainvirtualsettings.h"
#include "ui/forms/modals/dlgeditheightmappoint.h"
#include "ui/forms/modals/dlgeditprogram.h"
#include "ui/utils/thememanager.h"
#include "ui/utils/shortcutsmanager.h"
#include "ui/utils/statecolors.h"
#include "modules/pendant/pendant.h"
#include "modules/pendant/configurationpendant.h"
#include "modules/ai/configurationai.h"
#include "modules/camera/camera.h"
#include "ui/config/uiconfigs.h"
#include "core/macro/macrocompiler.h"
#include "ui_frmmain.h"
#include "ui/widgets/widgetmimedata.h"
#include "ui/widgets/dockabletitle.h"
#include "io/connection/connectionmanager.h"
#include "ui/drawers/vertexdataexporter.h"
#include "core/gcode/loader/gcodethreadedloader.h"
#include "core/gcode/exporter/gcodeexporter.h"
#include "core/gcode/converter/arcstolines.h"
#include "core/gcode/converter/streamconverter.h"
#include "core/gcode/converter/feedrateconverter.h"
#include "core/gcode/converter/fusionrestorerapidmovements.h"
#include "core/gcode/converter/modifyfeedrate.h"
#include "core/gcode/converter/movepath.h"
#include "core/gcode/converter/shakinggcode.h"
#include "core/gcode/converter/applyheightmap.h"
#include "core/gcode/converter/stripcomments.h"
#include "core/heightmap/configurationheightmap.h"
#include "core/heightmap/loader/heightmaploader.h"
#include "core/heightmap/exporter/heightmapexporter.h"
#include "core/utils/filesmanager.h"
#include "modules/ai/openaimanager.h"
#include "core/state_behavior/action.h"
#include "core/state_behavior/runningbehavior.h"
#include "core/state_behavior/userpromptbehavior.h"
#include "ui/utils/uipermissions.h"

#define FILE_FILTER_TEXT "G-Code files (*.nc *.ncc *.ngc *.tap *.gc *.gcode *.txt)"

FrmMain::FrmMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmMain),
#ifdef WINDOWS
    m_taskbarButtonCreatedMessageId(RegisterWindowMessage(L"TaskbarButtonCreated")),
    m_taskBar(this),
#endif
    m_configuration(Core::instance().configuration())
{
    ui->setupUi(this);

    initializeGCodeLoaderConfiguration();
    initializeLogMenu();
    initializeDockTitles();
    initializeUiScaleMenu();
    preloadSettings();
    Utils::setVisualMode(this, UiConfigs::instance().ui().darkTheme());
    initializeCommunicator();

    // Panels
    initializeConsolePanel();
    initializeJogPanel();
    initializeControlPanel();
    initializeStatePanel();
    initializeSpindlePanel();
    initializeProgramPanel();
    initializeHeightmapPanel();
    initializeOverridesPanel();
    initializeMacrosPanel();
    initializeCoordinatesPanel();

    // Drag&drop placeholders
    ui->fraDropDevice->setVisible(false);
    ui->fraDropModification->setVisible(false);
    ui->fraDropUser->setVisible(false);

    connectWindowTitleUpdater();
    connectFilesManagerToUpdateControls();

    // Initialize OpenAI key
    if (ConfigurationAI::instance().openAIKey() != "") {
        OpenAIManager::instance(ConfigurationAI::instance().openAIKey());
    }

    m_heightmapMode = false;
    program().resetProcessed();
    //updateCurrentModel(&m_programModel);

    initializeDockCorners();

    foreach (StyledToolButton* button, this->findChildren<StyledToolButton*>(QRegularExpression("cmdUser\\d"))) {
        connect(button, SIGNAL(clicked(bool)), this, SLOT(onCmdUserClicked(bool)));
    }

    initializeVisualizerPanel();

    m_senderErrorBox = new QMessageBox(QMessageBox::Warning, qApp->applicationDisplayName(), QString(),
                                       QMessageBox::Ignore | QMessageBox::Abort, this);
    m_senderErrorBox->setCheckBox(new QCheckBox(tr("Don't show again")));

    // Loading settings
    loadSettings();

    updateControlsState();

    // Setting up spindle slider box
    // ui->slbSpindle->setTitle(tr("Speed:"));
    // ui->slbSpindle->setCheckable(false);
    // ui->slbSpindle->setChecked(true);
    // connect(ui->slbSpindle, &SliderBox::valueUserChanged, this, &FrmMain::onSlbSpindleValueUserChanged);
    // connect(ui->slbSpindle, &SliderBox::valueChanged, this, &FrmMain::onSlbSpindleValueChanged);

    // Enable form actions
    // QList<QAction*> noActions;
    // noActions << ui->actJogXMinus << ui->actJogXPlus
    //           << ui->actJogYMinus << ui->actJogYPlus
    //           << ui->actJogZMinus << ui->actJogZPlus;
    // foreach (QAction* a, findChildren<QAction*>()) if (!noActions.contains(a)) addAction(a);

    // Handle file drop
    if (qApp->arguments().count() > 1 && Utils::isGCodeFile(qApp->arguments().last())) {
        loadFile(qApp->arguments().last());
    }

    initializeEventFilter();

    // Pendant
    Pendant *pendant = new Pendant(m_configuration, *communicator(), this);

    initializeVirtualSettingsPanel();

    updateLayouts();

    // Initialize central widget management, do it before adding dockable windows (camera)
    initializeCentralWidgets();

    // Camera
    addDockableWindow(
        "Camera",
        "camera",
        new Camera(this),
        Qt::TopDockWidgetArea,
        Qt::Horizontal
    );

    restoreCentralWidget();

    // After everything is set up, restore layout
    restoreDockableLayoutState();
}

FrmMain::~FrmMain()
{
    // Communicator and current connection are owned by Core and destroyed
    // with the singleton at program exit.
    delete m_senderErrorBox;
    delete ui; ui = nullptr;
}

void FrmMain::initializeGCodeLoaderConfiguration()
{
    GCodeLoaderConfiguration::setCurrent(m_configuration.parserModule());
}

void FrmMain::initializeCommunicator()
{
    //program() = new GCode();
    // @TODO temporary!
    // communicator()->streamCommands(program());

    connect(communicator(), &Communicator::machinePosChanged, this, &FrmMain::onMachinePosChanged);
    connect(communicator(), &Communicator::workPosChanged, this, &FrmMain::onWorkPosChanged);
    connect(communicator(), &Communicator::machineStateReceived, this, &FrmMain::onMachineStateReceived);
    connect(communicator(), &Communicator::machineStatusReportReceived, this, [this](MachineStatusReport report) {
        ui->state->setMachineStateReport(report.toMarkdown());
    });
    connect(communicator(), &Communicator::machineStateChanged, this, &FrmMain::onMachineStateChanged);
    connect(communicator(), &Communicator::senderStateReceived, this, &FrmMain::onSenderStateReceived);
    connect(communicator(), SIGNAL(spindleStateReceived(bool)), this, SLOT(onSpindleStateReceived(bool)));
    connect(communicator(), &Communicator::floodStateReceived, this, &FrmMain::onFloodStateReceived);
    connect(communicator(), &Communicator::commandSent, this, &FrmMain::onCommandSent);
    connect(communicator(), &Communicator::commandResponseReceived, this, &FrmMain::onCommandResponseReceived);
    connect(communicator(), &Communicator::parserStateReceived, this, &FrmMain::onParserStateReceived);
    connect(communicator(), &Communicator::welcomeMessageReceived, this, [this](QString message) {
        ui->console->appendSystem(message);
    });
    connect(communicator(), &Communicator::log, this, [this](QString message) {
        ui->console->append(message);
    });
    connect(communicator(), &Communicator::pinStateReceived, this, &FrmMain::onPinStateReceived);
    connect(communicator(), &Communicator::spindleSpeedReceived, this, &FrmMain::onSpindleSpeedReceived);
    // connect(communicator(), &Communicator::commandProcessed, this, &FrmMain::onCommandProcessed);
    connect(communicator(), SIGNAL(feedSpindleSpeedReceived(int,int)), this, SLOT(onFeedSpindleSpeedReceived(int,int)));
    connect(communicator()->overrides(), &Overrides::currentValuesChanged, this, [this](int feed, int spindle, int rapid) {
        ui->overrides->setCurrentFeed(feed);
        ui->overrides->setCurrentSpindle(spindle);
        ui->overrides->setCurrentRapid(rapid);
    });
    connect(communicator(), &Communicator::toolPositionReceived, this, &FrmMain::onToolPositionReceived);
    connect(communicator(), &Communicator::transferCompleted, this, &FrmMain::onTransferCompleted);
    connect(communicator(), &Communicator::aborted, this, &FrmMain::onAborted);
    connect(communicator(), &Communicator::machineConfigurationReceived, this, [this](PhysicalMachineConfiguration configuration) {
        m_partMainVirtualSettings->deviceConfigurationReceived(configuration);
    });
    connect(communicator()->stateBehaviorManager(), &StateBehaviorManager::stateBehaviorChanged, this, &FrmMain::updateOnStateBehaviorChanged);
    connect(communicator(), &Communicator::userPromptRequested, this, &FrmMain::onUserPromptRequested);
    connect(communicator(), &Communicator::connectionChanged, this, [this](AbstractConnection *connection) {
        ui->state->setConnectionName(connection->name());
    });
    connect(communicator(), &Communicator::connectionStateChanged, this, [this](ConnectionState state) {
        ui->state->setConnectionState(state == ConnectionState::Connected);
    });
    connect(communicator(), &Communicator::coordinatesCacheChanged, this, [this]() {
        ui->coordinates->updateFromCache(communicator()->positionTracker()->coordinateCache());
    });
    connect(communicator(), &Communicator::modalStateChanged, this, [this](ModalState state) {
        ui->coordinates->setActiveRow(state.coordinateSystem);
    });
}

void FrmMain::setLogFormWindow(FrmLog *logForm)
{
    m_logForm = logForm;
}

void FrmMain::initializeConsolePanel()
{
    ui->console->initialize(UiConfigs::instance().console());
    connect(ui->console, &PartMainConsole::newCommand,
            &Core::instance(), &Core::handleConsoleCommand);
    connect(&Core::instance(), &Core::openFileRequested,
            this, [this]() { onFileOpen(); });
    connect(&Core::instance(), &Core::log, this, [this](QString message) {
        ui->console->append(message);
    });
    ui->console->append(QString("G-Pilot %1 started").arg(qApp->applicationVersion()));
    ui->console->append("---");
}

void FrmMain::initializeJogPanel()
{
    ui->jog->initialize(m_configuration.joggingModule());

    connect(ui->jog, &PartMainJog::jog, this, [this](JoggindDir dir, QVector3D vector) {
        if (dir != JoggindDir::None) {
            ConfigurationJogging& jogging = m_configuration.joggingModule();
            communicator()->stateBehavior()->action(JoggingAction(
                vector,
                jogging.step(),
                jogging.continuous(),
                jogging.feed(),
                jogging.finalFeedZ()
            ));
        }
    });
    connect(ui->jog, &PartMainJog::stop, this, [this]() {
        communicator()->stateBehavior()->action(Action::Abort);
    });
    connect(ui->grpJog, &QGroupBox::toggled, this, &FrmMain::jogGroupToggled);
    // chkKeyboardControl was removed from the UI; keyboardControlToggled() is dead code
}

void FrmMain::initializeMacrosPanel()
{
    connect(ui->grpMacros, &QGroupBox::toggled, this, [this](bool checked) {
        updateLayouts();
        ui->macros->setVisible(checked);
    });
    connect(ui->macros, &PartMainMacros::runMacro, this, [this](int id) {
        const Macros& macros = Core::instance().macros();
        if (id >= macros.size()) {
            return;
        }
        const Macro& macro = macros.at(id);
        communicator()->stateBehavior()->action(RunMacroAction(MacroCompiler::build(macro)));
        qDebug() << "[FrmMain][Macro] Running macro:" << macro.name;
    });
    connect(ui->macros, &PartMainMacros::editMacro, this, [this](int id) {
        qDebug() << "[FrmMain][Macro] Edit, id:" << id;
        Macros& macros = Core::instance().macros();
        if (id >= macros.size()) {
            return;
        }
        Macro& macro = macros.at(id);
        DlgEditProgram dlg(this);
        dlg.setMacro(macro);
        if (dlg.exec() == QDialog::Accepted) {
            dlg.updateMacro(macro);
            macros.notifyUpdated();
        }
    });
    connect(ui->macros, &PartMainMacros::newMacroRequested, this, [this]() {
        qDebug() << "[FrmMain][Macro] New requested";
    });
    connect(&Core::instance().macros(), &Macros::updated, this, [this]() {
        ui->macros->updateMacros(Core::instance().macros());
    });
    ui->macros->updateMacros(Core::instance().macros());
}

void FrmMain::initializeCoordinatesPanel()
{
    connect(ui->grpCoordinates, &QGroupBox::toggled, this, [this](bool checked) {
        updateLayouts();
        ui->coordinates->setVisible(checked);
    });
    connect(ui->coordinates, &PartMainCoordinates::coordinateSystemSelected, this, [this](const QString &cs) {
        communicator()->sendCommand(CommandSource::GeneralUI, cs, TABLE_INDEX_UI);
    });
}

void FrmMain::initializeControlPanel()
{
    connect(ui->control, &PartMainControl::unlock, this, [this]() {
        communicator()->stateBehavior()->action(Action::Unlock);
    });
    connect(ui->control, &PartMainControl::home, this, [this]() {
        communicator()->stateBehavior()->action(Action::Home);
        // communicator()->home();
    });
    connect(ui->control, &PartMainControl::check, this, [this]() {
        communicator()->stateBehavior()->action(Action::CheckMode);
    });
    connect(ui->control, &PartMainControl::abortCheck, this, [this]() {
        communicator()->stateBehavior()->action(Action::Abort);
    });
    connect(ui->control, &PartMainControl::scanTable, this, [this]() {
        AbstractStateBehavior* sb = communicator()->stateBehavior();
        // Scan button doubles as the resume control while a scan-error prompt
        // is on top of a suspended ScanTableBehavior — fire the prompt's
        // "resume" choice. The UserPromptBehavior maps that to resumePrevious().
        if (sb->is(AbstractStateBehavior::Type::UserPrompt)) {
            auto *prompt = static_cast<UserPromptBehavior*>(sb);
            if (prompt->spec().promptId.startsWith("scan.")) {
                communicator()->respondToPrompt(prompt->spec().promptId, "resume");

                return;
            }
        }
        communicator()->stateBehavior()->action(ScanTableAction(&heightmap(), ConfigurationHeightmap::instance().probeFeed()));
    });
    connect(ui->control, &PartMainControl::probe, this, [this](ProbeMode mode) {
        ProbeAction::ProbeParameters params;
        params.doubleProbe = (mode == ProbeMode::Dual);
        communicator()->stateBehavior()->action(ProbeAction(params));
    });
    connect(ui->control, &PartMainControl::reset, this, [this]() {
        // communicator()->reset();
        communicator()->stateBehavior()->action(Action::Reset);
    });
    // connect(ui->control, &partMainControl::command, this, [=](GRBLCommand command) {
    //     qDebug() << "Command: " << command;
    // });
    connect(ui->control, &PartMainControl::zeroZ, this, [this]() {
        communicator()->stateBehavior()->action(Action::ZeroZ);
    });
    connect(ui->control, &PartMainControl::zeroXY, this, [this]() {
        communicator()->stateBehavior()->action(Action::ZeroXY);
    });

    connect(ui->grpControl, &QGroupBox::toggled, this, [this](bool checked) {
        updateLayouts();
        ui->control->setVisible(checked);
    });
}

void FrmMain::initializeStatePanel()
{
    connect(ui->state, &AbstractPartMainState::connectClicked, this, [this]() {
        communicator()->stateBehavior()->action(Action::Connect);
    });
    connect(ui->state, &AbstractPartMainState::disconnectClicked, this, [this]() {
        communicator()->stateBehavior()->action(Action::Disconnect);
    });
    connect(ui->grpState, &QGroupBox::toggled, this, [this](bool checked) {
        updateLayouts();
        ui->state->setVisible(checked);
    });
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](bool dark) {
        AbstractStateBehavior* sb = communicator()->stateBehavior();
        ui->state->setStatusText(
            sb->description(),
            colorForGroup(colorGroupForState(sb->type()), dark),
            dark ? "black" : "white"
        );
    });
}

void FrmMain::initializeSpindlePanel()
{
    connect(ui->grpSpindle, &QGroupBox::toggled, this, [this](bool checked) {
        updateLayouts();
        ui->spindle->setVisible(checked);
    });
    connect(ui->grpSpindle, &QGroupBox::toggled, this, [this](bool checked) {
        ui->grpSpindle->setProperty("overrided", checked);
        style()->unpolish(ui->grpSpindle);
        ui->grpSpindle->ensurePolished();

        if (checked) {
            // if (!ui->grpSpindle->isChecked()) ui->grpSpindle->setTitle(tr("Spindle") + QString(tr(" (%1)")).arg(ui->slbSpindle->value()));
        } else {
            ui->grpSpindle->setTitle(tr("Spindle"));
        }
    });
    connect(ui->grpSpindle, &QGroupBox::toggled, this, &FrmMain::spindleGroupToggled);
    // cmdSpindle lives inside PartMainSpindle sub-panel, not directly in Ui::frmMain
    connect(ui->spindle->findChild<QAbstractButton*>("cmdSpindle"), &QAbstractButton::clicked, this, &FrmMain::toggleSpindle);
}

void FrmMain::initializeProgramPanel()
{
    ui->program->initialize(&program(), &heightmap());
    ui->program->setupFileSendMenu(this, SLOT(onActSendFromLineTriggered()));

    // connect(&program(), &GCode::linesUpdated, this, [this](int fromLine, int toLine) {
    //     Q_UNUSED(fromLine);
    //     Q_UNUSED(toLine);

    //     if (!ui->program->isAutoScroll()) {
    //         return;
    //     }

    //     // int tableIndex = ui->program->getCurrentModelFilteredIndex(program().commandIndex());
    //     // ui->program->scrollToCurrentIndex(ui->program->currentModelIndex(tableIndex, 1));

    //     GCodeViewParser *parser = &viewParser();
    //     QVector<QList<int>> lineIndexes = parser->getLinesIndexes();
    //     QList<LineSegment>& list = parser->getLineSegmentList();
    //     QList<int> indexes;

    //     for (int i = fromLine; i <= toLine; i++) {
    //         // GCodeItem &item = program()[i];
    //         // int j = item.commandNumber;
    //         // if (j != -1) {
    //         //     foreach (int l, lineIndexes.at(j)) {
    //         //         if (item.state == GCodeItem::Sent) {
    //         //             list[l].setIsHightlight(true);
    //         //             list[l].setDrawn(false);
    //         //             indexes.append(l);
    //         //         } else if (item.state == GCodeItem::Processed) {
    //         //             list[l].setIsHightlight(false);
    //         //             list[l].setDrawn(true);
    //         //             indexes.append(l);
    //         //         }
    //         //     }
    //         // }
    //     }

    //     if (!indexes.isEmpty()) {
    //         ui->visualizer->updateCodeDrawer(indexes);
    //     }
    // });
    bindActiveProgram(&program());

    connect(ui->program, &PartMainProgram::viewChanged, this,
            [this](PartMainProgram::View) { updateProgramTitle(); });

    connect(ui->program, &PartMainProgram::clearRecentFiles, this, [this]() {
        Core::instance().clearRecentFiles(m_heightmapMode);
    });
    connect(&Core::instance(), &Core::recentFilesChanged, this, &FrmMain::updateRecentFilesMenus);
    // connect(ui->program, &PartMainProgram::modelDataChanged, this, &FrmMain::onTableCellChanged);
    connect(ui->program, &PartMainProgram::heightmapDataChangedByUser, this, &FrmMain::onHeightmapDataChangedByUser);
    // connect(&program(), &GCode::linesUpdated, this, &FrmMain::onProgramLinesUpdated);
    connect(ui->program, &PartMainProgram::manualScrollRequested, this, [this]() {
        if (communicator()->stateBehavior()->is(AbstractStateBehavior::Type::Running)) {
            ui->program->setAutoScroll(false);
        }
    });
    connect(ui->program, &PartMainProgram::currentChanged, this, &FrmMain::onTableCurrentChanged);
    connect(ui->program, &PartMainProgram::insertLinesRequested, this, &FrmMain::programInsertLines);
    connect(ui->program, &PartMainProgram::deleteLinesRequested, this, &FrmMain::programDeleteLines);
    connect(ui->program, &PartMainProgram::editLinesRequested, this, &FrmMain::programEditLines);

    connect(ui->program, &PartMainProgram::openFile, this, &FrmMain::onFileOpen);
    connect(ui->program, &PartMainProgram::startRequested, this, &FrmMain::onFileSend);
    connect(ui->program, &PartMainProgram::pause, this, &FrmMain::onFilePause);
    connect(ui->program, &PartMainProgram::abortRequested, this, &FrmMain::onFileAbort);
    connect(ui->program, &PartMainProgram::programResetRequested, this, &FrmMain::onFileReset);
}

void FrmMain::initializeHeightmapPanel()
{
    connect(ui->heightmap, &PartMainHeightmap::extremesRequired, this, [this]() {
        ui->heightmap->setHeightmapAreaRect(ui->visualizer->getCodeDrawerBounds());
    });
    connect(ui->heightmap, &PartMainHeightmap::newClicked, this, &FrmMain::newHeightmap);
    connect(ui->heightmap, &PartMainHeightmap::openClicked, this, &FrmMain::openHeightmap);
    connect(ui->heightmap, &PartMainHeightmap::useHeightmapToggled, this, &FrmMain::useHeightmapToggled);
    connect(ui->heightmap, &PartMainHeightmap::heightmapModeToggled, this, &FrmMain::heightmapModeToggled);
    connect(ui->heightmap, &PartMainHeightmap::showVisualizationChanged, this, [this](PartMainHeightmap::VisualizationDrawers drawers) {
        ui->visualizer->showHeightmapBorder(drawers.border);
        ui->visualizer->showHeightmapProbeGrid(drawers.grid);
        ui->visualizer->showHeightmapInterpolationGrid(drawers.interpolation);
    });
    connect(ui->heightmap, &PartMainHeightmap::areaChanged, this, [this](QRectF area) {
        if (area != heightmap().area()) {
            heightmap().setArea(area);
            ui->visualizer->updateHeightmap();
        }
    });
    connect(ui->heightmap, &PartMainHeightmap::gridParametersChanged, this, &FrmMain::onGridParametersChanged);
    connect(ui->heightmap, &PartMainHeightmap::interpolationModeChanged, this, [this](Heightmap::InterpolationMode mode) {
        ui->visualizer->setHeightmapInterpolationMode(mode);
        heightmap().setInterpolationMode(mode);
        ui->visualizer->updateHeightmap();
    });
    connect(ui->grpHeightmap, &QGroupBox::toggled, this, &FrmMain::heightmapGroupToggled);

    // ui->cmdHeightMapBorderAuto->setMinimumHeight(ui->chkHeightMapBorderShow->sizeHint().height());
    // ui->cmdHeightMapCreate->setMinimumHeight(ui->cmdFileOpen->sizeHint().height());
    // ui->cmdHeightMapLoad->setMinimumHeight(ui->cmdFileOpen->sizeHint().height());
    // ui->cmdHeightMapMode->setMinimumHeight(ui->cmdFileOpen->sizeHint().height());
}

void FrmMain::initializeOverridesPanel()
{
    connect(ui->overrides, &PartMainOverride::overrideChanged, this, [this](bool feedOverridden, double feed, bool rapidOverridden, double rapid, bool spindleOverridden, double spindle) {
        communicator()->overrides()->setTargets(feedOverridden, (int)feed, rapidOverridden, (int)rapid, spindleOverridden, (int)spindle);
        ui->grpOverriding->setProperty("overrided", feedOverridden | rapidOverridden | spindleOverridden);
        Utils::refreshStyle(ui->grpOverriding);
    });
    connect(ui->grpOverriding, &QGroupBox::toggled, this, &FrmMain::overridingGroupToggled);
}

void FrmMain::initializeLogMenu()
{
    ui->menuShowLog->setVisible(m_logForm != nullptr);
    connect(ui->menuShowLog, &QMenu::aboutToShow, this, [this]() {
        if (m_logForm != nullptr) {
            m_logForm->show();
            m_logForm->raise();
            m_logForm->activateWindow();
            ui->menuShowLog->hide();
        } else {
            qWarning() << "[FrmMain] No log window available to show";
            QMessageBox::warning(this, tr("No log window"), tr("Log window is disabled"));
        }
    });
}

void FrmMain::connectFilesManagerToUpdateControls()
{
    FilesManager& fm = FilesManager::instance();
    connect(&fm, &FilesManager::gcodeFileStateChanged, this, [this](bool, const QString&, bool) {
        updateControlsState(currentUiState());
    });
    connect(&fm, &FilesManager::heightmapFileStateChanged, this, [this](bool, const QString&, bool) {
        updateControlsState(currentUiState());
    });
}

void FrmMain::connectWindowTitleUpdater()
{
    FilesManager& fm = FilesManager::instance();
    connect(&fm, &FilesManager::gcodeFileStateChanged, this, [this, &fm](bool opened, const QString& filePath, bool modified) {
        Q_UNUSED(filePath);

        if (!opened) {
            this->setWindowTitle(qApp->applicationDisplayName());
        } else {
            QString mod = modified ? " (*)" : "";
            this->setWindowTitle(fm.gcodeFileName() + mod + " - " + qApp->applicationDisplayName());
        }
    });
}

void FrmMain::initializeDockCorners()
{
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
}

void FrmMain::initializeVisualizerPanel()
{
    ui->visualizer->setHeightmap(heightmap());
    ui->visualizer->setProgram(&program(), nullptr);
    ui->visualizer->initDrawables();

    connect(&program(), &GCode::linesUpdated, this, [this]() {
        // updateParser();
    });

    initializeVisualizer();
    initializeMainMenu();

    connect(ui->visualizer, &PartMainVisualizer::editHeightmapPoint, this, [this](QPoint point) {
        DlgEditHeightmapPoint* dialog = new DlgEditHeightmapPoint(point, heightmap().at(point), this);
        connect(dialog, &QDialog::finished, this, [this, dialog](int result) {
            if (result == QDialog::Accepted) {
                setHeightmapPoint(dialog->point(), dialog->height());
            }
            sender()->deleteLater();
        });
        dialog->open();
    });
    connect(ui->visualizer, &PartMainVisualizer::goToCursor, this, [this](QPointF pos) {
        QString text = QString("Are you sure you want to move machine to %1, %2?").arg(pos.x(), 2).arg(pos.y(), 2);
        if (QMessageBox::question(this, "Go to...", text) == QMessageBox::No) {
            return;
        }

        communicator()->stateBehavior()->action(GoToAction(pos, m_configuration.joggingModule().feed()));
    });
    connect(ui->dockVisualizer, &QDockWidget::visibilityChanged, this, &FrmMain::visualizerVisibilityChanged);
}

void FrmMain::initializeVirtualSettingsPanel()
{
    m_partMainVirtualSettings = new PartMainVirtualSettings();
    m_partMainVirtualSettings->setEnabled(true);
    appendPanel(
        ui->scrollContentsDevice,
        "VirtualSettings",
        "Virtual uCNC settings",
        m_partMainVirtualSettings
    );
    // Local name `conn` avoids shadowing the FrmMain::connection() accessor
    // used on the right-hand side of the dynamic_cast.
    connect(m_partMainVirtualSettings, &PartMainVirtualSettings::lockProbeAtCurrentPosition, this, [this]() {
        VirtualConnection *conn = dynamic_cast<VirtualConnection*>(connection());
        if (conn) {
            conn->lockProbeAtCurrentPosition();
        }
    });
    connect(m_partMainVirtualSettings, &PartMainVirtualSettings::resetProbePosition, this, [this]() {
        VirtualConnection *conn = dynamic_cast<VirtualConnection*>(connection());
        if (conn) {
            conn->resetProbePosition();
        }
    });
    connect(m_partMainVirtualSettings, &PartMainVirtualSettings::setHome, this, [this](bool abs, double x, double y, double z) {
        VirtualConnection *conn = dynamic_cast<VirtualConnection*>(connection());
        if (conn) {
            conn->setHome(abs, x, y, z);
        }
    });
    connect(m_partMainVirtualSettings, &PartMainVirtualSettings::setSingleLimit, this, [this](Axis axis, float pos) {
        VirtualConnection *conn = dynamic_cast<VirtualConnection*>(connection());
        if (conn) {
            conn->setSingleLimit(axis, pos);
        }
    });
    connect(m_partMainVirtualSettings, &PartMainVirtualSettings::estop, this, [this]() {
        VirtualConnection *conn = dynamic_cast<VirtualConnection*>(connection());
        if (conn) {
            conn->estop();
        }
    });

    appendSpacer(
        ui->scrollContentsDevice
    );
}

void FrmMain::initializeDockTitles()
{
    ui->dockDevice->setTitleBarWidget(new DockableTitle(ui->dockDevice));
    ui->dockConsole->setTitleBarWidget(new DockableTitle(ui->dockConsole));
    ui->dockVisualizer->setTitleBarWidget(new DockableTitle(ui->dockVisualizer));
    ui->dockUser->setTitleBarWidget(new DockableTitle(ui->dockUser));
    ui->dockProgram->setTitleBarWidget(new DockableTitle(ui->dockProgram));
    ui->dockModification->setTitleBarWidget(new DockableTitle(ui->dockModification));
}

void FrmMain::initializeVisualizer()
{
    connect(ui->visualizer, &PartMainVisualizer::viewModeChanged, this, [this](GLWidget::ViewMode mode) {
        switch (mode) {
            case GLWidget::ViewMode::Perspective:
                UiConfigs::instance().visualizer().setViewMode(ConfigurationVisualizer::ViewMode::Perspective);
                break;
            case GLWidget::ViewMode::Orthogonal:
                UiConfigs::instance().visualizer().setViewMode(ConfigurationVisualizer::ViewMode::Orthogonal);
                break;
            case GLWidget::ViewMode::View2D:
                UiConfigs::instance().visualizer().setViewMode(ConfigurationVisualizer::ViewMode::View2D);
                break;
        }
    });
}

void FrmMain::initializeEventFilter()
{
    qApp->installEventFilter(this);
}

void FrmMain::initializeMainMenu()
{
    connect(ui->actFileNew, &QAction::triggered, this, &FrmMain::newProgram);
    connect(ui->actFileOpen, &QAction::triggered, this, &FrmMain::fileOpen);
    connect(ui->actFileSave, &QAction::triggered, this, &FrmMain::fileSave);
    connect(ui->actFileSaveAs, &QAction::triggered, this, &FrmMain::fileSaveAs);
    connect(ui->actFileSaveTransformedAs, &QAction::triggered, this, &FrmMain::fileSaveTransformedAs);
    connect(ui->actFileExit, &QAction::triggered, this, &FrmMain::fileExit);
    connect(ui->actFileSettings, &QAction::triggered, this, &FrmMain::fileSettings);
    connect(ui->actServiceConfigureGRBL, &QAction::triggered, this, &FrmMain::serviceConfigureGRBL);
    connect(ui->actServiceResetGRBLConfiguration, &QAction::triggered, this, &FrmMain::serviceResetGRBLConfiguration);
    connect(ui->actTestConverterArcsToLines, &QAction::triggered, this, [this] { testConverter(0); });
    connect(ui->actTestConverterFusionRestoreRapidMovements, &QAction::triggered, this, [this] { testConverter(1); });
    connect(ui->actTestConverterFeedRateConverter, &QAction::triggered, this, [this] { testConverter(2); });
    connect(ui->actTestConverterCoordinateOffsetConverter, &QAction::triggered, this, [this] { testConverter(3); });
    connect(ui->actTestConverterSafeSpindleStopConverter, &QAction::triggered, this, [this] { testConverter(4); });
    connect(ui->actTestConverterMovementOptimizerConverter, &QAction::triggered, this, [this] { testConverter(5); });
    connect(ui->actTestConverterShakingGCode, &QAction::triggered, this, [this] { testConverter(6); });
    connect(ui->actTestConverterApplyHeightmap, &QAction::triggered, this, [this] { testConverter(7); });
    connect(ui->actTestConverterStripComments, &QAction::triggered, this, [this] { testConverter(8); });
    connect(ui->actTestConverterMovePath, &QAction::triggered, this, [this] { testConverter(9); });
    connect(ui->actTestConverterModifyFeedRate, &QAction::triggered, this, [this] { testConverter(10); });
    connect(ui->actAbout, &QAction::triggered, this, &FrmMain::aboutShow);
    connect(ui->actViewLockWindows, &QAction::toggled, this, &FrmMain::viewLockWindowsToggled);
    connect(ui->actViewDarkMode, &QAction::toggled, this, &FrmMain::viewDarkModeToggled);
    connect(ui->actViewCentralProgram, &QAction::toggled, this, &FrmMain::viewCentralProgramToggled);
    connect(ui->actViewCentralVisualizer, &QAction::toggled, this, &FrmMain::viewCentralVisualizerToggled);
    connect(ui->actHeightmapOpen2, &QAction::triggered, this, &FrmMain::openHeightmap);
    connect(ui->actHeightmapSave, &QAction::triggered, this, &FrmMain::saveHeightmap);
    connect(ui->menuViewWindows, &QMenu::aboutToShow, this, &FrmMain::populateViewWindowsMenu);
    connect(ui->menuViewPanels, &QMenu::aboutToShow, this, &FrmMain::populateViewPanelsMenu);
}

bool FrmMain::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
#ifdef WINDOWS
    MSG *msg = static_cast<MSG *>(message);
    if (msg->message == m_taskbarButtonCreatedMessageId) {
        m_taskBar.init();
        m_taskBar.setProgress(20, 100);

        return true;
    }
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

void FrmMain::showEvent(QShowEvent *se)
{
    Q_UNUSED(se)

    if (m_firstShow) {
        Utils::positionDialog(this, UiConfigs::instance().ui().mainFormGeometry(), UiConfigs::instance().ui().mainFormMaximized());
        m_firstShow = false;
    }
}

void FrmMain::hideEvent(QHideEvent *he)
{
    Q_UNUSED(he)
}

void FrmMain::resizeEvent(QResizeEvent *re)
{
    QMainWindow::resizeEvent(re);

    if (!m_firstShow) {
        UiConfigs::instance().ui().setMainFormGeometry(this);
    }
}

void FrmMain::closeEvent(QCloseEvent *ce)
{
    bool mode = m_heightmapMode;
    m_heightmapMode = false;

    if (!saveProgramChanges() || !saveHeightmapChanges()) {
        ce->ignore();
        m_heightmapMode = mode;
        return;
    }

    if ((communicator()->stateBehavior()->is(AbstractStateBehavior::Type::Running)) &&
        QMessageBox::warning(this, this->windowTitle(), tr("File sending in progress. Terminate and exit?"),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    {
        ce->ignore();
        m_heightmapMode = mode;
        return;
    }

    // Show closing form to prevent "Not responding" state in case of slow shutdown (e.g. waiting for connection to close)
    FrmClosingApp closingForm(this);
    closingForm.show();
    qApp->processEvents();

    communicator()->deinit();
    connection()->close();

    saveSettings();

    if (m_logForm) {
        m_logForm->close();
    }
}

void FrmMain::dragEnterEvent(QDragEnterEvent *dee)
{
    // Ignore dockable widget drops
    if (dee->mimeData()->hasFormat("application/widget")) {
        return;
    }

    m_fileDropOverlay = new FileDropOverlay(this);
    m_fileDropOverlay->setGeometry(0, 0, width(), height());
    m_fileDropOverlay->show();

    // Accept all, we will validate in drop event
    dee->acceptProposedAction();

    if (!communicator()->stateBehavior()->is(AbstractStateBehavior::Type::Idle) || dee->mimeData()->hasFormat("application/widget")) {
        m_fileDropOverlay->showForbidden();

        return;
    }

    if (dee->mimeData()->hasFormat("text/plain") && !m_heightmapMode) {
        m_fileDropOverlay->showValid();

        return;
    } else if (dee->mimeData()->hasFormat("text/uri-list") && dee->mimeData()->urls().count() == 1) {
        QString fileName = dee->mimeData()->urls().at(0).toLocalFile();

        if ((!m_heightmapMode && Utils::isGCodeFile(fileName)) || (m_heightmapMode && Utils::isHeightmapFile(fileName))) {
            m_fileDropOverlay->showValid();

            return;
        }
    }

    m_fileDropOverlay->showForbidden();
}

void FrmMain::dragLeaveEvent(QDragLeaveEvent *dle)
{
    Q_UNUSED(dle);

    if (m_fileDropOverlay) {
        delete m_fileDropOverlay;
        m_fileDropOverlay = nullptr;
    }
}

void FrmMain::dropEvent(QDropEvent *de)
{
    if (m_fileDropOverlay) {
        bool valid = m_fileDropOverlay->valid();

        delete m_fileDropOverlay;
        m_fileDropOverlay = nullptr;

        if (!valid) {
            return;
        }
    }

    QString fileName = de->mimeData()->urls().at(0).toLocalFile();

    if (!m_heightmapMode) {
        if (!saveProgramChanges()) {
            return;
        }

        // Load dropped g-code file
        if (!fileName.isEmpty()) {
            Core::instance().addRecentFile(fileName);
            loadFile(fileName);
        // Load dropped text
        } else {
            FilesManager::instance().resetGcodeFile();
            FilesManager::instance().setGcodeModified(true);
            //@todo fix after refactoring loadFile to be faster
            //loadFile(de->mimeData()->text().split("\n"));
        }
    } else {
        if (!saveHeightmapChanges()) {
            return;
        }

        // Load dropped heightmap file
        Core::instance().addRecentHeightmap(fileName);
        // loadHeightmap(fileName);
    }
}

void FrmMain::changeEvent(QEvent *ce)
{
    QMainWindow::changeEvent(ce);
    if (ce->type() == QEvent::WindowStateChange) {
        UiConfigs::instance().ui().setMainFormGeometry(this);
    }
}

void FrmMain::moveEvent(QMoveEvent *me)
{
    QMainWindow::moveEvent(me);
    if (!m_firstShow) {
        UiConfigs::instance().ui().setMainFormGeometry(this);
    }
}

QMenu *FrmMain::createPopupMenu()
{
    QMenu *menu = QMainWindow::createPopupMenu();

    foreach (QAction *a, menu->actions()) {
        if (a->text().contains("_spacer")) {
            a->setVisible(false);
        }
    }

    return menu;
}

void FrmMain::fileOpen()
{
    onFileOpen();
}

void FrmMain::fileSave()
{
    FilesManager& fm = FilesManager::instance();
    if (!m_heightmapMode) {
        // G-code saving
        if (fm.gcodeOpened()) fileSaveAs(); else {
            GCodeExporter exporter;
            exporter.exportToFile(program(), fm.gcodeFilePath());
            fm.setGcodeModified(false);
        }
    } else {
        // Height map saving
        if (fm.heightmapOpened()) fileSaveAs(); else {
            HeightmapExporter exporter;
            exporter.exportToFile(heightmap(), fm.heightmapFilePath());
        }
    }
}

void FrmMain::fileSaveAs()
{
    FilesManager& fm = FilesManager::instance();

    if (!m_heightmapMode) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save file as"), lastUsedDirectory(), tr(FILE_FILTER_TEXT));

        if (!fileName.isEmpty()) {
            GCodeExporter exporter;
            exporter.exportToFile(program(), fm.gcodeFilePath());

            fm.setGcodeFilePath(fileName);
            fm.setGcodeModified(false);

            Core::instance().addRecentFile(fileName);

            updateControlsState();
        }
    } else {
        QString fileName = (QFileDialog::getSaveFileName(this, tr("Save file as"), lastUsedDirectory(), tr("Heightmap files (*.map)")));

        if (!fileName.isEmpty()) {
            HeightmapExporter exporter;
            exporter.exportToFile(heightmap(), fm.heightmapFilePath());

            fm.setHeightmapFilePath(fileName);
            fm.setHeightmapModified(false);

            ui->heightmap->setOpenFile(fileName.mid(fileName.lastIndexOf("/") + 1));

            Core::instance().addRecentHeightmap(fileName);

            updateControlsState();
        }
    }
}

void FrmMain::fileSaveTransformedAs()
{
    QString fileName = (QFileDialog::getSaveFileName(this, tr("Save file as"), lastUsedDirectory(), tr(FILE_FILTER_TEXT)));

    if (!fileName.isEmpty()) {
        GCodeExporter exporter;
        exporter.exportToFile(program(), fileName);
    }
}

void FrmMain::openHeightmap()
{
    QString fileName = (QFileDialog::getOpenFileName(this, tr("Open heightmap"), lastUsedDirectory(), tr("Heightmap files (*.map)")));
    if (fileName.isEmpty()) {
        return;
    }

    try {
        HeightmapLoader::loadFromFile(fileName, heightmap());
    } catch (std::runtime_error &err) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load heightmap: %1").arg(err.what()));
        return;
    }

    ui->console->append(tr("Heightmap %1x%2loaded from %3").arg(heightmap().gridWidth()).arg(heightmap().gridHeight())
                                  .arg(fileName));

    ui->heightmap->setHeightmap(&heightmap());
    ui->visualizer->setHeightmap(heightmap());
}

void FrmMain::saveHeightmap()
{
    QString fileName = (QFileDialog::getSaveFileName(this, tr("Save heightmap as"), lastUsedDirectory(), tr("Heightmap files (*.map)")));
    if (fileName.isEmpty()) {
        return;
    }

    try {
        HeightmapExporter::exportToFile(heightmap(), fileName);
    } catch (std::runtime_error &err) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save heightmap: %1").arg(err.what()));
        return;
    }

    ui->console->append(tr("Heightmap saved to %1").arg(fileName));
}

void FrmMain::fileExit()
{
    close();
}

void FrmMain::fileSettings()
{
    QList<QAction*> acts = findChildren<QAction*>(QRegularExpression("act.*"));

    // QTableWidget *table = m_settings->ui->tblShortcuts;

    // table->clear();
    // table->setColumnCount(3);
    // table->setRowCount(acts.count());
    // table->setHorizontalHeaderLabels(QStringList() << tr("Command") << tr("Text") << tr("Shortcuts"));

    // table->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    // table->verticalHeader()->setFixedWidth(table->verticalHeader()->sizeHint().width() + 11);

    // qSort(acts.begin(), acts.end(), FrmMain::actionLessThan);
    // for (int i = 0; i < acts.count(); i++) {
    //     table->setItem(i, 0, new QTableWidgetItem(acts.at(i)->objectName()));
    //     table->setItem(i, 1, new QTableWidgetItem(acts.at(i)->text().remove("&")));
    //     table->setItem(i, 2, new QTableWidgetItem(acts.at(i)->shortcut().toString()));

    //     table->item(i, 0)->setFlags(Qt::ItemIsEnabled);
    //     table->item(i, 1)->setFlags(Qt::ItemIsEnabled);
    //     table->item(i, 2)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    // }

    // table->resizeColumnsToContents();
    // table->setMinimumHeight(table->rowHeight(0) * 10
    //     + table->horizontalHeader()->height() + table->frameWidth() * 2);
    // table->horizontalHeader()->setMinimumSectionSize(table->horizontalHeader()->sectionSize(2));
    // table->horizontalHeader()->setStretchLastSection(true);

    emit settingsAboutToShow();

    QScopedPointer<FrmSettings> form(new FrmSettings(this, m_configuration));
    Utils::setVisualMode(form.data(), UiConfigs::instance().ui().darkTheme());
    if (form->exec()) {
        // @TODO connection
        // if (m_settings->port() != "" && (m_settings->port() != m_serialPort.portName() ||
        //                                    m_settings->baud() != m_serialPort.baudRate())) {
        //     if (m_serialPort.isOpen()) m_serialPort.close();
        //     m_serialPort.setPortName(m_settings->port());
        //     m_serialPort.setBaudRate(m_settings->baud());
        //     openPort();
        // }

        m_configuration.save();

        updateControlsState();
        applySettings();

        // Update shortcuts
        for (int i = 0; i < acts.count(); i++) {
            //acts[i]->setShortcut(QKeySequence(table->item(i, 2)->data(Qt::DisplayRole).toString()));
        }

        emit settingsAccepted();

    } else {
        // @TODO so weird!
        // m_settings->undo();

        emit settingsRejected();
    }
}

void FrmMain::serviceConfigureGRBL()
{
    FrmGrblConfigurator *form = new FrmGrblConfigurator(this, UiConfigs::instance().ui(), communicator());
    form->exec();
    form->deleteLater();
}

void FrmMain::serviceResetGRBLConfiguration()
{
    int res = QMessageBox::warning(this, this->windowTitle(), tr("This will reset GRBL configuration to defaults. Continue?"),
                                   QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::No) return;

    communicator()->resetGRBLConfiguration();
}

void FrmMain::aboutShow()
{
    FrmAbout *form = new FrmAbout(this);
    form->exec();
    form->deleteLater();
}

void FrmMain::viewLockWindowsToggled(bool checked)
{
    QList<QDockWidget*> dl = findChildren<QDockWidget*>();

    foreach (QDockWidget *dock, dl) {
        Utils::setDockableLocked(dock, checked);
    }

    UiConfigs::instance().ui().setLockWindows(checked);
}

void FrmMain::viewDarkModeToggled(bool checked)
{
    UiConfigs::instance().ui().setDarkMode(checked);
    ThemeManager::instance().setDark(checked);
}

void FrmMain::viewCentralProgramToggled(bool checked)
{
    centralWidgetActionTriggered(checked);
}

// Visualiser in central widget, program docked, hide empty visualizer dock
void FrmMain::viewCentralVisualizerToggled(bool checked)
{
    centralWidgetActionTriggered(checked);
}

void FrmMain::onFileOpen(QString filePath)
{
    if (!m_heightmapMode) {
        if (!saveProgramChanges()) {
            return;
        }

        if (filePath.isEmpty()) {
            filePath = QFileDialog::getOpenFileName(this, tr("Open Heightmap"), lastUsedDirectory(),
                                   tr(FILE_FILTER_TEXT";;All files (*.*)"));
            if (filePath.isEmpty()) {
                return;
            }

            UiConfigs::instance().ui().currentWorkingDirectory(filePath.left(filePath.lastIndexOf(QRegularExpression("[/\\\\]+"))));
        }

        Core::instance().addRecentFile(filePath);

        loadFile(filePath);
    } else {
        if (!saveHeightmapChanges()) {
            return;
        }

        if (filePath.isEmpty()) {
            QString filePath = QFileDialog::getOpenFileName(this, tr("Open G-Code"), lastUsedDirectory(), tr("Heightmap files (*.map)"));
            if (filePath.isEmpty()) {
                return;
            }

            UiConfigs::instance().ui().currentWorkingDirectory(filePath.left(filePath.lastIndexOf(QRegularExpression("[/\\\\]+"))));
        }

        Core::instance().addRecentHeightmap(filePath);

        HeightmapLoader::loadFromFile(filePath, heightmap());
    }
}

void FrmMain::onFileSend()
{
    communicator()->stateBehavior()->action(RunAction(program()));

#ifdef WINDOWS
    m_taskBar.setProgress(0, program().count() - 1);
    m_taskBar.show();
#endif
}

void FrmMain::onFilePause(bool checked)
{
    static SenderState s;

    // if (checked) {
    //     //PAUSE
    //     s = communicator()->senderState();
    //     // setSenderState(SenderPaused);
    //     communicator()->setSenderStateAndEmitSignal(SenderState::Pausing);
    //     ui->cmdFilePause->setText(tr("Pausing..."));
    //     ui->cmdFilePause->setEnabled(false);
    // } else {
    //     //RESUME
    //     if (communicator()->senderState() == SenderState::ChangingTool) {
    //         communicator()->setSenderStateAndEmitSignal(SenderState::Transferring);
    //     } else {
    //         if (m_configuration.senderModule().usePauseCommands()) {
    //             communicator()->sendCommands(CommandSource::ProgramAdditionalCommands, m_configuration.senderModule().afterPauseCommands());
    //         }
    //         communicator()->setSenderStateAndEmitSignal(s);
    //     }
    //     updateControlsState();
    // }

    // if (checked) {
    AbstractStateBehavior* sb = communicator()->stateBehavior();
    if (sb->canExecute(Action::Pause)) {
        sb->action(Action::Pause);
    } else if (sb->canExecute(Action::Resume)) {
        sb->action(Action::Resume);
    }
    // if (communicator()->stateBehavior()->action(Action::PauseResume)) {
    //     // timer().pauseExecution();
    //     // ui->program->setPauseButtonText(tr("Resume"));
    // }
    // } else {
    //     Action action(Action::Resume);
    //     if (communicator()->stateBehavior()->action(action)) {
    //         timer().resumeExecution();
    //         ui->program->setPauseButtonText(tr("Pause"));
    //     }
    // }
}

void FrmMain::onFileAbort()
{
    // ui->program->setAbortButtonEnabled(false);
    // timer().stopExecution();
    // communicator()->abort();
    communicator()->stateBehavior()->action(Action::Abort);
}

void FrmMain::onFileReset()
{
    program().reset();
    ui->visualizer->resetLastDrawnLine();
    // communicator()->m_probeIndex = -1;

    if (!m_heightmapMode) {
        QList<LineSegment>& list = viewParser().getLineSegmentList();

        QList<int> indexes;
        for (int i = 0; i < list.count(); i++) {
            list[i].setDrawn(false);
            indexes.append(i);
        }
        ui->visualizer->updateCodeDrawer(indexes);

        ui->program->setTableUpdatesEnabled(false);

        // It should be done in `program().reset()`
        // for (int i = 0; i < m_currentProgram->count() - 1; i++) {
        //     (*m_currentProgram)[i].state = GCodeItem::InQueue;
        //     (*m_currentProgram)[i].response = QString();
        // }
        ui->program->setTableUpdatesEnabled(true);

        ui->program->resetToFirstRow();

        timer().reset();
        timeEstimator().resetEstimation();
        ui->visualizer->setTimeEstimation(timeEstimator());
    } else {
        ui->heightmap->setGridUpdateEnabled();

        // delete m_heightmapInterpolationDrawer.data();
        ui->visualizer->updateHeightmapInterpolation(true);

        ui->program->clearHeightmapModel();
        updateHeightmapGrid();
    }
}

// void FrmMain::on_cmdCommandSend_clicked()
// {
//     QString command = ui->cboCommand->currentText();
//     if (command.isEmpty()) return;

//     ui->cboCommand->storeText();
//     ui->cboCommand->setCurrentText("");
//     communicator()->sendCommand(command, COMMAND_TI_UI);
// }

// void FrmMain::on_cmdClearConsole_clicked()
// {
//     ui->txtConsole->clear();
// }

// void FrmMain::on_cmdHome_clicked()
// {
//     communicator()->m_homing = true;
//     communicator()->m_updateSpindleSpeed = true;
//     communicator()->sendCommand(CommandSource::GeneralUI, "$H", COMMAND_TI_UI);
// }

// void FrmMain::on_cmdCheck_clicked(bool checked)
// {
//     if (checked) {
//         communicator()->storeParserState();
//         communicator()->sendCommand(CommandSource::GeneralUI, "$C", COMMAND_TI_UI);
//     } else {
//         communicator()->m_aborting = true;
//         communicator()->reset();
//     };
// }

// void FrmMain::on_cmdReset_clicked()
// {
//     communicator()->reset();
//     //grblReset();
// }

// void FrmMain::on_cmdUnlock_clicked()
// {
//     communicator()->m_updateSpindleSpeed = true;
//     communicator()->sendCommand(CommandSource::GeneralUI, "$X", COMMAND_TI_UI);
// }

// void FrmMain::on_cmdHold_clicked(bool checked)
// {
//     connection()->sendByteArray(QByteArray(1, checked ? (char)'!' : (char)'~'));
// }

// void FrmMain::on_cmdSleep_clicked()
// {
//     communicator()->sendCommand(CommandSource::GeneralUI, "$SLP", COMMAND_TI_UI);
// }

// void FrmMain::on_cmdDoor_clicked()
// {
//     connection()->sendByteArray(QByteArray(1, (char)0x84));
// }

// void FrmMain::on_cmdFlood_clicked()
// {
//     connection()->sendByteArray(QByteArray(1, (char)0xa0));
// }

// void FrmMain::on_cmdSpindle_toggled(bool checked)
// {
//     ui->grpSpindle->setProperty("overrided", checked);
//     style()->unpolish(ui->grpSpindle);
//     ui->grpSpindle->ensurePolished();

//     if (checked) {
//         if (!ui->grpSpindle->isChecked()) ui->grpSpindle->setTitle(tr("Spindle") + QString(tr(" (%1)")).arg(ui->slbSpindle->value()));
//     } else {
//         ui->grpSpindle->setTitle(tr("Spindle"));
//     }
// }

void FrmMain::toggleSpindle(bool checked)
{
    if (ui->control->hold()) {
        connection()->sendByteArray(QByteArray(1, char(0x9e)));
    } else {
        // communicator()->sendCommand(CommandSource::GeneralUI, checked ? QString("M3 S%1").arg(ui->slbSpindle->value()) : "M5", TABLE_INDEX_UI);
    }
}

void FrmMain::overridingGroupToggled(bool checked)
{
    if (checked) {
        ui->grpOverriding->setTitle(tr("Overriding"));
    } else if (communicator()->overrides()->isFeedOverridden() | communicator()->overrides()->isRapidOverridden() | communicator()->overrides()->isSpindleOverridden()) {
        ui->grpOverriding->setTitle(tr("Overriding") + QString(tr(" (%1/%2/%3)"))
               .arg(communicator()->overrides()->isFeedOverridden() ? QString::number(communicator()->overrides()->targetFeed()) : "-")
               .arg(communicator()->overrides()->isRapidOverridden() ? QString::number(communicator()->overrides()->targetRapid()) : "-")
               .arg(communicator()->overrides()->isSpindleOverridden() ? QString::number(communicator()->overrides()->targetSpindle()) : "-"));
    }
    updateLayouts();

    ui->overrides->setVisible(checked);
}

void FrmMain::spindleGroupToggled(bool checked)
{
//     if (checked) {
//         ui->grpSpindle->setTitle(tr("Spindle"));
//     } else if (ui->cmdSpindle->isChecked()) {
// //        ui->grpSpindle->setTitle(tr("Spindle") + QString(tr(" (%1)")).arg(ui->txtSpindleSpeed->text()));
//         ui->grpSpindle->setTitle(tr("Spindle") + QString(tr(" (%1)")).arg(ui->slbSpindle->value()));
//     }
    updateLayouts();

    // ui->spindle->setVisible(checked);
}

void FrmMain::jogGroupToggled(bool checked)
{
    updateJogTitle();
    updateLayouts();

    ui->jog->setVisible(checked);
}

void FrmMain::heightmapGroupToggled(bool checked)
{
    ui->heightmap->setVisible(checked);
}

void FrmMain::keyboardControlToggled(bool checked)
{
    ui->grpJog->setProperty("overrided", checked);
    style()->unpolish(ui->grpJog);
    ui->grpJog->ensurePolished();

    // Store/restore coordinate system
    if (checked) {
        //communicator()->sendCommand(CommandSource::System, "$G", COMMAND_TI_UTIL1);
    } else {
        if (m_absoluteCoordinates) communicator()->sendCommand(CommandSource::System, "G90", TABLE_INDEX_UI);
    }

    if (!communicator()->stateBehavior()->is(AbstractStateBehavior::Type::Running)) {
        ui->jog->setKeyboardControl(checked);
    }

    updateJogTitle();
    updateControlsState();
}

//TODO heightmap
void FrmMain::useHeightmapToggled(bool checked)
{
// //    static bool fileChanged;

//     // Reset table view
//     QByteArray headerState = ui->tblProgram->horizontalHeader()->saveState();
//     ui->tblProgram->setModel(NULL);

//     CancelException cancel;

    if (checked) try {

//         // Prepare progress dialog
//         QProgressDialog progress(tr("Applying heightmap..."), tr("Abort"), 0, 0, this);
//         progress.setWindowModality(Qt::WindowModal);
//         progress.setFixedHeight(progress.sizeHint().height());
//         progress.show();
//         progress.setStyleSheet("QProgressBar {text-align: center; qproperty-format: \"\"}");

//         // Set current model to prevent reseting heightmap cache
//         updateCurrentModel(&m_programHeightmapModel);

//         // Update heightmap-modificated program if not cached
//         if (m_programHeightmapModel.rowCount() == 0) {

//             // Modifying linesegments
//             QList<LineSegment*> *list = viewParser().getLines();
//             QRectF borderRect = borderRectFromTextboxes();
//             double x, y, z;
//             QVector3D point;

//             progress.setLabelText(tr("Subdividing segments..."));
//             progress.setMaximum(list->count() - 1);

//             for (int i = 0; i < list->count(); i++) {
//                 if (!list->at(i)->isZMovement()) {
//                     QList<LineSegment*> subSegments = subdivideSegment(list->at(i));

//                     if (subSegments.count() > 0) {
//                         delete list->at(i);
//                         list->removeAt(i);
//                         foreach (LineSegment* subSegment, subSegments) list->insert(i++, subSegment);
//                         i--;
//                     }
//                 }

//                 if (progress.isVisible() && (i % PROGRESS_STEP == 0)) {
//                     progress.setMaximum(list->count() - 1);
//                     progress.setValue(i);
//                     qApp->processEvents();
//                     if (progress.wasCanceled()) throw cancel;
//                 }
//             }

//             progress.setLabelText(tr("Updating Z-coordinates..."));
//             progress.setMaximum(list->count() - 1);

//             for (int i = 0; i < list->count(); i++) {
//                 if (i == 0) {
//                     x = list->at(i)->getStart().x();
//                     y = list->at(i)->getStart().y();
//                     z = list->at(i)->getStart().z() + Interpolation::bicubicInterpolate(borderRect, &m_heightmapModel, x, y);
//                     list->at(i)->setStart(QVector3D(x, y, z));
//                 } else list->at(i)->setStart(list->at(i - 1)->getEnd());

//                 x = list->at(i)->getEnd().x();
//                 y = list->at(i)->getEnd().y();
//                 z = list->at(i)->getEnd().z() + Interpolation::bicubicInterpolate(borderRect, &m_heightmapModel, x, y);
//                 list->at(i)->setEnd(QVector3D(x, y, z));

//                 if (progress.isVisible() && (i % PROGRESS_STEP == 0)) {
//                     progress.setValue(i);
//                     qApp->processEvents();
//                     if (progress.wasCanceled()) throw cancel;
//                 }
//             }

//             progress.setLabelText(tr("Modifying G-code program..."));
//             progress.setMaximum(m_programModel.rowCount() - 2);

//             // Modifying g-code program
//             QString command;
//             QStringList args;
//             int lineNumber;
//             QString newCommand;
//             GCodeItem item;
//             int lastSegmentIndex = 0;
//             int lastCommandIndex = -1;

//             // Search strings
//             QString coords("XxYyZzIiJjKkRr");
//             QString g("Gg");
//             QString m("Mm");

//             char codeChar;          // Single code char G1 -> G
//             float codeNum;          // Code number      G1 -> 1

//             QString lastCode;
//             bool isLinearMove;
//             bool hasCommand;

//             m_programLoading = true;
//             for (int i = 0; i < m_programModel.rowCount() - 1; i++) {
//                 command = program()[i].command;
//                 lineNumber = program()[i].lineNumber;
//                 isLinearMove = false;
//                 hasCommand = false;

//                 if (lineNumber < 0 || lineNumber == lastCommandIndex || lastSegmentIndex == list->count() - 1) {
//                     item.command = command;
//                     m_programHeightmapModel.data().append(item);
//                 } else {
//                     // Get commands args
//                     args = m_programModel.data().at(i).args;
//                     newCommand.clear();

//                     // Parse command args
//                     foreach (QString arg, args) {                   // arg examples: G1, G2, M3, X100...
//                         codeChar = arg.at(0).toLatin1();            // codeChar: G, M, X...
//                         if (!coords.contains(codeChar)) {           // Not parameter
//                             codeNum = arg.mid(1).toDouble();
//                             if (g.contains(codeChar)) {             // 'G'-command
//                                 // Store 'G0' & 'G1'
//                                 if (codeNum == 0.0f || codeNum == 1.0f) {
//                                     lastCode = arg;
//                                     isLinearMove = true;            // Store linear move
//                                 }

//                                 // Replace 'G2' & 'G3' with 'G1'
//                                 if (codeNum == 2.0f || codeNum == 3.0f) {
//                                     newCommand.append("G1");
//                                     isLinearMove = true;
//                                 // Drop plane command for arcs
//                                 } else if (codeNum != 17.0f && codeNum != 18.0f && codeNum != 19.0f) {
//                                     newCommand.append(arg);
//                                 }

//                                 hasCommand = true;                  // Command has 'G'
//                             } else {
//                                 if (m.contains(codeChar))
//                                     hasCommand = true;              // Command has 'M'
//                                 newCommand.append(arg);       // Other commands
//                             }
//                         }
//                     }

//                     // Find first linesegment by command index
//                     for (int j = lastSegmentIndex; j < list->count(); j++) {
//                         if (list->at(j)->getLineNumber() == lineNumber) {
//                             if (!qIsNaN(list->at(j)->getEnd().length()) && (isLinearMove || (!hasCommand && !lastCode.isEmpty()))) {
//                                 // Create new commands for each linesegment with given command index
//                                 while ((j < list->count()) && (list->at(j)->getLineNumber() == lineNumber)) {

//                                     point = list->at(j)->getEnd();
//                                     if (!list->at(j)->isAbsolute()) point -= list->at(j)->getStart();
//                                     if (!list->at(j)->isMetric()) point /= 25.4f;

//                                     item.command = newCommand + QString("X%1Y%2Z%3")
//                                             .arg(point.x(), 0, 'f', 3).arg(point.y(), 0, 'f', 3).arg(point.z(), 0, 'f', 3);
//                                     m_programHeightmapModel.data().append(item);

//                                     if (!newCommand.isEmpty()) newCommand.clear();
//                                     j++;
//                                 }
//                             // Copy original command if not G0 or G1
//                             } else {
//                                 item.command = command;
//                                 m_programHeightmapModel.data().append(item);
//                             }

//                             lastSegmentIndex = j;
//                             break;
//                         }
//                     }
//                 }
//                 lastCommandIndex = lineNumber;

//                 if (progress.isVisible() && (i % PROGRESS_STEP == 0)) {
//                     progress.setValue(i);
//                     qApp->processEvents();
//                     if (progress.wasCanceled()) throw cancel;
//                 }
//             }
//             m_programHeightmapModel.insertRow(m_programHeightmapModel.rowCount());
//         }
//         progress.close();

//         ui->tblProgram->setModel(&m_programHeightmapModel);
//         ui->tblProgram->horizontalHeader()->restoreState(headerState);

//         connect(ui->tblProgram->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onTableCurrentChanged(QModelIndex,QModelIndex)));

//         m_programLoading = false;

//         // Update parser
//         m_currentDrawer = m_codeDrawer;
//         updateParser();

//         // Select first row
//         ui->tblProgram->selectRow(0);
    }
    catch (CancelException) {                       // Cancel modification
//         m_programHeightmapModel.clear();
//         updateCurrentModel(&m_programModel);

//         ui->tblProgram->setModel(&m_programModel);
//         ui->tblProgram->horizontalHeader()->restoreState(headerState);

//         connect(ui->tblProgram->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onTableCurrentChanged(QModelIndex,QModelIndex)));
//         ui->tblProgram->selectRow(0);

//         ui->chkHeightMapUse->setChecked(false);

//         return;
//     } else {                                        // Restore original program
//         updateCurrentModel(&m_programModel);

//         ui->tblProgram->setModel(&m_programModel);
//         ui->tblProgram->horizontalHeader()->restoreState(headerState);

//         connect(ui->tblProgram->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onTableCurrentChanged(QModelIndex,QModelIndex)));

//         // Store changes flag
//         bool fileChanged = m_fileChanged;

//         // Update parser
//         updateParser();

//         // Select first row
//         ui->tblProgram->selectRow(0);

//         // Restore changes flag
//         m_fileChanged = fileChanged;
    }

//     // Update groupbox title
//     ui->grpHeightMap->setProperty("overrided", checked);
//     style()->unpolish(ui->grpHeightMap);
//     ui->grpHeightMap->ensurePolished();

//     // Update menu
//     ui->actFileSaveTransformedAs->setVisible(checked);
}

void FrmMain::heightmapModeToggled(bool checked)
{
    // Update flag
    m_heightmapMode = checked;

    // Reset file progress
    program().reset();
    ui->visualizer->resetLastDrawnLine();

    // Reset/restore g-code program modification on edit mode enter/exit
    if (ui->heightmap->useMap()) {
        useHeightmapToggled(!checked); // Update gcode program parser
    }

    if (checked) {
        ui->program->switchToProbeModel();
        //updateCurrentModel(&m_programModel);
        updateParser();  // Update probe program parser
    } else {
        probeParser().reset();
        if (!ui->heightmap->useMap()) {
            ui->program->switchToProgramModel();
            // connect(ui->tblProgram->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onTableCurrentChanged(QModelIndex,QModelIndex)));
            ui->program->selectFirstRow();

            // updateCurrentModel(&m_programModel);

            if (!ui->heightmap->useMap()) {
                ui->visualizer->updateGCodeExtremes();
                // ui->glwVisualizer->updateExtremes(m_codeDrawer);
//                updateProgramEstimatedTime(m_currentDrawer->viewParser()->getLineSegmentList());
            }
        }
    }

    // Shadow toolpath
    QList<LineSegment>& list = viewParser().getLineSegmentList();
    QList<int> indexes;
    for (int i = 0; i < list.count(); i++) {
        list[i].setDrawn(checked);
        list[i].setIsHightlight(false);
        indexes.append(i);
    }
    // Update only vertex color.
    // If chkHeightMapUse was checked codeDrawer updated via updateParser
    if (!ui->heightmap->useMap()) ui->visualizer->updateCodeDrawer(indexes);

    updateRecentFilesMenus();
    updateControlsState();
}

void FrmMain::onLoadHeightmapRequested()
{
    if (!saveHeightmapChanges()) {
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(this, tr("Open"), lastUsedDirectory(), tr("Heightmap files (*.map)"));

    if (filePath != "") {
        Core::instance().addRecentHeightmap(filePath);
        HeightmapLoader::loadFromFile(filePath, heightmap());

        // If using heightmap
        if (ui->heightmap->useMap() && !m_heightmapMode) {
            // Restore original file
            useHeightmapToggled(false);
            // Apply heightmap
            useHeightmapToggled(true);
        }

        updateControlsState(); // Enable 'cmdHeightMapMode' button
    }
}

void FrmMain::populateViewWindowsMenu()
{
    QAction *action;
    QList<QAction*> al;

    foreach (QDockWidget *dock, findChildren<QDockWidget*>()) {
        if (dock->property("cw").toBool() == true) {
            // central widget cannot be hide/show
            continue;
        }
        action = new QAction(dock->windowTitle(), ui->menuViewWindows);
        action->setCheckable(true);
        action->setChecked(dock->isVisible());
        connect(action, &QAction::triggered, dock, &QDockWidget::setVisible);
        al.append(action);
    }

    std::sort(al.begin(), al.end(), FrmMain::actionTextLessThan);

    ui->menuViewWindows->clear();
    ui->menuViewWindows->addActions(al);
}

void FrmMain::populateViewPanelsMenu()
{
    QAction *a;

    ui->menuViewPanels->clear();

    QStringList panels;
    if (ui->scrollContentsDevice->isVisible()) panels << ui->scrollContentsDevice->saveState();
    if (ui->scrollContentsModification->isVisible()) panels << "\n" << ui->scrollContentsModification->saveState();
    if (ui->scrollContentsUser->isVisible()) panels << "\n" << ui->scrollContentsUser->saveState();

    foreach (QString s, panels) {
        if (s == "\n") {
            ui->menuViewPanels->addSeparator();
        } else {
            QGroupBox *b = findChild<QGroupBox*>(s);
            if (b) {
                a = ui->menuViewPanels->addAction(b->title());
                a->setCheckable(true);
                a->setChecked(b->isVisible());
                connect(a, &QAction::triggered, b, &QWidget::setVisible);
            }
        }
    }
}

void FrmMain::visualizerVisibilityChanged(bool visible)
{
    // Change setUpdatesEnabled2 to something better later
    ui->visualizer->setUpdatesEnabled2(visible);
}

void FrmMain::onConnectionError(QString error)
{
    // @TODO connection
    // if (error != QSerialPort::NoError && error != previousError) {
    //     previousError = error;
    //     ui->txtConsole->appendPlainText(tr("Serial port error ") + QString::number(error) + ": " + m_serialPort.errorString());
    //     if (m_serialPort.isOpen()) {
    //         m_serialPort.close();
    //         updateControlsState();
    //     }
    // }
    ui->console->append(tr("AbstractConnection error ") + error);
    updateControlsState();
}

void FrmMain::onMachinePosChanged(QVector3D pos)
{
    ui->state->setMachineCoordinates(pos);
}

void FrmMain::onWorkPosChanged(QVector3D pos)
{
    ui->state->setWorkCoordinates(pos);
}

void FrmMain::onMachineStateChanged(MachineState state)
{
    ui->state->setState(state);

    ui->control->updateControlsState(currentUiState());

    // ui->spindle->...

    // ui->cmdSpindle->setEnabled(state == DeviceHold0 || ((communicator()->senderState() != SenderTransferring) &&                                                        (communicator()->senderState() != SenderStopping)));
}

void FrmMain::onMachineStateReceived(MachineState state)
{
    // Update controls state
    // ui->control->updateControlsState(state == DeviceState::Idle, communicator()->deviceState());

    // ui->spindle->...
    // ui->cmdSpindle->setEnabled(state == DeviceHold0 || ((communicator()->senderState() != SenderTransferring) &&
    //                                                     (communicator()->senderState() != SenderStopping)));

    // Update elapsed time and remaining time with adaptive correction
    if (communicator()->stateBehavior()->is(AbstractStateBehavior::Type::Running)) {
        ui->visualizer->setTimeEstimation(timeEstimator());
    }

    updateControlsState();
}

void FrmMain::onSenderStateReceived(SenderState state)
{
    Q_UNUSED(state);

    updateControlsState();
}

void FrmMain::onSpindleStateReceived(bool state)
{
    // @TODO Pass spindle state to visualizator
}

void FrmMain::onFloodStateReceived(bool state)
{
    ui->control->setFlood(state);
}

void FrmMain::onParserStateReceived(QString state)
{
    ui->visualizer->setParserState(state);
}

void FrmMain::onPinStateReceived(PinState state)
{
    ui->visualizer->setPinState(state.toString());
}

void FrmMain::onFeedSpindleSpeedReceived(int feedRate, int spindleSpeed)
{
    ui->visualizer->setSpeedState(feedRate, spindleSpeed);
}

void FrmMain::onSpindleSpeedReceived(int spindleSpeed)
{
    // ui->slbSpindle->setCurrentValue(spindleSpeed);
}

// https://github.com/gnea/grbl/blob/master/doc/markdown/commands.md

void FrmMain::onAborted()
{
    //ui->cmdFileAbort->setEnabled(false);
    updateControlsState();
}

void FrmMain::onResponseReceived(QString command, int tableIndex, QString response)
{
    Q_UNUSED(command)
    Q_UNUSED(tableIndex)
    Q_UNUSED(response)

    // updateToolpathShadowingOnCheckMode();
}

void FrmMain::onCommandResponseReceived(CommandAttributes commandAttributes)
{
    ui->console->appendResponse(commandAttributes);
}

void FrmMain::onCommandSent(CommandAttributes commandAttributes)
{
    ui->console->appendFiltered(commandAttributes);
}

// void FrmMain::onCommandProcessed(int tableIndex, QString response)
// {
//     if (ui->chkAutoScrollGCode->isChecked()) {
//         // scroll to NEXT command (+1)
//         ui->tblProgram->scrollTo(m_currentModel->index(tableIndex + 1, 0));      // TODO: Update by timer
//         ui->tblProgram->setCurrentIndex(m_currentModel->index(tableIndex + 1, 1));
//     }

//     if (tableIndex > -1) {
//         m_currentModel->setData(m_currentModel->index(tableIndex, 2), GCodeItem::Processed);
//         m_currentModel->setData(m_currentModel->index(tableIndex, 3), response);
//     }
// }

void FrmMain::onConfigurationReceived(PhysicalMachineConfiguration configuration)
{
    ui->state->setUnits(configuration.units());
}

void FrmMain::onToolPositionReceived(QVector3D pos)
{
    updateToolPositionAndToolpathShadowing(pos);

    // Update time estimator with current progress for adaptive correction
    if (timeEstimator().isTracking()) {
        timeEstimator().updateProgress(program());
    }
}

void FrmMain::updateOnStateBehaviorChanged(AbstractStateBehavior *sb)
{
    ui->state->setStatusText(
        sb->description(),
        colorForGroup(colorGroupForState(sb->type()), ThemeManager::instance().dark()),
        ThemeManager::instance().dark() ? "black" : "white"
    );
    ui->console->appendSystem(QString("State: %1").arg(sb->description()));
    FilesManager& fm = FilesManager::instance();
    updateControlsState({ sb, { fm.gcodeOpened(), fm.heightmapOpened() } });

    if (auto *r = dynamic_cast<RunningBehavior*>(sb)) {
        bindActiveProgram(&r->program());
    } else {
        bindActiveProgram(&program());
    }

    // Auto-dismiss the prompt dialog when the user-prompt state goes away
    // (e.g. user resolved the prompt via a side-channel like the toolbar
    // resume button, or another behavior took over via Reset).
    if (m_userPromptBox && sb->type() != AbstractStateBehavior::Type::UserPrompt) {
        m_userPromptBox->reject();
        m_userPromptBox = nullptr;
        m_activeUserPromptId.clear();
    }
}

void FrmMain::bindActiveProgram(GCode *target)
{
    if (m_activeProgram == target) {
        return;
    }

    if (m_activeProgram) {
        disconnect(m_activeProgram.data(), nullptr, this, nullptr);
    }

    ui->program->setProgram(target);
    m_activeProgram = target;

    if (target) {
        connect(target, &GCode::lastSentCommandChanged, this, [this](int index) {
            ui->program->scrollToIndex(index);
        });
        connect(target, &GCode::nameChanged, this, [this] {
            updateProgramTitle();
        });
    }

    updateProgramTitle();
}

bool FrmMain::programIsCentralWidget() const
{
    return ui->program->parentWidget() == ui->centralWidget;
}

void FrmMain::updateProgramTitle()
{
    QString title;

    if (ui->program->view() == PartMainProgram::View::Heightmap) {
        title = tr("Heightmap");
    } else {
        const GCode *p = m_activeProgram;
        if (p && p->type() == GCodeType::Macro) {
            title = tr("Program - macro `%1`").arg(p->name());
        } else if (p && p->type() == GCodeType::MainProgram && !p->name().isEmpty()) {
            title = tr("Program - main `%1`").arg(p->name());
        } else {
            title = tr("Program - main");
        }
    }

    // Dock path: DockableTitle reacts to windowTitleChanged automatically.
    ui->dockProgram->setWindowTitle(title);

    // Central widget path: refresh the centralWidgetTitle label too.
    if (programIsCentralWidget()) {
        ui->centralWidgetTitle->setTitle(title);
    }
}

void FrmMain::onUserPromptRequested(PromptSpec spec)
{
    if (m_userPromptBox && m_activeUserPromptId == spec.promptId) {
        // Same prompt re-emitted (e.g. recovery coroutine fell back to a
        // re-prompt). Keep the dialog as-is.
        return;
    }
    if (m_userPromptBox) {
        m_userPromptBox->reject();
        m_userPromptBox = nullptr;
    }

    auto *box = new QMessageBox(this);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setIcon(QMessageBox::Question);
    box->setWindowTitle(spec.title.isEmpty() ? tr("AstroCore") : spec.title);
    box->setText(spec.message);

    QHash<QAbstractButton*, QString> buttonChoices;
    QAbstractButton *defaultBtn = nullptr;
    for (const PromptChoice &choice : spec.choices) {
        QMessageBox::ButtonRole role = choice.destructive
            ? QMessageBox::DestructiveRole
            : QMessageBox::AcceptRole;
        QPushButton *btn = box->addButton(choice.label, role);
        buttonChoices.insert(btn, choice.id);
        if (choice.isDefault) {
            defaultBtn = btn;
        }
    }
    if (defaultBtn) {
        box->setDefaultButton(static_cast<QPushButton*>(defaultBtn));
    }

    const QString promptId = spec.promptId;
    connect(box, &QMessageBox::buttonClicked, this, [this, promptId, buttonChoices](QAbstractButton *btn) {
        const QString choiceId = buttonChoices.value(btn);
        if (choiceId.isEmpty()) {
            return;
        }
        m_activeUserPromptId.clear();
        m_userPromptBox = nullptr;
        communicator()->respondToPrompt(promptId, choiceId);
    });

    m_userPromptBox = box;
    m_activeUserPromptId = promptId;
    box->open();  // non-modal — user keeps access to the rest of the UI
}

void FrmMain::programEditLines(int from, int to)
{
    if (communicator()->stateBehavior()->is(AbstractStateBehavior::Type::Running)) {
        return;
    }

    DlgEditProgram dlg(this);
    dlg.setProgramText(program().linesAsText(from, to));
    if (dlg.exec() == QDialog::Accepted) {
        GCode gcode;
        QStringList lines = dlg.programText().split("\n");
        GcodePreprocessorUtils::parseLines(lines, gcode);
        program().replace(from, to, gcode);

        updateParser();
    }
}

void FrmMain::programInsertLines(int current, bool before)
{
    if (communicator()->stateBehavior()->is(AbstractStateBehavior::Type::Running)) {
        return;
    }

    DlgEditProgram dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        GCode gcode;
        QStringList lines = dlg.programText().split("\n");
        GcodePreprocessorUtils::parseLines(lines, gcode);
        program().replace(
            before ? current : current + 1,
            before ? current : current + 1,
            gcode
        );

        updateParser();
    }
}

void FrmMain::programDeleteLines(int from, int to)
{
    if (communicator()->stateBehavior()->is(AbstractStateBehavior::Type::Running)) {
        return;
    }

    program().deleteLines(from, to);

    updateParser();
}

void FrmMain::onTableCellChanged(QModelIndex i1, QModelIndex i2)
{
    Q_UNUSED(i2)

    GCodeTableModel *model = (GCodeTableModel*)sender();

    if (i1.column() != 1) return;
    // Inserting new line at end
    if (i1.row() == (model->rowCount() - 1) && model->data(model->index(i1.row(), 1)).toString() != "") {
        model->setData(model->index(model->rowCount() - 1, 2), GCodeItem::InQueue);
        model->insertRow(model->rowCount());
        ui->program->setCurrentIndex(model->index(i1.row() + 1, 1));
    }

    // Clear cached args
    model->setData(model->index(i1.row(), 5), QVariant());

    // Drop heightmap cache
    if (ui->program->isCurrentModelProgramModel()) {
        ui->program->clearProgramHeightmapModel();
    }

    // Update visualizer
    updateParser();

    // Hightlight w/o current cell changed event (double hightlight on current cell changed)
    QList<LineSegment>& list = viewParser().getLineSegmentList();
    for (int i = 0; i < list.count() && list[i].getLineNumber() <= ui->program->currentModelData(ui->program->currentModelIndex(i1.row(), 4)).toInt(); i++) {
        list[i].setIsHightlight(true);
    }
}

void FrmMain::onTableCurrentChanged(QModelIndex currentIndex, QModelIndex previousIndex)
{
    ui->visualizer->updateToolpathHighlighting(currentIndex.row(), previousIndex.row());
}


void FrmMain::onActRecentFileTriggered()
{
    QAction *action = static_cast<QAction*>(sender());
    QString filePath = action->text();

    if (action != NULL) {
        if (!m_heightmapMode) {
            if (!saveProgramChanges()) {
                return;
            }
            loadFile(filePath);
        } else {
            if (!saveHeightmapChanges()) {
                return;
            }
            HeightmapLoader::loadFromFile(filePath, heightmap());
        }
    }
}

// Starts G-code execution from the currently selected line in the program table,
// optionally sending initialization commands to restore machine state for that position
// void FrmMain::onActSendFromLineTriggered()
// {
//     if (m_currentModel->rowCount() == 1) return;

//     //Line to start from
//     int commandIndex = ui->tblProgram->currentIndex().row();

//     // Set parser state
//     if (m_configuration.senderModule().setParserStateBeforeSendingFromSelectedLine()) {
//         QString commands = getLineInitCommands(commandIndex);

//         QMessageBox box(this);
//         box.setIcon(QMessageBox::Information);
//         box.setText(tr("Following commands will be sent before selected line:\n") + commands);
//         box.setWindowTitle(qApp->applicationDisplayName());
//         box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
//         box.addButton(tr("Skip"), QMessageBox::DestructiveRole);

//         int res = box.exec();
//         if (res == QMessageBox::Cancel) return;
//         else if (res == QMessageBox::Ok) {
//             // foreach (QString command, commands) {
//             //     sendCommand(command, COMMAND_TI_UI);
//             // }
//             communicator()->sendCommands(CommandSource::ProgramAdditionalCommands, commands, TABLE_INDEX_UI);
//         }
//     }

//     program().reset(commandIndex);
//     m_lastDrawnLineIndex = 0;
//     // communicator()->m_probeIndex = -1;

//     QList<LineSegment>& list = viewParser().getLineSegmentList();

//     QList<int> indexes;
//     for (int i = 0; i < list.count(); i++) {
//         list[i].setDrawn(list[i].getLineNumber() < (*m_currentProgram)[commandIndex].lineNumber);
//         indexes.append(i);
//     }
//     m_codeDrawer->update(indexes);

//     ui->tblProgram->setUpdatesEnabled(false);

//     for (int i = 0; i < m_currentProgram->count() - 1; i++) {
//         (*m_currentProgram)[i].state = i < commandIndex ? GCodeItem::Skipped : GCodeItem::InQueue;
//         (*m_currentProgram)[i].response = QString();
//     }
//     ui->tblProgram->setUpdatesEnabled(true);
//     ui->glwVisualizer->setSpendTime(QTime(0, 0, 0));

//     m_startTime = QDateTime::currentSecsSinceEpoch();

//     communicator()->setSenderStateAndEmitSignal(SenderState::Transferring);

//     ui->jog->storeAndResetKeyboardControl();
//     // m_storedKeyboardControl = ui->chkKeyboardControl->isChecked();
//     // ui->chkKeyboardControl->setChecked(false);

//     communicator()->storeParserState();

// #ifdef WINDOWS
//     // if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7) {
//     //     if (m_taskBarProgress) {
//     //         m_taskBarProgress->setMaximum(m_currentModel->rowCount() - 2);
//     //         m_taskBarProgress->setValue(commandIndex);
//     //         m_taskBarProgress->show();
//     //     }
//     // }
// #endif

//     updateControlsState();
//     ui->cmdFilePause->setFocus();

//     program().reset(commandIndex);
//     // communicator()->sendStreamerCommandsUntilBufferIsFull();
// }

void FrmMain::onSlbSpindleValueUserChanged()
{
    // communicator()->m_updateSpindleSpeed = true;
}

void FrmMain::onSlbSpindleValueChanged()
{
    // if (!ui->grpSpindle->isChecked() && ui->cmdSpindle->isChecked())
    //     ui->grpSpindle->setTitle(tr("Spindle") + QString(tr(" (%1)")).arg(ui->slbSpindle->value()));
}

// void FrmMain::onCboCommandReturnPressed()
// {
//     QString command = ui->cboCommand->currentText();
//     if (command.isEmpty()) return;

//     ui->cboCommand->setCurrentText("");
//     communicator()->sendCommand(command, COMMAND_TI_UI);
// }

void FrmMain::onDockTopLevelChanged(bool topLevel)
{
    Q_UNUSED(topLevel)
    static_cast<QWidget*>(sender())->setStyleSheet("");
}

// void FrmMain::onProgramLinesUpdated(int from, int to)
// {
//     qDebug() << "FrmMain::onProgramLinesUpdated from" << from << "to" << to;
// }

// void FrmMain::updateHeightmapInterpolationDrawer(bool reset)
// {
//     if (m_settingsLoading) return;

//     QRectF borderRect = ui->heightmap->areaRectFromTextboxes();
//     // m_heightmapInterpolationDrawer.setBorderRect(borderRect);

//     QVector<QVector<double>> *interpolationData = new QVector<QVector<double>>;

//     int interpolationPointsX = heightmap().interpolationStepSize().width();// * (ui->txtHeightMapGridX->value() - 1) + 1;
//     int interpolationPointsY = heightmap().interpolationStepSize().height();// * (ui->txtHeightMapGridY->value() - 1) + 1;

//     double interpolationStepX = interpolationPointsX > 1 ? borderRect.width() / (interpolationPointsX - 1) : 0;
//     double interpolationStepY = interpolationPointsY > 1 ? borderRect.height() / (interpolationPointsY - 1) : 0;

//     for (int i = 0; i < interpolationPointsY; i++) {
//         QVector<double> row;
//         for (int j = 0; j < interpolationPointsX; j++) {

//             double x = interpolationStepX * j + borderRect.x();
//             double y = interpolationStepY * i + borderRect.y();

//             row.append(reset ? qQNaN() : Interpolation::bicubicInterpolate(borderRect, ui->program->getHeightmapModelForInterpolation(), x, y));
//         }
//         interpolationData->append(row);
//     }

//     // if (m_heightmapInterpolationDrawer.data() != NULL) {
//     //     delete m_heightmapInterpolationDrawer.data();
//     // }
//     ui->visualizer->setInterpolationData(interpolationData, borderRect);

//     // Update grid drawer
//     ui->visualizer->updateHeightmapGrid();

//     // Reset heightmapped program model
//     ui->program->clearProgramHeightmapModel();
// }

void FrmMain::onHeightmapDataChangedByUser()
{
    FilesManager::instance().setHeightmapModified(true);
    // updateHeightmapInterpolationDrawer();
}

void FrmMain::onGridParametersChanged(QSize gridSize, PartMainHeightmap::MinMax zMinMax, int probeFeed, QSize interpolationStep)
{
    auto& cfg = ConfigurationHeightmap::instance();
    cfg.setProperty("gridX", gridSize.width());
    cfg.setProperty("gridY", gridSize.height());
    cfg.setProperty("gridZBottom", zMinMax.min);
    cfg.setProperty("gridZTop", zMinMax.max);
    cfg.setProperty("probeFeed", probeFeed);
    cfg.setProperty("interpolationStepX", interpolationStep.width());
    cfg.setProperty("interpolationStepY", interpolationStep.height());

    Heightmap& hm = heightmap();
    hm.beginUpdate();
    hm.setGridSize(gridSize);
    hm.setZBottomTop({ zMinMax.min, zMinMax.max });
    hm.setInterpolationStepSize(QSizeF(interpolationStep.width(), interpolationStep.height()));
    hm.endUpdate();

    updateHeightmapGrid();
}

void FrmMain::preloadSettings()
{
    ConfigurationUI &uiConfiguration = UiConfigs::instance().ui();
    ConfigurationVisualizer &visualizerConfiguration = UiConfigs::instance().visualizer();

    ThemeManager::instance().setScale(uiConfiguration.uiScale());
    for (auto action : ui->menuUIScale->actions()) {
        action->setChecked(action->property("scale").toDouble() == uiConfiguration.uiScale());
    }

    // Update v-sync in glformat
    // QGLFormat fmt = QGLFormat::defaultFormat();
    // fmt.setSwapInterval(visualizerConfiguration.vsync() ? 1 : 0);
    // QGLFormat::setDefaultFormat(fmt);
}

void FrmMain::applyOverridesConfiguration(ConfigurationMachine &machineConfiguration)
{
    // ui->slbFeedOverride->setChecked(machineConfiguration.overrideFeed());
    // ui->slbFeedOverride->setValue(machineConfiguration.overrideFeedValue());

    // ui->slbRapid->setChecked(machineConfiguration.overrideRapid());
    // ui->slbRapid->setValue(machineConfiguration.overrideRapidValue());

    // ui->slbSpindle->setChecked(machineConfiguration.overrideSpindleSpeed());
    // ui->slbSpindle->setValue(machineConfiguration.overrideSpindleSpeedValue());
}

void FrmMain::applySpindleConfiguration(ConfigurationMachine &machineConfiguration)
{
    // ui->slbSpindle->setRatio(machineConfiguration.spindleSpeedRatio());
    // ui->slbSpindle->setMinimum(machineConfiguration.spindleSpeedRange().min);
    // ui->slbSpindle->setMaximum(machineConfiguration.spindleSpeedRange().max);
    // ui->slbSpindle->setValue(machineConfiguration.spindleSpeed());
}

void FrmMain::applyHeightmapConfiguration(ConfigurationHeightmap &heightmapConfiguration)
{
    ui->heightmap->applyHeightmapConfiguration(heightmapConfiguration);
}

void FrmMain::loadSettings()
{
    emit settingsAboutToLoad();

//    this->restoreGeometry(set.value("formGeometry", QByteArray()).toByteArray());

    // ui->cboCommand->setMinimumHeight(ui->cboCommand->height());
    // ui->cmdClearConsole->setFixedHeight(ui->cboCommand->height());
    // ui->cmdCommandSend->setFixedHeight(ui->cboCommand->height());

    // m_storedKeyboardControl = set.value("keyboardControl", false).toBool();

    // QStringList steps = set.value("jogSteps").toStringList();
    // if (steps.count() > 0) {
    //     steps.insert(0, ui->cboJogStep->items().first());
    //     ui->cboJogStep->setItems(steps);
    // }
    // ui->cboJogStep->setCurrentIndex(ui->cboJogStep->findText(set.value("jogStep").toString()));
    // ui->cboJogFeed->setItems(set.value("jogFeeds").toStringList());
    // ui->cboJogFeed->setCurrentIndex(ui->cboJogFeed->findText(set.value("jogFeed").toString()));

    // foreach (ColorPicker* pick, m_settings->colors()) {
    //     pick->setColor(QColor(set.value(pick->objectName().mid(3), "black").toString()));
    // }


    // Apply settings
    applySettings();

    // Shortcuts
    ShortcutsManager::instance().importList(UiConfigs::instance().ui().shortcuts());

    // Menu
    ConfigurationUI &uiConfiguration = UiConfigs::instance().ui();
    ui->actViewLockWindows->setChecked(uiConfiguration.lockWindows());
    ui->actViewLockPanels->setChecked(uiConfiguration.lockPanels());
    ui->actViewDarkMode->setChecked(uiConfiguration.darkTheme());
    // @TODO move to configuration form
    //m_settings->restoreGeometry(set.value("formSettingsGeometry", m_settings->saveGeometry()).toByteArray());

    emit settingsLoaded();
}

void FrmMain::restoreDockableLayoutState()
{
    ConfigurationUI& uiConfiguration = UiConfigs::instance().ui();

    ui->program->restoreHeaderState(uiConfiguration.programHeaderState());
    restoreGeometry(uiConfiguration.mainFormGeometryData());

    // Adjust docks width
    int w = qMax(ui->dockDevice->widget()->sizeHint().width(),
        ui->dockModification->widget()->sizeHint().width());
    // ui->dockDevice->setMinimumWidth(w);
    // ui->dockDevice->setMaximumWidth(w + ui->dockDeviceScrollArea->verticalScrollBar()->width());
    // ui->dockModification->setMinimumWidth(w);
    // ui->dockModification->setMaximumWidth(w + ui->dockModificationScrollArea->verticalScrollBar()->width());
    // ui->dockUser->setMinimumWidth(w);
    // ui->dockUser->setMaximumWidth(w + ui->dockUserScrollArea->verticalScrollBar()->width());

    // Buttons
    // int b = (w - ui->grpControl->layout()->margin() * 2 - ui->grpControl->layout()->spacing() * 3) / 4 * 0.8;
    // int c = b * 0.8;
    // setStyleSheet(styleSheet() + QString("\nStyledToolButton[adjustSize='true'] {\n\
       //  min-width: %1px;\n\
       //  min-height: %1px;\n\
       //  qproperty-iconSize: %2px;\n\
    //     }").arg(b).arg(c));
    // ensurePolished();

    // foreach (QDockWidget *w, findChildren<QDockWidget*>()) {
    //     w->setStyleSheet("");
    // }

    // Restore docks
    // Signals/slots
    foreach (QDockWidget *w, findChildren<QDockWidget*>()) {
        // connect(w, &QDockWidget::topLevelChanged, this, &FrmMain::onDockTopLevelChanged);
    }

    // Panels
    ui->scrollContentsDevice->restoreState(this, uiConfiguration.panelDeviceState());
    ui->scrollContentsModification->restoreState(this, uiConfiguration.panelModificationState());
    ui->scrollContentsUser->restoreState(this, uiConfiguration.panelUserState());

    QStringList hiddenPanels = uiConfiguration.hiddenPanels();
    foreach (QString s, hiddenPanels) {
        QGroupBox *b = findChild<QGroupBox*>(s);
        if (b) { b->setHidden(true); }
    }

    QStringList collapsedPanels = uiConfiguration.collapsedPanels();
    foreach (QString s, collapsedPanels) {
        QGroupBox *b = findChild<QGroupBox*>(s);
        if (b) { b->setChecked(false); }
    }

    // Normal window state
    restoreState(uiConfiguration.mainFormState());

    // Hide central widget dock
    for (auto dock : findChildren<QDockWidget*>()) {
        if (dock->property("cw").toBool() == true) {
            dock->setVisible(false);
        }
    }
}

void FrmMain::saveSettings()
{
    emit settingsAboutToSave();

    ConfigurationUI &uiConfiguration = UiConfigs::instance().ui();

    uiConfiguration.setAutoScrollGCode(ui->program->isAutoScroll());
    uiConfiguration.setProgramHeaderState(ui->program->saveHeaderState());
    uiConfiguration.setMainFormState(saveState());
    uiConfiguration.setMainFormGeometryData(saveGeometry());

    // Shortcuts
    uiConfiguration.setShortcuts(ShortcutsManager::instance().exportList());

    // Panels
    uiConfiguration.setPanelModificationState(ui->scrollContentsModification->saveState());
    uiConfiguration.setPanelDeviceState(ui->scrollContentsDevice->saveState());
    uiConfiguration.setPanelUserState(ui->scrollContentsUser->saveState());
    qDebug() << "[FrmMain] Saving panels state:" << ui->scrollContentsUser->saveState();

    QStringList panels;
    QStringList hiddenPanels;
    QStringList collapsedPanels;
    if (ui->scrollContentsDevice->isVisible()) panels << ui->scrollContentsDevice->saveState();
    if (ui->scrollContentsModification->isVisible()) panels << ui->scrollContentsModification->saveState();
    if (ui->scrollContentsUser->isVisible()) panels << ui->scrollContentsUser->saveState();
    foreach (QString s, panels) {
        QGroupBox *b = findChild<QGroupBox*>(s);
        if (b && b->isHidden()) hiddenPanels << s;
        if (b && b->isCheckable() && !b->isChecked()) collapsedPanels << s;
    }
    uiConfiguration.setHiddenPanels(hiddenPanels);
    uiConfiguration.setCollapsedPanels(collapsedPanels);

    // Menu
    uiConfiguration.setLockPanels(ui->actViewLockPanels->isChecked());
    uiConfiguration.setLockWindows(ui->actViewLockWindows->isChecked());

    m_configuration.save();

    emit settingsSaved();
}

void FrmMain::initializeConnection(ConfigurationConnection::ConnectionMode mode)
{
    // Core::setConnection deletes the previous connection before storing the new one.
    Core::instance().setConnection(connectionManager().createConnection(mode));

    connect(connection(), &AbstractConnection::error, this, &FrmMain::onConnectionError);
}

void FrmMain::applyUIConfiguration(ConfigurationUI &uiConfiguration)
{
    ui->program->setAutoScroll(uiConfiguration.autoScrollGCode());
    ui->actViewDarkMode->setChecked(uiConfiguration.darkTheme());
    ThemeManager& tm = ThemeManager::instance();
    tm.setScale(uiConfiguration.uiScale());
    tm.setDark(uiConfiguration.darkTheme());
}

void FrmMain::applyJoggingConfiguration(ConfigurationJogging &joggingConfiguration)
{
    ui->jog->configurationUpdated();
}

void FrmMain::appendPanel(DropWidget *dockPanel, const QString name, const QString title, QWidget *panel)
{
    QGroupBox *grp = new QGroupBox(tr(title.toStdString().c_str()));
    grp->setCheckable(true);
    grp->setObjectName("grp" + name);
    connect(grp, &QGroupBox::toggled, dockPanel, [panel](bool checked) {
        panel->setVisible(checked);
    });
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(panel);
    grp->setLayout(layout);
    dockPanel->layout()->addWidget(grp);
}

void FrmMain::appendSpacer(DropWidget *dockPanel)
{
    QGroupBox *grp = new QGroupBox();
    QSizePolicy sp = grp->sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Preferred);
    sp.setVerticalPolicy(QSizePolicy::Expanding);
    sp.setHorizontalStretch(0);
    sp.setVerticalStretch(0);
    grp->setStyleSheet("QGroupBox {	margin-top: 7; }");
    QVBoxLayout *layout = (QVBoxLayout*)dockPanel->layout();
    layout->addWidget(grp);
    layout->setStretchFactor(grp, 1);
}

void FrmMain::addDockableWindow(const QString title, const QString name, QWidget *widget, Qt::DockWidgetArea area, Qt::Orientation orientation)
{
    QDockWidget *dock = new QDockWidget(tr(title.toStdString().c_str()));
    dock->setObjectName("dock-" + name);
    dock->setMinimumHeight(200);
    dock->setWidget(widget);
    Utils::setDockableLocked(dock, UiConfigs::instance().ui().lockWindows());
    dock->setTitleBarWidget(new DockableTitle(dock));
    addDockWidget(area, dock, orientation);

    QAction* action = new QAction(title, ui->menuCentralWidget);
    action->setCheckable(true);
    action->setChecked(false);
    // connect(action, &QAction::triggered, this, &FrmMain::centralWidgetActionTriggered);
    connect(action, &QAction::triggered, this, &FrmMain::centralWidgetActionTriggered);
    ui->menuCentralWidget->addAction(action);

    m_centralWidgets.append({
        widget,
        dock,
        action,
        name,
        title
    });
}

void FrmMain::applySettings()
{
    ConfigurationVisualizer &visualizerConfiguration = UiConfigs::instance().visualizer();
    ConfigurationHeightmap &heightmapConfiguration = ConfigurationHeightmap::instance();
    ConfigurationMachine &machineConfiguration = m_configuration.machineModule();
    ConfigurationUI &uiConfiguration = UiConfigs::instance().ui();
    ConfigurationJogging &joggingConfiguration = m_configuration.joggingModule();

    ui->visualizer->applyVisualizerConfiguration(visualizerConfiguration, machineConfiguration);

    // @TODO watch for changes is communicator?
    // communicator()->stopUpdatingState();
    // communicator()->startUpdatingState(m_configuration.connectionModule().queryStateInterval());

    applySpindleConfiguration(machineConfiguration);
    applyJoggingConfiguration(joggingConfiguration);
    applyOverridesConfiguration(machineConfiguration);
    applyUIConfiguration(uiConfiguration);
    updateRecentFilesMenus();
    applyHeightmapConfiguration(heightmapConfiguration);

    if (!connection() || connection()->supportedMode() != m_configuration.connectionModule().connectionMode()) {
        initializeConnection(m_configuration.connectionModule().connectionMode());

        if (communicator()->connection()) {
            if (!communicator()->startReconnecting(connection())) {
                ui->console->appendSystem("Couldn't update connection. Restart application.");
            }
        } else {
            communicator()->setConnection(connection(), false);
        }
    }
}

void FrmMain::updateParser()
{
    if (m_visualizerUpdater) {
        // Update in progress, cancel it
        m_visualizerUpdater->cancel();
        delete m_visualizerUpdater;
        m_visualizerUpdater = nullptr;
    }

    m_visualizerUpdater = new GCodeThreadedLoader(this);
    connect(m_visualizerUpdater, &GCodeThreadedLoader::cancelled, this, [this]() {
        qDebug() << "[FrmMain] Visualizer update cancelled";
        m_visualizerUpdater->deleteLater();
        m_visualizerUpdater = nullptr;
    });
    connect(m_visualizerUpdater, &GCodeThreadedLoader::finished, this, [this](GCodeLoaderData *data) {
        qDebug() << "[FrmMain] Finished updating visualizer data";
        this->applyUpdaterGCode(data);

        // In update mode data->gcode == &program() — do NOT delete it.
        // Only the freshly built viewParser is orphaned and must be freed here.
        delete data->viewParser;
        delete data;
        m_visualizerUpdater->deleteLater();
        m_visualizerUpdater = nullptr;
    });

    m_visualizerUpdater->update(&program());


    // GCodeViewParser *viewParse = ui->visualizer->getCurrentParser();

    // GcodeParser parser;
    // parser.setTraverseSpeed(communicator()->machineConfiguration().maxRate().x()); // uses only x axis speed
    // if (UiConfigs::instance().visualizer().ignoreZ()) {
    //     parser.reset(QVector3D(qQNaN(), qQNaN(), 0));
    // }

    // ui->program->setTableUpdatesEnabled(false);

    // QString stripped;
    // QList<QString> args;

    // QProgressDialog progress(tr("Updating..."), tr("Abort"), 0, ui->program->currentModelRowCount() - 2, this);
    // progress.setWindowModality(Qt::WindowModal);
    // progress.setFixedSize(progress.sizeHint());

    // if (ui->program->currentModelRowCount() > PROGRESS_MIN_LINES) {
    //     progress.show();
    //     progress.setStyleSheet("QProgressBar {text-align: center; qproperty-format: \"\"}");
    // }

    // for (int i = 0; i < ui->program->currentModelRowCount() - 1; i++) {
    //     // Get stored args
    //     args = (*m_currentProgram)[i].args;

    //     // Store args if none
    //     if (args.isEmpty()) {
    //         stripped = GcodePreprocessorUtils::removeComment((*m_currentProgram)[i].command);
    //         args = GcodePreprocessorUtils::splitCommand(stripped);
    //         (*m_currentProgram)[i].args = args;
    //     }

    //     // Add command to parser
    //     parser.addCommand(args);

    //     // Update table model
    //     (*m_currentProgram)[i].state = GCodeItem::InQueue;
    //     (*m_currentProgram)[i].response = QString();
    //     (*m_currentProgram)[i].lineNumber = parser.getCommandNumber();

    //     if (progress.isVisible() && (i % PROGRESS_STEP == 0)) {
    //         progress.setValue(i);
    //         qApp->processEvents();
    //         if (progress.wasCanceled()) break;
    //     }
    // }
    // progress.close();

    // ui->program->setTableUpdatesEnabled(true);

    // viewParse->reset();

    // ConfigurationParser &configurationParser = m_configuration.parserModule();

    // // updateProgramEstimatedTime(
    //     // viewParse->getLinesFromParser(
    //     //     &parser,
    //     //     configurationParser.arcApproximationValue(),
    //     //     configurationParser.arcApproximationMode() == ConfigurationParser::ParserArcApproximationMode::ByAngle
    //     // )
    // // );
    // ui->visualizer->updateCurrentDrawerGeometry();
    // ui->visualizer->updateGCodeExtremes();
    // updateControlsState();

    // if (ui->program->isCurrentModelProgramModel()) {
    //     FilesManager& fm = FilesManager::instance();
    //     fm.setGcodeModified(true);
    // }
}

void FrmMain::loadFile(QString filePath)
{
    GCodeThreadedLoader *loader = new GCodeThreadedLoader(this);
    int progressIndex = ui->console->appendProgress("Loading " + filePath);
    connect(loader, &GCodeThreadedLoader::progress, this, [this, progressIndex](int progress) {
        ui->console->setProgress(progressIndex, progress);
        #ifdef WINDOWS
            m_taskBar.setProgress(progress, 100);
        #endif
    });
    connect(loader, &GCodeThreadedLoader::cancelled, this, [this, loader]() {
        qDebug() << "[FrmMain] Loading cancelled";
        ui->console->appendSystem("Cancelled loading");
        loader->deleteLater();
    });
    connect(loader, &GCodeThreadedLoader::finished, this, [this, loader, filePath](GCodeLoaderData *data) {
        qDebug() << "[FrmMain] Finished loading file" << data->gcode->count();
        ui->console->appendSystem("Finished loading");
        #ifdef WINDOWS
            m_taskBar.hide();
        #endif
        this->applyLoaderGCode(data);

        // GCodeLoaderData only holds raw pointers. The worker allocated both
        // gcode and viewParser with `new`; after applyLoaderGCode has consumed
        // them (deep copy / bulk append) we are the owner and must free them.
        delete data->gcode;
        delete data->viewParser;
        delete data;
        loader->deleteLater();

        FilesManager& filesManager = FilesManager::instance();
        filesManager.setGcodeFilePath(filePath);
    });

    loader->loadFromFile(filePath);
}

void FrmMain::applyUpdaterGCode(GCodeLoaderData *data)
{
    viewParser() = *data->viewParser;

    timeEstimator().calculateEstimatedTime(
        viewParser().getLines(),
        communicator()->overrides()->targetFeed(),
        communicator()->overrides()->targetRapid()
    );
    ui->visualizer->setTimeEstimation(timeEstimator());

    ui->visualizer->setProgram(&program(), &viewParser());
    ui->visualizer->updateCodeDrawer();
}

// Replaces program() and viewParser() with freshly loaded data. Organised in
// three phases:
//   A) detach — cancel background work, drop consumer references to old state
//   B) swap   — clear program under signal blocker, deep-copy new view parser
//   C) attach — rebind models/drawers, populate program (triggers `loaded()`
//               which the program table model uses to reset itself)
void FrmMain::applyLoaderGCode(GCodeLoaderData *data)
{
    // --- PHASE A: detach ---

    // A background visualizer update may still hold &program(). Disconnect
    // first so its queued finished/cancelled slots cannot run after we null
    // m_visualizerUpdater below.
    if (m_visualizerUpdater) {
        m_visualizerUpdater->disconnect();
        m_visualizerUpdater->cancel();
        delete m_visualizerUpdater;
        m_visualizerUpdater = nullptr;
    }

    // Loader is async: the "open" button is disabled when state != Idle, but
    // pendant or scripted actions could still push the machine into Running
    // between load start and load finish. Replacing program() while the
    // RunningBehavior is iterating it crashes the sender.
    if (!communicator()->stateBehavior()->isOneOf(AbstractStateBehavior::Type::Idle, AbstractStateBehavior::Type::Alarm)) {
        qWarning() << "[FrmMain] applyLoaderGCode: machine is not idle, discarding loaded data";
        ui->console->appendSystem(tr("Cannot replace program: machine is not idle"));
        return;
    }

    const QByteArray headerState = ui->program->saveProgramHeaderState();

    ui->program->close();              // table models drop their GCode* source
    ui->visualizer->close();           // code drawer drops its GCodeViewParser*

    timeEstimator().resetEstimation(); // drops cached pointer into viewParser().getLines()

    // --- PHASE B: swap ---

    {
        QSignalBlocker blocker(program());
        program().clear();
        program().reset();
    }

    // Deep copy. GCodeViewParser has no user-defined assignment but its
    // members (QList<LineSegment>, QVector3D, ...) are all value types,
    // and LineSegment has no owning pointers — so default assignment
    // produces an independent copy.
    viewParser() = *data->viewParser;
    probeParser().reset();

    // --- PHASE C: attach ---

    ui->heightmap->resetUseHeighmap();
    ui->grpHeightmap->setProperty("overrided", false);
    Utils::refreshStyle(ui->grpHeightmap);

    // Rebind models/drawers to the (still empty) new program BEFORE the bulk
    // append. `operator<<(const GCode&)` emits `loaded()` synchronously and
    // the connected table model resets itself in response — only useful if
    // the model is already pointing at program() again.
    ui->program->setProgram(&program());
    ui->program->switchToProgramModel();
    ui->program->restoreHeaderState(headerState);

    ui->visualizer->setProgram(&program(), &viewParser());

    // Populate program. Triggers `loaded()` → table model reset → view refresh.
    program() << *data->gcode;

    timeEstimator().calculateEstimatedTime(
        viewParser().getLines(),
        communicator()->overrides()->targetFeed(),
        communicator()->overrides()->targetRapid()
    );
    ui->visualizer->setTimeEstimation(timeEstimator());

    ui->visualizer->updateCodeDrawer();
    ui->visualizer->fitCodeDrawer();

    ui->program->selectFirstRow();

    updateControlsState();
}

// Converter index:
//   0 - ArcsToLines (G2/G3 -> G1 segments, 0.1 mm chord)
//   1 - FusionRestoreRapidMovements (G1 reposition moves -> G0)
//   2 - FeedRateConverter (multiply feed rate x2)
//   3 - CoordinateOffsetConverter (X+10 mm offset)
//   4 - SafeSpindleStopConverter
//   5 - MovementOptimizerConverter
//   6 - ShakingGCode (random path distortion, for testing)
//   7 - ApplyHeightmap (uses current heightmap(), 1 mm segments)
//   8 - StripComments (remove all inline and standalone comments)
//   9 - MovePath (shift path by X+10, Y+5, Z+0)
//  10 - ModifyFeedRate (150% of original feed rate)
void FrmMain::testConverter(int converterIndex)
{
    if (program().count() == 0) {
        qDebug() << "[FrmMain] testConverter: no program loaded";
        return;
    }

    FrmConverterSettings settings(this);
    switch (converterIndex) {
        case 0: settings.loadConfig(ArcsToLines::parameterSchema()); break;
        case 1: settings.loadConfig(FusionRestoreRapidMovements::parameterSchema()); break;
        case 2: settings.loadConfig(FeedRateConverter::parameterSchema()); break;
        case 3: settings.loadConfig(CoordinateOffsetConverter::parameterSchema()); break;
        case 4: settings.loadConfig(SafeSpindleStopConverter::parameterSchema()); break;
        case 5: settings.loadConfig(MovementOptimizerConverter::parameterSchema()); break;
        case 6: settings.loadConfig(ShakingGCode::parameterSchema()); break;
        case 7: settings.loadConfig(ApplyHeightmap::parameterSchema()); break;
        case 8: settings.loadConfig(StripComments::parameterSchema()); break;
        case 9: settings.loadConfig(MovePath::parameterSchema()); break;
        case 10: settings.loadConfig(ModifyFeedRate::parameterSchema()); break;
        default:
            qDebug() << "[FrmMain] testConverter: unknown index" << converterIndex;
            return;
    }
    if (settings.exec() != QDialog::Accepted) return;

    QVariantMap values = settings.values();

    StreamConverter *converter = nullptr;

    switch (converterIndex) {
        case 0: converter = new ArcsToLines(values["arcPrecision"].toDouble(), values["arcDegreeMode"].toBool()); break;
        case 1: converter = new FusionRestoreRapidMovements(); break;
        case 2: converter = new FeedRateConverter(values["multiplier"].toDouble()); break;
        case 3: converter = new CoordinateOffsetConverter(values["offsetX"].toDouble()); break;
        case 4: converter = new SafeSpindleStopConverter(); break;
        case 5: converter = new MovementOptimizerConverter(); break;
        case 6: converter = new ShakingGCode(values["segmentLength"].toDouble(), values["maxOffset"].toDouble(), values["shakeZ"].toBool()); break;
        case 7: converter = new ApplyHeightmap(&heightmap(), values["segmentLength"].toDouble()); break;
        case 8: converter = new StripComments(); break;
        case 9: converter = new MovePath(values["offsetX"].toDouble(), values["offsetY"].toDouble(), values["offsetZ"].toDouble()); break;
        case 10: converter = new ModifyFeedRate(values["percent"].toDouble()); break;
        default:
            qDebug() << "[FrmMain] testConverter: unknown index" << converterIndex;
            return;
    }

    GCode *result = converter->convertAll(program());
    delete converter;

    if (!result) {
        qDebug() << "[FrmMain] testConverter: converter returned null";
        return;
    }

    qDebug() << "[FrmMain] testConverter[" << converterIndex << "]:"
             << program().count() << "->" << result->count() << "lines";

    {
        QSignalBlocker blocker(program());
        program().clear();
        program().reset();
    }
    program() << *result;
    delete result;

    updateParser();
}

bool FrmMain::saveProgramChanges()
{
    FilesManager& fm = FilesManager::instance();
    if (!fm.gcodeModified()) {
        return true;
    }

    int res = QMessageBox::warning(this, this->windowTitle(),
                                   tr("G-code program file was changed. Save?"),
                                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (res == QMessageBox::Cancel) {
        return false;
    }
    if (res == QMessageBox::Yes) {
        fileSave();
    }

    fm.setGcodeModified(false);
    return true;
}

bool FrmMain::saveHeightmapChanges()
{
    FilesManager& fm = FilesManager::instance();
    if (!fm.heightmapModified()) {
        return true;
    }

    int res = QMessageBox::warning(this, this->windowTitle(),
                                   tr("Heightmap file was changed. Save?"),
                                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (res == QMessageBox::Cancel) {
        return false;
    }
    if (res == QMessageBox::Yes) {
        // fileSave() reads m_heightmapMode to pick the exporter; force it for the duration of the save.
        bool prevMode = m_heightmapMode;
        m_heightmapMode = true;
        fileSave();
        m_heightmapMode = prevMode;
        updateRecentFilesMenus();
    }

    fm.setHeightmapModified(false);
    return true;
}

void FrmMain::resetHeightmap()
{
    // delete m_heightmapInterpolationDrawer.data();
    ui->visualizer->updateHeightmapInterpolation(true);

    ui->program->setHeightmap(NULL);
    ui->program->resizeHeightmapModel(1, 1);

    ui->heightmap->resetOpenFile();

    FilesManager& fm = FilesManager::instance();
    fm.resetHeightmapFile();
    fm.setHeightmapModified(false);
}

// 1. Zamknac ui program i visualizer
// 2. Zresetować parsery
// 3. Wyczyścić program
// 4. Zresetować estymację czasu
// ...
// 7.
void FrmMain::newProgram()
{
    if (!saveProgramChanges()) {
        return;
    }

    ui->program->close();
    ui->visualizer->close();

    viewParser().reset();
    probeParser().reset();
    program().clear();

    timeEstimator().resetEstimation();
    ui->visualizer->setTimeEstimation(timeEstimator());

    FilesManager::instance().resetGcodeFile();
    ui->heightmap->resetUseHeighmap();
    QByteArray headerState = ui->program->saveHeaderState();

    ui->program->switchToProgramModel();
    ui->program->restoreHeaderState(headerState);

    ui->program->selectFirstRow();

    resetHeightmap();

    ui->program->setProgram(&program());
    ui->visualizer->setProgram(&program(), nullptr);
    ui->visualizer->updateCodeDrawer();

    updateControlsState();
}

void FrmMain::newHeightmap()
{
    if (!saveHeightmapChanges()) {
        return;
    }

    ui->program->clearHeightmapModel();
    onFileReset();
    ui->heightmap->setOpenFile(tr("Untitled"));

    FilesManager& fm = FilesManager::instance();
    fm.resetHeightmapFile();

    //TODO heightmap
    // updateHeightmapBorderDrawer();
    updateHeightmapGrid();

    updateControlsState();
}

UiState FrmMain::currentUiState() const
{
    FilesManager& fm = FilesManager::instance();

    return {
        Core::instance().communicator()->stateBehavior(),
        {
         fm.gcodeOpened(),
         fm.heightmapOpened()
        },
    };
}

void FrmMain::updateControlsState(const UiState& state)
{
    bool portOpened = connection() && connection()->isConnected();
    AbstractStateBehavior *sb = state.sb;
    bool running = sb->is(AbstractStateBehavior::Type::Running);
    bool paused = sb->is(AbstractStateBehavior::Type::Pause) || sb->is(AbstractStateBehavior::Type::ToolChange);
    bool idle = sb->is(AbstractStateBehavior::Type::Idle);

    // ui->grpState->setEnabled(portOpened);
    // ui->control->setEnabled(portOpened);
    ui->spindle->setEnabled(portOpened);
    // TODO: add Action::Jog to ToolChangeBehavior::availableActions(), then simplify to sb->canExecute(Action::Jog)

    ui->console->setEnabled(portOpened && !m_configuration.joggingModule().keyboardControl());
    // ui->cmdCommandSend->setEnabled(portOpened);

    ui->control->updateControlsState(state);
    ui->jog->updateControlsState(state);

    //ui->spindle->...
    // ui->cmdSpindle->setEnabled(!running);

    bool canOpenFile = UiPermissions::isAllowed(UiPermission::OpenFile, sb->type());
    ui->actFileNew->setEnabled(idle);
    ui->actFileOpen->setEnabled(canOpenFile);
    // ui->program->setOpenButtonEnabled(canOpenFile);
    // ui->program->setResetButtonEnabled(idle && !program().empty());
    // ui->program->setSendButtonEnabled(sb->canExecute(Action::Type::Run) && !program().empty());
    ui->program->updateControlsState(state);
    // switch (senderState) {
    //     case SenderState::Pausing:
    //     case SenderState::Pausing2:
    //         ui->cmdFilePause->setText(tr("Pausing..."));
    //         break;
    //     case SenderState::Paused:
    //     case SenderState::ChangingTool:
    //         ui->cmdFilePause->setText(tr("Resume"));
    //         break;
    //     default:
    //         ui->cmdFilePause->setText(tr("Pause"));
    //         break;
    // }
    // ui->cmdFilePause->setEnabled(true);//portOpened && (process || paused) && (senderState != SenderState::Pausing) && (senderState != SenderState::Pausing2));
    // ui->cmdFilePause->setChecked(paused);
    // ui->program->setAbortButtonEnabled(senderState != SenderState::Stopped && senderState != SenderState::Stopping);
    ui->menuRecent->setEnabled(
        idle &&
        ((UiConfigs::instance().ui().hasAnyRecentFiles() && !m_heightmapMode) || (UiConfigs::instance().ui().hasAnyRecentHeightmaps() && m_heightmapMode))
    );
    ui->actFileSave->setEnabled(!program().empty());
    ui->actFileSaveAs->setEnabled(!program().empty());

    ui->program->setProgramTableEditTriggers(!idle ? QAbstractItemView::NoEditTriggers :
        QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked |
        QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);

    if (!running) {
        ui->jog->restoreKeyboardControl();
    }

#ifdef WINDOWS
    m_taskBar.setPaused(paused);
    if (idle) {
        m_taskBar.hide();
    }
#endif

    ui->program->updateButtonStyles();

    // Heightmap
    // m_heightmapBorderDrawer.setVisible(ui->chkHeightMapBorderShow->isChecked() && m_heightmapMode);
    // m_heightmapGridDrawer.setVisible(true);//ui->chkHeightMapGridShow->isChecked() && m_heightmapMode);
    ui->visualizer->setHeightmapInterpolationVisible(ui->heightmap->showInterpolationGrid() && m_heightmapMode);

    // TODO Central widget
    // We can't do this since we don't know what is our current central widget
    // ui->centralWidgetTitle->setTitle(m_heightmapMode ? tr("Heightmap") : tr("G-code program"));
    // ui->centralWidgetTitle->setProperty("overrided", m_heightmapMode);

    // ui->cboJogStep->setEditable(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogFeed->setEditable(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogStep->setEnabled(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogFeed->setEnabled(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogStep->setStyleSheet(QString("font-size: %1").arg(UiConfigs::instance().ui().fontSize()));
    // ui->cboJogFeed->setStyleSheet(ui->cboJogStep->styleSheet());

    // ui->program->setHeightMapVisible(m_heightmapMode);
    // ui->program->setProgramVisible(!m_heightmapMode);

    ui->program->setSendButtonText(m_heightmapMode ? tr("Probe") : tr("Send"));

    ui->heightmap->updateControlsState(state);

    ui->actFileSaveTransformedAs->setVisible(ui->heightmap->useMap());

    ui->program->setSendMenuFirstActionEnabled(!ui->heightmap->heightmapMode());

    ui->visualizer->setSelectionVisible(!ui->heightmap->heightmapMode());
}

void FrmMain::updateLayouts()
{
    this->update();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void FrmMain::updateRecentFilesMenus()
{
    ui->menuRecent->clear();
    ui->program->setRecentFiles(UiConfigs::instance().ui().recentFiles());

    QStringList files = !m_heightmapMode ? UiConfigs::instance().ui().recentFiles() : UiConfigs::instance().ui().recentHeightmaps();
    if (!files.empty())
    {
        QStringList::const_iterator it = files.constEnd();
        while (it != files.constBegin()) {
            --it;
            QAction *action = new QAction(*it, this);
            connect(action, &QAction::triggered, this, &FrmMain::onActRecentFileTriggered);
            ui->menuRecent->addAction(action);
        }

        ui->menuRecent->addSeparator();

        QAction *clearAction = new QAction(tr("&Clear"), this);
        connect(clearAction, &QAction::triggered, this, [this]() {
            Core::instance().clearRecentFiles(m_heightmapMode);
        });

        ui->menuRecent->addAction(clearAction);
    }

    updateControlsState();
}

void FrmMain::updateJogTitle()
{
    ConfigurationJogging& jogging = m_configuration.joggingModule();

    if (ui->grpJog->isChecked() || !jogging.keyboardControl()) {
        ui->grpJog->setTitle(tr("Jog"));
    } else if (jogging.keyboardControl()) {
        ui->grpJog->setTitle(tr("Jog") + QString(tr(" (%1/%2)"))
                .arg((!jogging.continuous()) ? QString::number(jogging.step()) : tr("C"))
                .arg(jogging.feed()));
    }
}

//TODO heightmap
// void FrmMain::updateHeightmapBorderDrawer()
// {
//     if (m_settingsLoading) return;

//     ui->visualizer->setHeightmapBorderRect(ui->heightmap->borderRectFromTextboxes());
// }

bool FrmMain::updateHeightmapGrid()
{
    QRectF borderRect = ui->heightmap->areaRectFromTextboxes();

    int gridPointsX = heightmap().gridSize().width();
    int gridPointsY = heightmap().gridSize().height();

    // Update interpolation
    ui->visualizer->updateHeightmapInterpolation(true);

    // Generate probe program
    double gridStepX = gridPointsX > 1 ? borderRect.width() / (gridPointsX - 1) : 0;
    double gridStepY = gridPointsY > 1 ? borderRect.height() / (gridPointsY - 1) : 0;

    {
        QSignalBlocker blocker(ui->program);
        ui->program->clearProbeModel();
        ui->program->insertProbeModelRow(0);

        int lastRow = ui->program->probeModelRowCount() - 1;
        ui->program->setProbeModelData(lastRow, 1, QString("G21G90F%1G0Z%2").
                        arg(ConfigurationHeightmap::instance().probeFeed()).arg(heightmap().zBottomTop().top));
        ui->program->setProbeModelData(lastRow, 1, QString("G0X0Y0"));
        ui->program->setProbeModelData(lastRow, 1, QString("G38.2Z%1")
                             .arg(heightmap().zBottomTop().bottom));
        ui->program->setProbeModelData(lastRow, 1, QString("G0Z%1")
                             .arg(heightmap().zBottomTop().top));

        double x, y;

        for (int i = 0; i < gridPointsY; i++) {
            y = borderRect.top() + gridStepY * i;
            for (int j = 0; j < gridPointsX; j++) {
                x = borderRect.left() + gridStepX * (i % 2 ? gridPointsX - 1 - j : j);
                lastRow = ui->program->probeModelRowCount() - 1;
                ui->program->setProbeModelData(lastRow, 1, QString("G0X%1Y%2")
                                     .arg(x, 0, 'f', 3).arg(y, 0, 'f', 3));
                ui->program->setProbeModelData(lastRow, 1, QString("G38.2Z%1")
                                     .arg(heightmap().zBottomTop().bottom));
                ui->program->setProbeModelData(lastRow, 1, QString("G0Z%1")
                                     .arg(heightmap().zBottomTop().top));
            }
        }
    }

    if (m_heightmapMode) {
        updateParser();
    }

    FilesManager::instance().setHeightmapModified(true);

    return true;
}

bool FrmMain::eventFilter(QObject *obj, QEvent *event)
{
    if (ui == nullptr) {
        return QMainWindow::eventFilter(obj, event);
    }

    if (obj->inherits("QWidgetWindow")) {

        // Jog on keyboard control
        QKeySequence ks;
        QKeyEvent *ev = static_cast<QKeyEvent*>(event);

        if ((event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyRelease)) {
            if (ev->key() == Qt::Key_Shift) {
                ks = QKeySequence(Qt::ShiftModifier);
            } else if (ev->key() == Qt::Key_Control) {
                ks = QKeySequence(Qt::ControlModifier);
            } else if (ev->key() == Qt::Key_Alt) {
                ks = QKeySequence(Qt::AltModifier);
            } else {
                ks = QKeySequence(ev->key() | ev->modifiers());
            }
        }

        if (!communicator()->stateBehavior()->is(AbstractStateBehavior::Type::Running)
            && m_configuration.joggingModule().keyboardControl() && !ev->isAutoRepeat())
        {
            static QList<QAction*> acts;
            // if (acts.isEmpty()) acts << ui->actJogXMinus << ui->actJogXPlus
            //                          << ui->actJogYMinus << ui->actJogYPlus
            //                          << ui->actJogZMinus << ui->actJogZPlus;

            // @TODO is this for keyboard jogging??
            // static QList<QAbstractButton*> buttons;
            // if (buttons.isEmpty()) buttons << ui->cmdXMinus << ui->cmdXPlus
            //                                << ui->cmdYMinus << ui->cmdYPlus
            //                                << ui->cmdZMinus << ui->cmdZPlus;

            // for (int i = 0; i < acts.count(); i++) {
            //     if ((!buttons.at(i)->isDown()) && (event->type() == QEvent::ShortcutOverride)) {
            //         if (acts.at(i)->shortcut().matches(ks) == QKeySequence::ExactMatch) {
            //             buttons.at(i)->pressed();
            //             buttons.at(i)->setDown(true);
            //         }
            //     } else if (buttons.at(i)->isDown() && (event->type() == QEvent::KeyRelease)) {
            //         if ((acts.at(i)->shortcut().matches(ks) == QKeySequence::ExactMatch)
            //             || (acts.at(i)->shortcut().toString().contains(ks.toString()))
            //             || (ks.toString().contains(acts.at(i)->shortcut().toString()))
            //             )
            //         {
            //             buttons.at(i)->released();
            //             buttons.at(i)->setDown(false);
            //         }
            //     }
            // }
        }
    }

    // Visualizer updates
    if (obj == this && event->type() == QEvent::WindowStateChange) {
        ui->visualizer->setUpdatesEnabled2(!isMinimized() && ui->dockVisualizer->isVisible());
    }

    // Drag & drop panels
    static QObject *mouseDownObject = nullptr;
    if (event->type() == QEvent::MouseButtonRelease && mouseDownObject != nullptr) {
        mouseDownObject = nullptr;
    }

    if (!ui->actViewLockPanels->isChecked() && obj->parent() != nullptr && obj->inherits("QGroupBox")
        && (obj->parent() == ui->scrollContentsDevice
            || obj->parent() == ui->scrollContentsModification
            || obj->parent() == ui->scrollContentsUser
        )
        && obj->objectName().startsWith("grp")) {

        static QPoint mousePressPos;

        switch (event->type()) {
            case QEvent::MouseButtonPress:
                mouseDownObject = obj;
                break;
            case QEvent::MouseButtonRelease:
                mouseDownObject = nullptr;
                break;
            case QEvent::MouseMove: {
                if (obj != mouseDownObject) break;

                QMouseEvent *e = static_cast<QMouseEvent*>(event);
                int d = (e->pos() - mousePressPos).manhattanLength();

                if (e->buttons() & Qt::LeftButton && d > QApplication::startDragDistance()) {
                    QDrag *drag = new QDrag(this);
                    WidgetMimeData *mimeData = new WidgetMimeData();

                    mimeData->setWidget(static_cast<QWidget*>(obj));

                    QPixmap *pix = new QPixmap(static_cast<QWidget*>(obj)->size());
                    static_cast<QWidget*>(obj)->render(pix);

                    drag->setMimeData(mimeData);
                    drag->setPixmap(*pix);
                    drag->exec();
                }
                break;
            }
            default:
                break;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

// void FrmMain::updateCurrentModel(GCodeTableModel *m_currentModel)
// {
//     this->m_currentModel = m_currentModel;
//     program()->setModel(m_currentModel);
// }

// Updates tool position in visualizer and marks toolpath segments as drawn when tool reaches them
// during active program execution (excluding check mode)
void FrmMain::updateToolPositionAndToolpathShadowing(QVector3D toolPosition)
{
    AbstractStateBehavior *sb = communicator()->stateBehavior();

    // CheckMode has its own behavior type, so it's automatically excluded here
    if (sb->is(AbstractStateBehavior::Type::Running) || sb->is(AbstractStateBehavior::Type::Pause)) {
        int lineIndex = ui->program->currentModelData(ui->program->currentModelIndex(program().processedCommandIndex(), 4)).toInt();
        ui->visualizer->updateToolTracking(toolPosition, lineIndex);
    } else {
        ui->visualizer->setToolPosition(toolPosition);
    }
}

// Updates toolpath visualization in check mode by marking processed segments as drawn
// and positioning the tool indicator at the end of the last processed line
// void FrmMain::updateToolpathShadowingOnCheckMode()
// {
//     GCodeViewParser *parser = m_currentDrawer->viewParser();
//     QList<LineSegment> list = parser->getLineSegmentList();

//     if ((communicator()->m_senderState != SenderState::Stopping) && program().processedCommandIndex() < m_currentModel->rowCount() - 1) {
//         int i;
//         QList<int> drawnLines;

//         for (i = m_lastDrawnLineIndex; i < list.count()
//                                                && list[i].getLineNumber()
//                                                 <= (m_currentModel->data(m_currentModel->index(program().processedCommandIndex(), 4)).toInt()); i++) {
//             drawnLines << i;
//         }

//         if (!drawnLines.isEmpty() && (i < list.count())) {
//             m_lastDrawnLineIndex = i;
//             QVector3D vec = list[i].getEnd();
//             m_toolDrawer.setToolPosition(vec);
//         }

//         foreach (int i, drawnLines) {
//             list[i].setDrawn(true);
//         }
//         if (!drawnLines.isEmpty()) m_currentDrawer->update(drawnLines);
//     } else {
//         for (auto& s : list) {
//             if (!qIsNaN(s.getEnd().length())) {
//                 m_toolDrawer.setToolPosition(s.getEnd());
//                 break;
//             }
//         }
//     }
// }

QString FrmMain::lastUsedDirectory()
{
    return UiConfigs::instance().ui().currentWorkingDirectory();
}

// QList<LineSegment*> FrmMain::subdivideSegment(LineSegment* segment)
// {
//     QList<LineSegment*> list;

//     QRectF borderRect = borderRectFromTextboxes();

//     double interpolationStepX = borderRect.width() / (ui->txtHeightMapInterpolationStepX->value() - 1);
//     double interpolationStepY = borderRect.height() / (ui->txtHeightMapInterpolationStepY->value() - 1);

//     double length;

//     QVector3D vec = segment->getEnd() - segment->getStart();

//     if (qIsNaN(vec.length())) return QList<LineSegment*>();

//     if (fabs(vec.x()) / fabs(vec.y()) < interpolationStepX / interpolationStepY) length = interpolationStepY / (vec.y() / vec.length());
//     else length = interpolationStepX / (vec.x() / vec.length());

//     length = fabs(length);

//     if (qIsNaN(length)) {
//         return QList<LineSegment*>();
//     }

//     QVector3D seg = vec.normalized() * length;
//     // int count = trunc(vec.length() / length);
//     int count = (vec.length() / length);

//     if (count == 0) return QList<LineSegment*>();

//     for (int i = 0; i < count; i++) {
//         LineSegment* line = new LineSegment(segment);
//         line->setStart(i == 0 ? segment->getStart() : list[i - 1]->getEnd());
//         line->setEnd(line->getStart() + seg);
//         list.append(line);
//     }

//     if (list.count() > 0 && list.last()->getEnd() != segment->getEnd()) {
//         LineSegment* line = new LineSegment(segment);
//         line->setStart(list.last()->getEnd());
//         line->setEnd(segment->getEnd());
//         list.append(line);
//     }

//     return list;
// }

void FrmMain::onTransferCompleted()
{
    // Shadow last segment and reset
    ui->visualizer->finalizeTransfer();

    updateControlsState();

    // Show message box
    qApp->beep();
    // communicator()->stopUpdatingState();

    QMessageBox::information(this, qApp->applicationDisplayName(), tr("Job done.\nTime elapsed: %1")
                                .arg(timeEstimator().elapsedTime().toString("hh:mm:ss")));

    // communicator()->startUpdatingState();
}

QString FrmMain::getLineInitCommands(int row)
{
    int commandIndex = row;
    int lineNumber = ui->program->currentModelData(ui->program->currentModelIndex(commandIndex, 4)).toInt();

    auto segmentInfo = ui->visualizer->getSegmentInfoForLine(lineNumber);
    if (!segmentInfo.firstSegment || !segmentInfo.lastSegment) {
        return QString();
    }

    QString commands;
    LineSegment& firstSegment = *segmentInfo.firstSegment;
    LineSegment& lastSegment = *segmentInfo.lastSegment;
    LineSegment& feedSegment = *segmentInfo.feedSegment;
    LineSegment& plungeSegment = *segmentInfo.plungeSegment;

    // commands.append(QString("M3 S%1\n").arg(qMax<double>(lastSegment->getSpindleSpeed(), ui->slbSpindle->value())));

    commands.append(QString("G21 G90 G0 X%1 Y%2\n")
                    .arg(firstSegment.getStart().x())
                    .arg(firstSegment.getStart().y()));
    commands.append(QString("G1 Z%1 F%2\n")
                    .arg(firstSegment.getStart().z())
                    .arg(plungeSegment.getSpeed()));

    commands.append(QString("%1 %2 %3 F%4\n")
                    .arg(lastSegment.isMetric() ? "G21" : "G20")
                    .arg(lastSegment.isAbsolute() ? "G90" : "G91")
                    .arg(lastSegment.isFastTraverse() ? "G0" : "G1")
                    .arg(lastSegment.isMetric() ? feedSegment.getSpeed() : feedSegment.getSpeed() / 25.4));

    if (lastSegment.isArc()) {
        commands.append(lastSegment.plane() == PointSegment::XY ? "G17"
        : lastSegment.plane() == PointSegment::ZX ? "G18" : "G19");
    }

    return commands;
}

bool FrmMain::actionLessThan(const QAction *a1, const QAction *a2)
{
    return a1->objectName() < a2->objectName();
}

bool FrmMain::actionTextLessThan(const QAction *a1, const QAction *a2)
{
    return a1->text() < a2->text();
}

void FrmMain::setHeightmapPoint(QPoint point, double height)
{
    heightmap().setHeightAt(point, height);
    // ui->visualizer->updateHeightmap();

    ui->console->append(QString("[Heightmap] Point (%1, %2) set to %3").arg(point.x()).arg(point.y()).arg(height));
}
