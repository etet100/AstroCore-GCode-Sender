// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include <QFileDialog>
#include <QTextStream>
#include <QDebug>
#include <QStringList>
#include <QTextBlock>
#include <QTextCursor>
#include <QMessageBox>
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
#include "utils/utils.h"
#include "ui/forms/partials/main/partmainjog.h"
#include "ui/forms/partials/main/partmaincontrol.h"
#include "ui/forms/partials/main/partmainvirtualsettings.h"
#include "ui/utils/thememanager.h"
#include "modules/pendant/pendant.h"
#include "modules/camera/camera.h"
#include "ui_frmmain.h"
#include "ui_partmainoverride.h"
#include "ui/widgets/widgetmimedata.h"
#include "ui/widgets/dockabletitle.h"
#include "io/connection/connectionmanager.h"
#include "ui/drawers/vertexdataexporter.h"
#include "core/gcode/loader/gcodethreadedloader.h"
#include "core/heightmap/loader/heightmaploader.h"
#include "core/heightmap/exporter/heightmapexporter.h"
#include "core/utils/filesmanager.h"
#include "state_behaviour/action.h"
#include "state_behaviour/joggingbehavior.h"
#include "state_behaviour/gotobehavior.h"
#include "state_behaviour/reconnectingbehavior.h"

#define FILE_FILTER_TEXT "G-Code files (*.nc *.ncc *.ngc *.tap *.gc *.gcode *.txt)"

FrmMain::FrmMain(Configuration &configuration, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmMain),
#ifdef WINDOWS
    m_taskbarButtonCreatedMessageId(RegisterWindowMessage(L"TaskbarButtonCreated")),
    m_taskBar(this),
#endif
    m_heightmap(),
    m_connectionManager(this, configuration.connectionModule()),
    m_connection(nullptr),
    m_program(),
    m_programModel(m_program),
    m_probeModel(m_program),
    m_programHeightmapModel(m_program),
    m_heightmapModel(m_heightmap),
    m_configuration(configuration)
{
    // Loading settings
    m_settingsFileName = qApp->applicationDirPath() + "/settings.ini";

    // Initializing variables

    m_fileChanged = false;
    m_heightmapChanged = false;
    m_currentModel = &m_programModel;

    // to communicator
    // m_communicator->m_homing = false;
    // m_updateSpindleSpeed = false;
    // m_updateParserStatus = false;

    // to communicator
    // m_reseting = false;
    // m_communicator->m_resetCompleted = true;
    // m_aborting = false;
    // m_statusReceived = false;

    // to communicator
    // m_deviceState = DeviceUnknown;
    // m_communicator->m_senderState = SenderUnknown;

    ui->setupUi(this);

    ui->dockDevice->setTitleBarWidget(new DockableTitle(ui->dockDevice));
    ui->dockConsole->setTitleBarWidget(new DockableTitle(ui->dockConsole));
    ui->dockVisualizer->setTitleBarWidget(new DockableTitle(ui->dockVisualizer));
    ui->dockUser->setTitleBarWidget(new DockableTitle(ui->dockUser));
    ui->dockProgram->setTitleBarWidget(new DockableTitle(ui->dockProgram));
    ui->dockModification->setTitleBarWidget(new DockableTitle(ui->dockModification));

    initializeFontSizeMenu();
    preloadSettings();
    Utils::setVisualMode(this, m_configuration.uiModule().darkTheme());
    initializeCommunicator();

    ui->jog->initialize(m_configuration.joggingModule());

    ui->console->initialize(m_configuration.consoleModule());
    connect(ui->console, &PartMainConsole::newCommand, this, &FrmMain::onConsoleNewCommand);
    ui->console->append(QString("G-Pilot %1 started").arg( qApp->applicationVersion()));
    ui->console->append("---");

    connect(&m_program, &GCode::linesUpdated, this, [this](int fromLine, int toLine) {
        Q_UNUSED(fromLine);
        Q_UNUSED(toLine);

        if (!ui->program->isAutoScroll()) {
            return;
        }

        int tableIndex = m_currentModel->toFilteredIndex(m_program.commandIndex());
        ui->program->scrollToCurrentIndex(m_currentModel->index(tableIndex, 1));

        GCodeViewParser *parser = &m_viewParser;
        QVector<QList<int>> lineIndexes = parser->getLinesIndexes();
        QList<LineSegment>& list = parser->getLineSegmentList();
        QList<int> indexes;

        for (int i = fromLine; i <= toLine; i++) {
            GCodeItem &item = m_program[i];
            int j = item.lineNumber;
            if (j != -1)
            foreach (int l, lineIndexes.at(j)) {
                if (item.state == GCodeItem::Sent) {
                    list[l].setIsHightlight(true);
                    list[l].setDrawn(false);
                    indexes.append(l);
                } else if (item.state == GCodeItem::Processed) {
                    list[l].setIsHightlight(false);
                    list[l].setDrawn(true);
                    indexes.append(l);
                }
            }
        }

        if (!indexes.isEmpty()) {
            ui->visualizer->updateCodeDrawer(indexes);
        }
    });

    connect(ui->program, &PartMainProgram::hideCommentsChanged, this, [this](bool checked) {
        m_programModel.setCommentsVisible(!checked);
    });

    connect(ui->control, &PartMainControl::unlock, this, [this]() {
        // m_communicator->m_updateSpindleSpeed = true;
        // m_communicator->sendCommand(CommandSource::GeneralUI, "$X", TABLE_INDEX_UI);
        m_communicator->unlock();
    });
    connect(ui->control, &PartMainControl::home, this, [this]() {
        // m_communicator->m_homing = true;
        // m_communicator->m_updateSpindleSpeed = true;
        // m_communicator->sendCommand(CommandSource::GeneralUI, "$H", TABLE_INDEX_UI);

        m_communicator->home();
    });
    connect(ui->control, &PartMainControl::probe, this, [this]() {
        m_communicator->probe();
    });
    connect(ui->control, &PartMainControl::reset, this, [this]() {
        m_communicator->reset();
    });
    // connect(ui->control, &partMainControl::command, this, [=](GRBLCommand command) {
    //     qDebug() << "Command: " << command;
    // });

    // toggle section visibility
    connect(ui->grpControl, &QGroupBox::toggled, this, [this](bool checked) {
        updateLayouts();
        ui->control->setVisible(checked);
    });
    connect(ui->grpState, &QGroupBox::toggled, this, [this](bool checked) {
        updateLayouts();
        ui->state->setVisible(checked);
    });
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

    connect(ui->jog, &PartMainJog::jog, this, [this](JoggindDir dir, QVector3D jog) {
        //m_communicator->jogger().jog(dir);

        m_configuration.save();

        if (dir != JoggindDir::None) {
            JoggingBehavior *joggingBehavior = new JoggingBehavior(
                dir,
                m_configuration.joggingModule().step(),
                m_configuration.joggingModule().feed(),
                m_configuration.joggingModule().finalFeedZ()
            );
            m_communicator->execute(joggingBehavior);
        }

        // Q_UNUSED(dir)
        // qDebug() << "Jog: " << jog;
        // jogStep(jog);
    });
    connect(ui->jog, &PartMainJog::stop, this, [this]() {
        JoggingBehavior *joggingBehavior = dynamic_cast<JoggingBehavior*>(m_communicator->stateBehavior());
        if (joggingBehavior) {
            joggingBehavior->stopJogging();
        }

        // m_communicator->clearQueue();
        // m_communicator->sendRealtimeCommand(GRBL_LIVE_JOG_CANCEL);
        // while (m_communicator->deviceState() == DeviceState::Jog) {
        //     qApp->processEvents();
        // }
    });

    // Drag&drop placeholders
    ui->fraDropDevice->setVisible(false);
    ui->fraDropModification->setVisible(false);
    ui->fraDropUser->setVisible(false);

    //
    FilesManager& fm = FilesManager::instance();
    connect(&fm, &FilesManager::gcodeFileStateChanged, this, [this, &fm](bool opened, const QString& filePath) {
        Q_UNUSED(filePath);
        this->setWindowTitle(!opened ? qApp->applicationDisplayName() : fm.gcodeFileName() + " - " + qApp->applicationDisplayName());
    });

#ifdef WINDOWS
    // m_taskBar.setMin(0);
    // m_taskBar.setMax(100);
    // m_taskBar.setValue(50);
    // m_taskBar.setVisible(true);

    // if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7) {
    //     m_taskBarButton = NULL;
    //     m_taskBarProgress = NULL;
    // }
#endif

//    ui->scrollArea->updateMinimumWidth();

    m_heightmapMode = false;
    m_program.resetProcessed();
    m_programLoading = false;
    //updateCurrentModel(&m_programModel);

    // Dock widgets
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    connect(ui->heightmap, &PartMainHeightmap::extremesRequired, this, [this]() {
        ui->heightmap->setHeightmapBorderRect(ui->visualizer->getCodeDrawerBounds());
    });
    connect(ui->heightmap, &PartMainHeightmap::newHeightmapRequested, this, &FrmMain::on_actFileNew_triggered);
    connect(ui->heightmap, &PartMainHeightmap::loadHeightmapRequested, this, &FrmMain::onLoadHeightmapRequested);
    connect(ui->heightmap, &PartMainHeightmap::useHeightmapToggled, this, &FrmMain::useHeightmapToggled);
    connect(ui->heightmap, &PartMainHeightmap::heightmapModeToggled, this, &FrmMain::heightmapModeToggled);
    connect(ui->heightmap, &PartMainHeightmap::showVisualizationChanged, this, [this](PartMainHeightmap::VisualizationDrawers drawers) {
        ui->visualizer->showHeightmapBorder(drawers.border);
        ui->visualizer->showHeightmapProbeGrid(drawers.grid);
        ui->visualizer->showHeightmapInterpolationGrid(drawers.interpolation);
    });

    // ui->cmdToggleProjection->setParent(ui->glwVisualizer);
    // ui->cmdFit->setParent(ui->glwVisualizer);
    // ui->cmdIsometric->setParent(ui->glwVisualizer);
    // ui->cmdTop->setParent(ui->glwVisualizer);
    // ui->cmdFront->setParent(ui->glwVisualizer);
    // ui->cmdLeft->setParent(ui->glwVisualizer);
    // ui->cmdRotationCube->setParent(ui->glwVisualizer);

    // ui->cmdHeightMapBorderAuto->setMinimumHeight(ui->chkHeightMapBorderShow->sizeHint().height());
    // ui->cmdHeightMapCreate->setMinimumHeight(ui->cmdFileOpen->sizeHint().height());
    // ui->cmdHeightMapLoad->setMinimumHeight(ui->cmdFileOpen->sizeHint().height());
    // ui->cmdHeightMapMode->setMinimumHeight(ui->cmdFileOpen->sizeHint().height());

    // ui->cboJogStep->setValidator(new QDoubleValidator(0, 10000, 2));
    // ui->cboJogFeed->setValidator(new QIntValidator(0, 100000));
    // connect(ui->cboJogStep, &ComboBoxKey::currentTextChanged, this, &FrmMain::updateJogTitle);
    // connect(ui->cboJogFeed, &ComboBoxKey::currentTextChanged, this, &FrmMain::updateJogTitle);

    // Prepare Open and Send menus
    ui->program->setupFileOpenMenu(this, SLOT(onFileOpen()), SLOT(on_cmdHeightMapLoad_clicked()));
    ui->program->setupFileSendMenu(this, SLOT(onActSendFromLineTriggered()));

    foreach (StyledToolButton* button, this->findChildren<StyledToolButton*>(QRegularExpression("cmdUser\\d"))) {
        connect(button, SIGNAL(clicked(bool)), this, SLOT(onCmdUserClicked(bool)));
    }

    // ui->visualizer = new PartMainVisualizer(this);
    // m_program, m_heightmap
    ui->visualizer->setCodeParser(&m_viewParser);
    ui->visualizer->setProbeParser(&m_probeParser);
    ui->visualizer->initDrawables();

    m_tableMenu = new QMenu(this);
    m_tableMenu->addAction(tr("&Insert line"), this, SLOT(onTableInsertLine()), QKeySequence(Qt::Key_Insert));
    m_tableMenu->addAction(tr("&Delete lines"), this, SLOT(onTableDeleteLines()), QKeySequence(Qt::Key_Delete));

    initializeVisualizer();

    connect(ui->visualizer, &PartMainVisualizer::goToCursor, this, [this](QPointF pos) {
        m_communicator->execute(new GoToBehavior(pos, m_configuration.joggingModule().feed()));
    });
    connect(&m_programModel, &QAbstractItemModel::dataChanged, this, &FrmMain::onTableCellChanged);
    connect(&m_programHeightmapModel, &QAbstractItemModel::dataChanged, this, &FrmMain::onTableCellChanged);
    connect(&m_probeModel, &QAbstractItemModel::dataChanged, this, &FrmMain::onTableCellChanged);
    connect(&m_heightmapModel, SIGNAL(dataChangedByUserInput()), this, SLOT(updateHeightMapInterpolationDrawer()));
    // connect(&m_program, &GCode::linesUpdated, this, &FrmMain::onProgramLinesUpdated);

    ui->program->setProgramModel(&m_programModel);
    ui->program->setProgramItemDelegate(&m_programItemDelegate);
    connect(ui->program, &PartMainProgram::manualScrollRequested, this, [this]() {
        if ((m_communicator->senderState() == SenderState::Transferring) || (m_communicator->senderState() == SenderState::Stopping))
            ui->program->setAutoScroll(false);
    });
    connect(ui->program, &PartMainProgram::currentChanged, this, &FrmMain::onTableCurrentChanged);
    clearTable();

    connect(ui->program, &PartMainProgram::open, this, &FrmMain::onFileOpen);
    connect(ui->program, &PartMainProgram::start, this, &FrmMain::onFileSend);
    connect(ui->program, &PartMainProgram::pause, this, &FrmMain::onFilePause);
    connect(ui->program, &PartMainProgram::abort, this, &FrmMain::onFileAbort);
    connect(ui->program, &PartMainProgram::reset, this, &FrmMain::onFileReset);
    connect(ui->program, &PartMainProgram::customContextMenuRequested, this, &FrmMain::onProgramTableContextMenuRequested);

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

    // Signals/slots
    connect(&m_timerConnection, SIGNAL(timeout()), this, SLOT(onTimerConnection()));

    // Event filter
    qApp->installEventFilter(this);

    // Start timers
    m_timerConnection.start(1000);
    m_timerToolAnimation.start(25, this);

    // Pendant
    Pendant *pendant = new Pendant(this, *m_communicator);

    // Virtual uCNC settings
    m_partMainVirtualSettings = new PartMainVirtualSettings();
    m_partMainVirtualSettings->setEnabled(false);
    appendPanel(
        ui->scrollContentsDevice,
        "VirtualSettings",
        "Virtual uCNC settings",
        m_partMainVirtualSettings
    );

    appendSpacer(
        ui->scrollContentsDevice
    );

    updateLayouts();

    // Initialize central widget management
    initializeCentralWidgets();

    // Camera
    addDockableWindow(
        "Camera",
        new Camera(this),
        Qt::TopDockWidgetArea,
        Qt::Horizontal
    );

    // After everything is set up, restore layout
    restoreDockableLayoutState();
}

FrmMain::~FrmMain()
{
    delete m_communicator;
    delete m_connection;
    delete m_senderErrorBox;
    delete ui; ui = nullptr;
}

void FrmMain::initializeCommunicator()
{
    m_communicator = new Communicator(
        this,
        nullptr,
        &m_configuration
    );
    //m_program = new GCode();
    // @TODO temporary!
    // m_communicator->streamCommands(m_program);

    connect(m_communicator, &Communicator::machinePosChanged, this, &FrmMain::onMachinePosChanged);
    connect(m_communicator, &Communicator::workPosChanged, this, &FrmMain::onWorkPosChanged);
    connect(m_communicator, &Communicator::machineStateReceived, this, &FrmMain::onMachineStateReceived);
    connect(m_communicator, &Communicator::machineStateChanged, this, &FrmMain::onMachineStateChanged);
    connect(m_communicator, &Communicator::senderStateReceived, this, &FrmMain::onSenderStateReceived);
    connect(m_communicator, SIGNAL(spindleStateReceived(bool)), this, SLOT(onSpindleStateReceived(bool)));
    connect(m_communicator, &Communicator::floodStateReceived, this, &FrmMain::onFloodStateReceived);
    connect(m_communicator, &Communicator::commandSent, this, &FrmMain::onCommandSent);
    connect(m_communicator, &Communicator::commandResponseReceived, this, &FrmMain::onCommandResponseReceived);
    connect(m_communicator, &Communicator::parserStateReceived, this, &FrmMain::onParserStateReceived);
    connect(m_communicator, &Communicator::welcomeMessageReceived, this, [this](QString message) {
        ui->console->appendSystem(message);
    });
    connect(m_communicator, &Communicator::log, this, [this](QString message) {
        ui->console->append(message);
    });
    connect(m_communicator, &Communicator::pinStateReceived, this, &FrmMain::onPinStateReceived);
    connect(m_communicator, &Communicator::spindleSpeedReceived, this, &FrmMain::onSpindleSpeedReceived);
    // connect(m_communicator, &Communicator::commandProcessed, this, &FrmMain::onCommandProcessed);
    connect(m_communicator, SIGNAL(feedSpindleSpeedReceived(int,int)), this, SLOT(onFeedSpindleSpeedReceived(int,int)));
    connect(m_communicator, &Communicator::overridesReceived, this, &FrmMain::onOverridesReceived);
    connect(m_communicator, &Communicator::toolPositionReceived, this, &FrmMain::onToolPositionReceived);
    connect(m_communicator, &Communicator::transferCompleted, this, &FrmMain::onTransferCompleted);
    connect(m_communicator, &Communicator::aborted, this, &FrmMain::onAborted);
    connect(m_communicator, &Communicator::machineConfigurationReceived, this, [this](PhysicalMachineConfiguration configuration) {
        m_partMainVirtualSettings->deviceConfigurationReceived(configuration);
    });
    // connect(m_communicator, &Communicator::statusReceived, this, [this]() {
    //     jogContinuous();
    // });
    connect(m_communicator, &Communicator::stateBehaviorChanged, this, &FrmMain::onStateBehaviorChanged);
    connect(m_communicator, &Communicator::connectionChanged, this, [this](Connection *connection) {
        ui->state->setConName(connection->name());
    });
}

void FrmMain::initializeVisualizer()
{
    ui->visualizer->fitCodeDrawer();
    connect(ui->visualizer, &PartMainVisualizer::viewModeChanged, this, [this](GLWidget::ViewMode mode) {
        switch (mode) {
            case GLWidget::ViewMode::Perspective:
                m_configuration.visualizerModule().setViewMode(ConfigurationVisualizer::ViewMode::Perspective);
                break;
            case GLWidget::ViewMode::Orthogonal:
                m_configuration.visualizerModule().setViewMode(ConfigurationVisualizer::ViewMode::Orthogonal);
                break;
            case GLWidget::ViewMode::View2D:
                m_configuration.visualizerModule().setViewMode(ConfigurationVisualizer::ViewMode::View2D);
                break;
        }
    });
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
        Utils::positionDialog(this, m_configuration.uiModule().mainFormGeometry(), m_configuration.uiModule().mainFormMaximized());
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

    resizeTableHeightmapSections();

    if (!m_firstShow) {
        m_configuration.uiModule().setMainFormGeometry(this);
    }
}

void FrmMain::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_timerToolAnimation.timerId()) {
        // ui->visualizer->toolDrawer()->rotate((m_communicator->m_spindleCW ? -40 : 40) * (double)(ui->slbSpindle->currentValue())
        //                     / (ui->slbSpindle->maximum()));
        // ui->visualizer->cursorDrawer()->rotate();
    } else {
        QMainWindow::timerEvent(te);
    }
}

void FrmMain::closeEvent(QCloseEvent *ce)
{
    bool mode = m_heightmapMode;
    m_heightmapMode = false;

    if (!saveChanges(m_heightmapMode)) {
        ce->ignore();
        m_heightmapMode = mode;
        return;
    }

    if ((m_communicator->senderState() != SenderState::Stopped) &&
        QMessageBox::warning(this, this->windowTitle(), tr("File sending in progress. Terminate and exit?"),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    {
        ce->ignore();
        m_heightmapMode = mode;
        return;
    }

    m_timerConnection.stop();
    m_communicator->deinit();
    m_connection->close();

    saveSettings();
}

void FrmMain::dragEnterEvent(QDragEnterEvent *dee)
{
    m_fileDropOverlay = new FileDropOverlay(this);
    m_fileDropOverlay->setGeometry(0, 0, width(), height());
    m_fileDropOverlay->show();

    if (m_communicator->senderState() != SenderState::Stopped || dee->mimeData()->hasFormat("application/widget")) {
        m_fileDropOverlay->showForbidden();

        return;
    }

    if (dee->mimeData()->hasFormat("text/plain") && !m_heightmapMode) {
        dee->acceptProposedAction();
        m_fileDropOverlay->showValid();

        return;
    } else if (dee->mimeData()->hasFormat("text/uri-list") && dee->mimeData()->urls().count() == 1) {
        QString fileName = dee->mimeData()->urls().at(0).toLocalFile();

        if ((!m_heightmapMode && Utils::isGCodeFile(fileName)) || (m_heightmapMode && Utils::isHeightmapFile(fileName))) {
            dee->acceptProposedAction();
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
        delete m_fileDropOverlay;
        m_fileDropOverlay = nullptr;
    }

    QString fileName = de->mimeData()->urls().at(0).toLocalFile();

    if (!m_heightmapMode) {
        if (!saveChanges(false)) return;

        // Load dropped g-code file
        if (!fileName.isEmpty()) {
            addRecentFile(fileName);
            updateRecentFilesMenu();
            loadFile(fileName);
        // Load dropped text
        } else {
            FilesManager::instance().resetGcodeFile();
            m_fileChanged = true;
            //@todo fix after refactoring loadFile to be faster
            //loadFile(de->mimeData()->text().split("\n"));
        }
    } else {
        if (!saveChanges(true)) return;

        // Load dropped heightmap file
        addRecentHeightmap(fileName);
        updateRecentFilesMenu();
        // loadHeightmap(fileName);
    }
}

void FrmMain::changeEvent(QEvent *ce)
{
    QMainWindow::changeEvent(ce);
    if (ce->type() == QEvent::WindowStateChange) {
        m_configuration.uiModule().setMainFormGeometry(this);
    }
}

void FrmMain::moveEvent(QMoveEvent *me)
{
    QMainWindow::moveEvent(me);
    if (!m_firstShow) {
        m_configuration.uiModule().setMainFormGeometry(this);
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

void FrmMain::on_actFileNew_triggered()
{
    if (!saveChanges(m_heightmapMode)) return;

    if (!m_heightmapMode) {
        newFile();
    } else {
        newHeightmap();
    }
}

void FrmMain::on_actFileOpen_triggered()
{
    onFileOpen();
}

void FrmMain::on_actFileSave_triggered()
{
    FilesManager& fm = FilesManager::instance();
    if (!m_heightmapMode) {
        // G-code saving
        if (fm.gcodeOpened()) on_actFileSaveAs_triggered(); else {
            saveProgramToFile(fm.gcodeFilePath(), m_program);
            m_fileChanged = false;
        }
    } else {
        // Height map saving
        if (fm.heightmapOpened()) on_actFileSaveAs_triggered(); else {
            saveHeightmap(fm.heightmapFilePath());
        }
    }
}

void FrmMain::on_actFileSaveAs_triggered()
{
    FilesManager& fm = FilesManager::instance();

    if (!m_heightmapMode) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save file as"), "", tr(FILE_FILTER_TEXT));

        if (!fileName.isEmpty()) if (saveProgramToFile(fileName, m_program)) {
            fm.setGcodeFilePath(fileName);
            m_fileChanged = false;

            addRecentFile(fileName);
            updateRecentFilesMenu();

            updateControlsState();
        }
    } else {
        QString fileName = (QFileDialog::getSaveFileName(this, tr("Save file as"), lastWorkingDirectory(), tr("Heightmap files (*.map)")));

        if (!fileName.isEmpty()) if (saveHeightmap(fileName)) {
            fm.setHeightmapFilePath(fileName);
            m_heightmapChanged = false;

            ui->heightmap->setOpenFile(fileName.mid(fileName.lastIndexOf("/") + 1));

            addRecentHeightmap(fileName);
            updateRecentFilesMenu();

            updateControlsState();
        }
    }
}

void FrmMain::on_actFileSaveTransformedAs_triggered()
{
    QString fileName = (QFileDialog::getSaveFileName(this, tr("Save file as"), lastWorkingDirectory(), tr(FILE_FILTER_TEXT)));

    if (!fileName.isEmpty()) {
//        saveProgramToFile(fileName, &m_programHeightmapModel);
    }
}

void FrmMain::on_actHeightmapOpen2_triggered()
{
    QString fileName = (QFileDialog::getOpenFileName(this, tr("Open heightmap"), lastWorkingDirectory(), tr("Heightmap files (*.map)")));
    if (fileName.isEmpty()) {
        return;
    }

    try {
        m_heightmap = HeightmapLoader::loadFromFile(fileName);
    } catch (std::runtime_error &err) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load heightmap: %1").arg(err.what()));
        return;
    }

    ui->console->append(tr("Heightmap %1x%2loaded from %3").arg(m_heightmap.gridWidth()).arg(m_heightmap.gridHeight())
                                  .arg(fileName));

    ui->visualizer->setHeightmap(m_heightmap);
}

void FrmMain::on_actHeightmapSave_triggered()
{
    QString fileName = (QFileDialog::getSaveFileName(this, tr("Save heightmap as"), lastWorkingDirectory(), tr("Heightmap files (*.map)")));
    if (fileName.isEmpty()) {
        return;
    }

    try {
        HeightmapExporter::exportToFile(m_heightmap, fileName);
    } catch (std::runtime_error &err) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save heightmap: %1").arg(err.what()));
        return;
    }

    ui->console->append(tr("Heightmap saved to %1").arg(fileName));
}

void FrmMain::onActRecentClearTriggered()
{
    if (!m_heightmapMode) m_configuration.uiModule().clearRecentFiles();
        else m_configuration.uiModule().clearRecentHeightmaps();
    m_configuration.save();
    updateRecentFilesMenu();
}

void FrmMain::on_actFileExit_triggered()
{
    close();
}

void FrmMain::on_actServiceSettings_triggered()
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
    Utils::setVisualMode(form.data(), m_configuration.uiModule().darkTheme());
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

void FrmMain::on_actServiceConfigureGRBL_triggered()
{
    FrmGrblConfigurator *form = new FrmGrblConfigurator(this, m_configuration.uiModule(), m_communicator);
    form->exec();
    form->deleteLater();
}

void FrmMain::on_actAbout_triggered()
{
    FrmAbout *form = new FrmAbout(this);
    form->exec();
    form->deleteLater();
}

void FrmMain::on_actSpindleSpeedPlus_triggered()
{
    // ui->slbSpindle->setSliderPosition(ui->slbSpindle->sliderPosition() + 1);
}

void FrmMain::on_actSpindleSpeedMinus_triggered()
{
    // ui->slbSpindle->setSliderPosition(ui->slbSpindle->sliderPosition() - 1);
}

void FrmMain::on_actViewLockWindows_toggled(bool checked)
{
    QList<QDockWidget*> dl = findChildren<QDockWidget*>();

    foreach (QDockWidget *dock, dl) {
        Utils::setDockableLocked(dock, checked);
    }

    m_configuration.uiModule().setLockWindows(checked);
}

void FrmMain::on_actViewDarkMode_toggled(bool checked)
{
    m_configuration.uiModule().setDarkMode(checked);
    ThemeManager::instance().setDark(checked);
}

void FrmMain::on_actViewCentralProgram_toggled(bool checked)
{
    Q_UNUSED(checked);
    switchCentralWidget(ui->actViewCentralProgram);
}

// Visualiser in central widget, program docked, hide empty visualizer dock
void FrmMain::on_actViewCentralVisualizer_toggled(bool checked)
{
    Q_UNUSED(checked);
    switchCentralWidget(ui->actViewCentralVisualizer);
}

void FrmMain::onFileOpen()
{
    if (!m_communicator->isMachineConfigurationReady()) {
        qWarning() << "[UI] Machine configuration is not ready";

        return;
    }

    if (!m_heightmapMode) {
        if (!saveChanges(false)) return;

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), "",
                                   tr(FILE_FILTER_TEXT";;All files (*.*)"));
        if (fileName.isEmpty()) {
            return;
        }

        m_configuration.uiModule().currentWorkingDirectory(fileName.left(fileName.lastIndexOf(QRegularExpression("[/\\\\]+"))));

        addRecentFile(fileName);
        updateRecentFilesMenu();

        loadFile(fileName);
    } else {
        if (!saveChanges(true)) return;

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), lastWorkingDirectory(), tr("Heightmap files (*.map)"));
        if (fileName.isEmpty()) {
            return;
        }

        addRecentHeightmap(fileName);
        updateRecentFilesMenu();
        loadHeightmap(fileName);
    }
}

void FrmMain::onFileSend()
{
    m_program.reset();
    m_communicator->sb()->action(RunAction(m_program));

//     if (m_currentModel->rowCount() == 1) return;

//     onFileReset();

//     m_startTime = QDateTime::currentSecsSinceEpoch();

//     m_communicator->setSenderStateAndEmitSignal(SenderState::Transferring);

//     ui->jog->storeAndResetKeyboardControl();
//     // m_storedKeyboardControl = ui->chkKeyboardControl->isChecked();
//     // ui->chkKeyboardControl->setChecked(false);

//     m_communicator->storeParserState();

// #ifdef WINDOWS
//     // if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7) {
//     //     if (m_taskBarProgress) {
//     //         m_taskBarProgress->setMaximum(m_currentModel->rowCount() - 2);
//     //         m_taskBarProgress->setValue(0);
//     //         m_taskBarProgress->show();
//     //     }
//     // }
// #endif

//     updateControlsState();
//     ui->cmdFilePause->setFocus();

//     if (m_configuration.senderModule().useProgramStartCommands())
//         m_communicator->sendCommands(CommandSource::ProgramAdditionalCommands, m_configuration.senderModule().programStartCommands());

//     // rather temporary solution
//     // m_program->setModel(&m_programModel);
//     m_communicator->sendStreamerCommandsUntilBufferIsFull();
}

void FrmMain::onFilePause(bool checked)
{
    static SenderState s;

    // if (checked) {
    //     //PAUSE
    //     s = m_communicator->senderState();
    //     // setSenderState(SenderPaused);
    //     m_communicator->setSenderStateAndEmitSignal(SenderState::Pausing);
    //     ui->cmdFilePause->setText(tr("Pausing..."));
    //     ui->cmdFilePause->setEnabled(false);
    // } else {
    //     //RESUME
    //     if (m_communicator->senderState() == SenderState::ChangingTool) {
    //         m_communicator->setSenderStateAndEmitSignal(SenderState::Transferring);
    //     } else {
    //         if (m_configuration.senderModule().usePauseCommands()) {
    //             m_communicator->sendCommands(CommandSource::ProgramAdditionalCommands, m_configuration.senderModule().afterPauseCommands());
    //         }
    //         m_communicator->setSenderStateAndEmitSignal(s);
    //     }
    //     updateControlsState();
    // }

    if (checked) {
        Action action(Action::Pause);
        if (m_communicator->stateBehavior()->action(action)) {
            ui->program->setPauseButtonText(tr("Resume"));
        }
    } else {
        Action action(Action::Resume);
        if (m_communicator->stateBehavior()->action(action)) {
            ui->program->setPauseButtonText(tr("Pause"));
        }
    }
}



void FrmMain::onFileAbort()
{
    ui->program->setAbortButtonEnabled(false);
    m_communicator->abort();
}

void FrmMain::onFileReset()
{
    m_program.reset();
    ui->visualizer->resetLastDrawnLine();
    // m_communicator->m_probeIndex = -1;

    if (!m_heightmapMode) {
        QList<LineSegment>& list = m_viewParser.getLineSegmentList();

        QList<int> indexes;
        for (int i = 0; i < list.count(); i++) {
            list[i].setDrawn(false);
            indexes.append(i);
        }
        ui->visualizer->updateCodeDrawer(indexes);

        ui->program->setTableUpdatesEnabled(false);

        // It should be done in `m_program.reset()`
        // for (int i = 0; i < m_currentProgram->count() - 1; i++) {
        //     (*m_currentProgram)[i].state = GCodeItem::InQueue;
        //     (*m_currentProgram)[i].response = QString();
        // }
        ui->program->setTableUpdatesEnabled(true);

        ui->program->resetToFirstRow();

        ui->visualizer->setSpendTime(QTime(0, 0, 0));
    } else {
        ui->heightmap->setGridUpdateEnabled();

        // delete m_heightmapInterpolationDrawer.data();
        ui->visualizer->updateHeightmapInterpolation(true);

        m_heightmapModel.clear();
        updateHeightmapGrid();
    }
}

// void FrmMain::on_cmdCommandSend_clicked()
// {
//     QString command = ui->cboCommand->currentText();
//     if (command.isEmpty()) return;

//     ui->cboCommand->storeText();
//     ui->cboCommand->setCurrentText("");
//     m_communicator->sendCommand(command, COMMAND_TI_UI);
// }

// void FrmMain::on_cmdClearConsole_clicked()
// {
//     ui->txtConsole->clear();
// }

// void FrmMain::on_cmdHome_clicked()
// {
//     m_communicator->m_homing = true;
//     m_communicator->m_updateSpindleSpeed = true;
//     m_communicator->sendCommand(CommandSource::GeneralUI, "$H", COMMAND_TI_UI);
// }

// void FrmMain::on_cmdCheck_clicked(bool checked)
// {
//     if (checked) {
//         m_communicator->storeParserState();
//         m_communicator->sendCommand(CommandSource::GeneralUI, "$C", COMMAND_TI_UI);
//     } else {
//         m_communicator->m_aborting = true;
//         m_communicator->reset();
//     };
// }

// void FrmMain::on_cmdReset_clicked()
// {
//     m_communicator->reset();
//     //grblReset();
// }

// void FrmMain::on_cmdUnlock_clicked()
// {
//     m_communicator->m_updateSpindleSpeed = true;
//     m_communicator->sendCommand(CommandSource::GeneralUI, "$X", COMMAND_TI_UI);
// }

// void FrmMain::on_cmdHold_clicked(bool checked)
// {
//     m_connection->sendByteArray(QByteArray(1, checked ? (char)'!' : (char)'~'));
// }

// void FrmMain::on_cmdSleep_clicked()
// {
//     m_communicator->sendCommand(CommandSource::GeneralUI, "$SLP", COMMAND_TI_UI);
// }

// void FrmMain::on_cmdDoor_clicked()
// {
//     m_connection->sendByteArray(QByteArray(1, (char)0x84));
// }

// void FrmMain::on_cmdFlood_clicked()
// {
//     m_connection->sendByteArray(QByteArray(1, (char)0xa0));
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

void FrmMain::on_cmdSpindle_clicked(bool checked)
{
    if (ui->control->hold()) {
        m_connection->sendByteArray(QByteArray(1, char(0x9e)));
    } else {
        // m_communicator->sendCommand(CommandSource::GeneralUI, checked ? QString("M3 S%1").arg(ui->slbSpindle->value()) : "M5", TABLE_INDEX_UI);
    }
}

void FrmMain::on_grpOverriding_toggled(bool checked)
{
    PartMainOverride::Overrides overrides = ui->overrides->overrides();

    if (checked) {
        ui->grpOverriding->setTitle(tr("Overriding"));
    } else if (overrides.feedOverridden | overrides.rapidOverridden | overrides.spindleOverridden) {
        ui->grpOverriding->setTitle(tr("Overriding") + QString(tr(" (%1/%2/%3)"))
                                    .arg(overrides.feedOverridden ? QString::number(overrides.feed) : "-")
                                    .arg(overrides.rapidOverridden ? QString::number(overrides.rapid) : "-")
                                    .arg(overrides.spindleOverridden ? QString::number(overrides.spindleOverridden) : "-"));
    }
    updateLayouts();

    ui->overrides->setVisible(checked);
}

void FrmMain::on_grpSpindle_toggled(bool checked)
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

void FrmMain::on_grpJog_toggled(bool checked)
{
    updateJogTitle();
    updateLayouts();

    ui->jog->setVisible(checked);
}

void FrmMain::on_grpHeightmap_toggled(bool checked)
{
    ui->heightmap->setVisible(checked);
}

void FrmMain::on_chkKeyboardControl_toggled(bool checked)
{
    ui->grpJog->setProperty("overrided", checked);
    style()->unpolish(ui->grpJog);
    ui->grpJog->ensurePolished();

    // Store/restore coordinate system
    if (checked) {
        //m_communicator->sendCommand(CommandSource::System, "$G", COMMAND_TI_UTIL1);
    } else {
        if (m_absoluteCoordinates) m_communicator->sendCommand(CommandSource::System, "G90", TABLE_INDEX_UI);
    }

    if ((m_communicator->senderState() != SenderState::Transferring) && (m_communicator->senderState() != SenderState::Stopping))
        ui->jog->setKeyboardControl(checked);

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
//             QList<LineSegment*> *list = m_viewParser.getLines();
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

//                 if (progress.isVisible() && (i % PROGRESSSTEP == 0)) {
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

//                 if (progress.isVisible() && (i % PROGRESSSTEP == 0)) {
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
//                 command = m_program[i].command;
//                 lineNumber = m_program[i].lineNumber;
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

//                 if (progress.isVisible() && (i % PROGRESSSTEP == 0)) {
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
    m_program.reset();
    ui->visualizer->resetLastDrawnLine();

    // Reset/restore g-code program modification on edit mode enter/exit
    if (ui->heightmap->useMap()) {
        useHeightmapToggled(!checked); // Update gcode program parser
    }

    if (checked) {
        ui->program->setProgramModel(&m_probeModel);
        resizeTableHeightmapSections();
        //updateCurrentModel(&m_programModel);
        ui->visualizer->useProbeDrawer();
        updateParser();  // Update probe program parser
    } else {
        m_probeParser.reset();
        if (!ui->heightmap->useMap()) {
            ui->program->setProgramModel(&m_programModel);
            // connect(ui->tblProgram->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onTableCurrentChanged(QModelIndex,QModelIndex)));
            ui->program->selectFirstRow();

            resizeTableHeightmapSections();
            // updateCurrentModel(&m_programModel);
            ui->visualizer->useCodeDrawer();

            if (!ui->heightmap->useMap()) {
                ui->visualizer->updateGCodeExtremes();
                // ui->glwVisualizer->updateExtremes(m_codeDrawer);
//                updateProgramEstimatedTime(m_currentDrawer->viewParser()->getLineSegmentList());
            }
        }
    }

    // Shadow toolpath
    QList<LineSegment>& list = m_viewParser.getLineSegmentList();
    QList<int> indexes;
    for (int i = 0; i < list.count(); i++) {
        list[i].setDrawn(checked);
        list[i].setIsHightlight(false);
        indexes.append(i);
    }
    // Update only vertex color.
    // If chkHeightMapUse was checked codeDrawer updated via updateParser
    if (!ui->heightmap->useMap()) ui->visualizer->updateCodeDrawer(indexes);

    updateRecentFilesMenu();
    updateControlsState();
}

void FrmMain::onLoadHeightmapRequested()
{
    if (!saveChanges(true)) {
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), lastWorkingDirectory(), tr("Heightmap files (*.map)"));

    if (fileName != "") {
        addRecentHeightmap(fileName);
        loadHeightmap(fileName);

        // If using heightmap
        if (ui->heightmap->useMap() && !m_heightmapMode) {
            // Restore original file
            useHeightmapToggled(false);
            // Apply heightmap
            useHeightmapToggled(true);
        }

        updateRecentFilesMenu();
        updateControlsState(); // Enable 'cmdHeightMapMode' button
    }
}

void FrmMain::onProgramTableContextMenuRequested(const QPoint &pos)
{
    if (m_communicator->senderState() != SenderState::Stopped) return;

    QModelIndexList selectedRows = ui->program->getSelectedRows();
    bool hasSelection = !selectedRows.isEmpty();
    int selectedRow = hasSelection ? selectedRows[0].row() : -1;
    ui->program->showTableContextMenu(pos, m_tableMenu, hasSelection, selectedRow, m_currentModel->rowCount());
}

void FrmMain::on_menuViewWindows_aboutToShow()
{
    QAction *action;
    QList<QAction*> al;

    foreach (QDockWidget *dock, findChildren<QDockWidget*>()) {
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

void FrmMain::on_menuViewPanels_aboutToShow()
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

void FrmMain::on_dockVisualizer_visibilityChanged(bool visible)
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
    ui->console->append(tr("Connection error ") + error);
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

    ui->control->updateControlsState(m_communicator->senderState(), state);

    // ui->spindle->...
    // ui->cmdSpindle->setEnabled(state == DeviceHold0 || ((m_communicator->senderState() != SenderTransferring) &&                                                        (m_communicator->senderState() != SenderStopping)));
}

void FrmMain::onMachineStateReceived(MachineState state)
{
    // Update controls state
    // ui->control->updateControlsState(state == DeviceState::Idle, m_communicator->deviceState());

    // ui->spindle->...
    // ui->cmdSpindle->setEnabled(state == DeviceHold0 || ((m_communicator->senderState() != SenderTransferring) &&
    //                                                     (m_communicator->senderState() != SenderStopping)));

    // Update "elapsed time" timer
    if ((m_communicator->senderState() == SenderState::Transferring) || (m_communicator->senderState() == SenderState::Stopping)) {
        int elapsed = QDateTime::currentSecsSinceEpoch() - m_startTime;
        QTime time(0, 0, 0);
        time.addSecs(elapsed);
        ui->visualizer->setSpendTime(time);
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
    switch (state) {
        case true:
            m_timerToolAnimation.start(25, this);
            // ui->cmdSpindle->setChecked(true);
            break;
        default:
            //m_timerToolAnimation.stop();
            // ui->cmdSpindle->setChecked(false);
            break;
    }
}

void FrmMain::onFloodStateReceived(bool state)
{
    ui->control->setFlood(state);
}

void FrmMain::onParserStateReceived(QString state)
{
    ui->visualizer->setParserState(state);
}

void FrmMain::onPinStateReceived(QString state)
{
    ui->visualizer->setPinState(state);
}

void FrmMain::onFeedSpindleSpeedReceived(int feedRate, int spindleSpeed)
{
    ui->visualizer->setSpeedState((QString(tr("F/S: %1 / %2")).arg(feedRate, spindleSpeed)));
}

void FrmMain::onSpindleSpeedReceived(int spindleSpeed)
{
    // ui->slbSpindle->setCurrentValue(spindleSpeed);
}

void FrmMain::onOverridesReceived(int feedOverride, int spindleOverride, int rapidOverride)
{
    updateOverride(ui->overrides->ui->slbFeed, feedOverride, '\x91');
    updateOverride(ui->overrides->ui->slbSpindle, spindleOverride, '\x9a');

    PartMainOverride::Overrides overrides = ui->overrides->overrides();

    ui->overrides->setRapid(rapidOverride);

    int target = overrides.rapidOverridden ? overrides.rapid : 100;

    if (rapidOverride != target) {
        switch (target) {
            case 25:
                m_communicator->sendRealtimeCommand(GRBL_LIVE_RAPID_FULL_RATE);
                break;
            case 50:
                m_communicator->sendRealtimeCommand(GRBL_LIVE_RAPID_HALF_RATE);
                break;
            case 100:
                m_communicator->sendRealtimeCommand(GRBL_LIVE_RAPID_QUARTER_RATE);
                break;
        }
    }
}

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
}

void FrmMain::onConsoleNewCommand(QString command, bool isInternal)
{
    if (isInternal) {
        qDebug() << "Internal commands not handled yet:" << command;

        return;
    }

    m_communicator->sendCommand(CommandSource::Console, command, TABLE_INDEX_UI);
}

void FrmMain::onStateBehaviorChanged(StateBehavior *sb)
{
    ui->state->setStatusText(sb->description(), "black", "white");
    ui->console->appendSystem(QString("State: %1").arg(sb->description()));
}

void FrmMain::onTimerConnection()
{
    // /openPortIfNeeded();

    // @TODO move it completely to communicator
    m_communicator->processConnectionTimer();
}

// @todo another way to update visualizer??
// void FrmMain::onTimerStateQuery()
// {
//     if (m_connection->isConnected() && m_communicator->m_resetCompleted && m_communicator->m_statusReceived) {
//         m_connection->sendByteArray(QByteArray(1, '?'));
//         m_communicator->m_statusReceived = false;
//     }

//     ui->glwVisualizer->setBufferState(QString(tr("Buffer: %1 / %2 / %3")).arg(bufferLength()).arg(m_communicator->m_commands.length()).arg(m_communicator->m_queue.length()));
// }

void FrmMain::onTableInsertLine()
{
    QModelIndexList selectedRows = ui->program->getSelectedRows();
    if (selectedRows.count() == 0 ||
        (m_communicator->senderState() == SenderState::Transferring) || (m_communicator->senderState() == SenderState::Stopping)) return;

    int row = selectedRows[0].row();

    m_currentModel->insertRow(row);
    m_currentModel->setData(m_currentModel->index(row, 2), GCodeItem::InQueue);

    updateParser();

    ui->program->selectRow(row);
}

void FrmMain::onTableDeleteLines()
{
    QModelIndexList selectedRows = ui->program->getSelectedRows();
    if (selectedRows.count() == 0 ||
        (m_communicator->senderState() == SenderState::Transferring) || (m_communicator->senderState() == SenderState::Stopping) ||
        QMessageBox::warning(this, this->windowTitle(), tr("Delete lines?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    QModelIndex firstRow = selectedRows[0];
    int rowsCount = selectedRows.count();
    if (selectedRows.last().row() == m_currentModel->rowCount() - 1) rowsCount--;

    if (firstRow.row() != m_currentModel->rowCount() - 1) {
        m_currentModel->removeRows(firstRow.row(), rowsCount);
    } else return;

    // Drop heightmap cache
    if (m_currentModel == &m_programModel) m_programHeightmapModel.clear();

    updateParser();

    ui->program->selectRow(firstRow.row());
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
        if (!m_programLoading) ui->program->setCurrentIndex(model->index(i1.row() + 1, 1));
    }

    if (!m_programLoading) {
        // Clear cached args
        model->setData(model->index(i1.row(), 5), QVariant());

        // Drop heightmap cache
        if (m_currentModel == &m_programModel) m_programHeightmapModel.clear();

        // Update visualizer
        updateParser();

        // Hightlight w/o current cell changed event (double hightlight on current cell changed)
        QList<LineSegment>& list = m_viewParser.getLineSegmentList();
        for (int i = 0; i < list.count() && list[i].getLineNumber() <= m_currentModel->data(m_currentModel->index(i1.row(), 4)).toInt(); i++) {
            list[i].setIsHightlight(true);
        }
    }
}

void FrmMain::onTableCurrentChanged(QModelIndex currentIndex, QModelIndex previousIndex)
{
    ui->visualizer->updateToolpathHighlighting(currentIndex.row(), previousIndex.row(), *m_currentProgram);
}

void FrmMain::onOverridingToggled(bool checked)
{
    Q_UNUSED(checked)

    PartMainOverride::Overrides overrides = ui->overrides->overrides();

    ui->grpOverriding->setProperty("overrided", overrides.feedOverridden | overrides.rapidOverridden | overrides.spindleOverridden);
    style()->unpolish(ui->grpOverriding);
    ui->grpOverriding->ensurePolished();
}

void FrmMain::onOverrideChanged()
{
//    updateProgramEstimatedTime(m_currentDrawer->viewParser()->getLineSegmentList());
}

void FrmMain::onActRecentFileTriggered()
{
    QAction *action = static_cast<QAction*>(sender());
    QString fileName = action->text();

    if (action != NULL) {
        if (!saveChanges(m_heightmapMode)) return;
        if (!m_heightmapMode) loadFile(fileName); else loadHeightmap(fileName);
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
//             m_communicator->sendCommands(CommandSource::ProgramAdditionalCommands, commands, TABLE_INDEX_UI);
//         }
//     }

//     m_program.reset(commandIndex);
//     m_lastDrawnLineIndex = 0;
//     // m_communicator->m_probeIndex = -1;

//     QList<LineSegment>& list = m_viewParser.getLineSegmentList();

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

//     m_communicator->setSenderStateAndEmitSignal(SenderState::Transferring);

//     ui->jog->storeAndResetKeyboardControl();
//     // m_storedKeyboardControl = ui->chkKeyboardControl->isChecked();
//     // ui->chkKeyboardControl->setChecked(false);

//     m_communicator->storeParserState();

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

//     m_program.reset(commandIndex);
//     // m_communicator->sendStreamerCommandsUntilBufferIsFull();
// }

void FrmMain::onSlbSpindleValueUserChanged()
{
    // m_communicator->m_updateSpindleSpeed = true;
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
//     m_communicator->sendCommand(command, COMMAND_TI_UI);
// }

void FrmMain::onDockTopLevelChanged(bool topLevel)
{
    Q_UNUSED(topLevel)
    static_cast<QWidget*>(sender())->setStyleSheet("");
}

// void FrmMain::onVisualizerCursorPosChanged(QPointF pos)
// {
//     m_cursorDrawer.setPosition(pos);
// }

// void FrmMain::onProgramLinesUpdated(int from, int to)
// {
//     qDebug() << "FrmMain::onProgramLinesUpdated from" << from << "to" << to;
// }

void FrmMain::updateHeightMapInterpolationDrawer(bool reset)
{
    if (m_settingsLoading) return;

    QRectF borderRect = ui->heightmap->borderRectFromTextboxes();
    // m_heightmapInterpolationDrawer.setBorderRect(borderRect);

    QVector<QVector<double>> *interpolationData = new QVector<QVector<double>>;

    int interpolationPointsX = m_heightmap.interpolationStepSize().width();// * (ui->txtHeightMapGridX->value() - 1) + 1;
    int interpolationPointsY = m_heightmap.interpolationStepSize().height();// * (ui->txtHeightMapGridY->value() - 1) + 1;

    double interpolationStepX = interpolationPointsX > 1 ? borderRect.width() / (interpolationPointsX - 1) : 0;
    double interpolationStepY = interpolationPointsY > 1 ? borderRect.height() / (interpolationPointsY - 1) : 0;

    for (int i = 0; i < interpolationPointsY; i++) {
        QVector<double> row;
        for (int j = 0; j < interpolationPointsX; j++) {

            double x = interpolationStepX * j + borderRect.x();
            double y = interpolationStepY * i + borderRect.y();

            row.append(reset ? qQNaN() : Interpolation::bicubicInterpolate(borderRect, &m_heightmapModel, x, y));
        }
        interpolationData->append(row);
    }

    // if (m_heightmapInterpolationDrawer.data() != NULL) {
    //     delete m_heightmapInterpolationDrawer.data();
    // }
    ui->visualizer->setInterpolationData(interpolationData, borderRect);

    // Update grid drawer
    ui->visualizer->updateHeightmapGrid();

    // Heightmap changed by table user input
    if (sender() == &m_heightmapModel) m_heightmapChanged = true;

    // Reset heightmapped program model
    m_programHeightmapModel.clear();
}

void FrmMain::preloadSettings()
{
    ConfigurationUI &uiConfiguration = m_configuration.uiModule();
    ConfigurationVisualizer &visualizerConfiguration = m_configuration.visualizerModule();

    ThemeManager::instance().setFontSize(uiConfiguration.fontSize());
    for (auto action : ui->menuFontSize->actions()) {
        action->setChecked(action->property("size").toInt() == uiConfiguration.fontSize());
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

void FrmMain::applyRecentFilesConfiguration(ConfigurationUI &uiConfiguration)
{
    updateRecentFilesMenu();
}

void FrmMain::loadSettings()
{
    QSettings set(m_settingsFileName, QSettings::IniFormat);

    m_settingsLoading = true;

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
    ShortcutsMap shortcutsMap;

    QByteArray ba = set.value("shortcuts").toByteArray();
    QDataStream s(&ba, QIODevice::ReadOnly);
    s >> shortcutsMap;

    for (int i = 0; i < shortcutsMap.count(); i++) {
        QAction *action = findChild<QAction*>(shortcutsMap.keys().at(i));
        if (action) action->setShortcuts(shortcutsMap.values().at(i));
    }

    // Menu
    ConfigurationUI &uiConfiguration = m_configuration.uiModule();
    ui->actViewLockWindows->setChecked(uiConfiguration.lockWindows());
    ui->actViewLockPanels->setChecked(uiConfiguration.lockPanels());
    ui->actViewDarkMode->setChecked(uiConfiguration.darkTheme());
    // @TODO move to configuration form
    //m_settings->restoreGeometry(set.value("formSettingsGeometry", m_settings->saveGeometry()).toByteArray());

    m_settingsLoading = false;

    emit settingsLoaded();
}

void FrmMain::restoreDockableLayoutState()
{
    QSettings set(m_settingsFileName, QSettings::IniFormat);

    ui->program->restoreHeaderState(set.value("header", QByteArray()).toByteArray());

    // Restore last commands list
    // ui->cboCommand->addItems(set.value("recentCommands", QStringList()).toStringList());
    // ui->cboCommand->setCurrentIndex(-1);

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

    ConfigurationUI& uiConfiguration = m_configuration.uiModule();

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
    restoreState(set.value("formMainState").toByteArray());

    //     // Maximized window state
    // show();
    // qApp->processEvents();
    // restoreState(set.value("formMainState").toByteArray());

    // Setup coords textboxes
    // @TODO do we need this here?
    // setupCoordsTextboxes();

    // Settings form geometry
    // m_settings->restoreGeometry(set.value("formSettingsGeometry").toByteArray());
    // m_settings->ui->splitMain->restoreState(set.value("settingsSplitMain").toByteArray());
}

void FrmMain::initializeFontSizeMenu()
{
    QAction* action;
    for (int i = 8; i <= 12; i++) {
        action = ui->menuFontSize->addAction(QString::number(i) + " pt");
        action->setProperty("size", i);
        action->setCheckable(true);
        connect(action, &QAction::triggered, this, [this](bool checked) {
            QAction* act = qobject_cast<QAction*>(sender());
            if (checked) {
                for (auto action : ui->menuFontSize->actions()) {
                    if (action != sender()) {
                        action->setChecked(false);
                    }
                }
            } else {
                // ignore unsetting
                act->setChecked(true);
                return;
            }

            int size = act->property("size").toInt();
            ThemeManager::instance().setFontSize(size);
            m_configuration.uiModule().setFontSize(size);
        });
    }
}

void FrmMain::saveSettings()
{
    QSettings set(m_settingsFileName, QSettings::IniFormat);

    emit settingsAboutToSave();

    ConfigurationUI &uiConfiguration = m_configuration.uiModule();
    // ConfigurationJogging &joggingConfiguration = m_configuration.joggingModule();

    // m_configuration.machineModule().setSpindleSpeed(ui->slbSpindle->value());
    uiConfiguration.setAutoScrollGCode(ui->program->isAutoScroll());

    set.setValue("header", ui->program->saveHeaderState());
//    set.setValue("settingsSplitMain", m_settings->ui->splitMain->saveState());
//    set.setValue("formGeometry", this->saveGeometry());
//    set.setValue("formSettingsGeometry", m_settings->saveGeometry());
//    uiConfiguration.setMainFormGeometry(this->geometry());
    //uiConfiguration.setSettingsFormGeometry(m_settings->geometry());

    // joggingConfiguration.setJogStep(ui->cboJogStep->currentText().toDouble());
    // joggingConfiguration.setJogFeed(ui->cboJogFeed->currentText().toInt());

    // set.setValue("jogSteps", (QStringList)ui->cboJogStep->items().mid(1, ui->cboJogStep->items().count() - 1));
    // set.setValue("jogStep", ui->cboJogStep->currentText());
    // set.setValue("jogFeeds", ui->cboJogFeed->items());
    // set.setValue("jogFeed", ui->cboJogFeed->currentText());

    QStringList list;

    // Docks
    set.setValue("formMainState", saveState());
    set.setValue("formMainGeometry", saveGeometry());

    // Shortcuts
    ShortcutsMap m;
    QByteArray ba;
    QDataStream s(&ba, QIODevice::WriteOnly);
    QList<QAction*> acts = findChildren<QAction*>(QRegularExpression("act.*"));

    foreach (QAction *a, acts) m[a->objectName()] = a->shortcuts();
    s << m;
    set.setValue("shortcuts", ba);

    // Panels
    uiConfiguration.setPanelModificationState(ui->scrollContentsModification->saveState());
    uiConfiguration.setPanelDeviceState(ui->scrollContentsDevice->saveState());
    uiConfiguration.setPanelUserState(ui->scrollContentsUser->saveState());
    qDebug() << "Saving panels state:" << ui->scrollContentsUser->saveState();

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
    m_connection = m_connectionManager.createConnection(mode);

    //connect(m_connection, SIGNAL(lineReceived(QString)), this, SLOT(onConnectionLineReceived(QString)));
    connect(m_connection, SIGNAL(error(QString)), this, SLOT(onConnectionError(QString)));
}

// void FrmMain::applyVisualizerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
// {
//     ui->glwVisualizer->setLineWidth(visualizerConfiguration.lineWidth());
//     ui->glwVisualizer->setAntialiasing(visualizerConfiguration.antialiasing());
//     ui->glwVisualizer->setMsaa(visualizerConfiguration.msaa());
//     ui->glwVisualizer->setZBuffer(visualizerConfiguration.zBuffer());
//     ui->glwVisualizer->setFov(visualizerConfiguration.fieldOfView());
//     ui->glwVisualizer->setNearPlane(visualizerConfiguration.nearPlane());
//     ui->glwVisualizer->setFarPlane(visualizerConfiguration.farPlane());
//     ui->glwVisualizer->setVsync(visualizerConfiguration.vsync());
//     ui->glwVisualizer->setFps(visualizerConfiguration.fpsLock());
//     ui->glwVisualizer->setColorBackground(visualizerConfiguration.backgroundColor());
//     ui->glwVisualizer->setColorText(visualizerConfiguration.textColor());

//     // Adapt visualizer buttons colors
//     const int LIGHTBOUND = 140;
//     const int NORMALSHIFT = 40;
//     const int HIGHLIGHTSHIFT = 80;

//     QColor base = visualizerConfiguration.backgroundColor();
//     bool light = base.value() > LIGHTBOUND;

//     // Use background color with some transparency for buttons background
//     ui->visualizerButtons->setStyleSheet(
//         ui->visualizerButtons->styleSheet().replace(
//             QRegularExpression("/\\* bbg \\*/ background-color: rgba\\([^;^\\}]+\\)"),
//                         QString("/* bbg */ background-color: rgba(%1,%2,%3,%4)").arg(base.red())
//                                                    .arg(base.green())
//                                                    .arg(base.blue())
//                 .arg(std::max(0, base.alpha() - 100))
//             )
//         );

//     ui->cmdToggleProjection->setIcon(QIcon(":/images/visualizer_toggle_view_mode.png"));
//     ui->cmdFit->setIcon(QIcon(":/images/fit_1.png"));
//     ui->cmdIsometric->setIcon(QIcon(":/images/visualizer_isometric.png"));
//     ui->cmdFront->setIcon(QIcon(":/images/visualizer_front.png"));
//     ui->cmdLeft->setIcon(QIcon(":/images/visualizer_left.png"));
//     ui->cmdTop->setIcon(QIcon(":/images/visualizer_top.png"));

//     if (!light) {
//         Utils::invertButtonIconColors(ui->cmdToggleProjection);
//         Utils::invertButtonIconColors(ui->cmdFit);
//         Utils::invertButtonIconColors(ui->cmdIsometric);
//         Utils::invertButtonIconColors(ui->cmdFront);
//         Utils::invertButtonIconColors(ui->cmdLeft);
//         Utils::invertButtonIconColors(ui->cmdTop);
//     }

//     QColor normal, highlight;

//     normal.setHsv(base.hue(), base.saturation(), base.value() + (light ? -NORMALSHIFT : NORMALSHIFT));
//     highlight.setHsv(base.hue(), base.saturation(), base.value() + (light ? -HIGHLIGHTSHIFT : HIGHLIGHTSHIFT));

//     ui->glwVisualizer->setStyleSheet(QString("QToolButton {border: 1px solid %1; \
//                 background-color: %3} QToolButton:hover {border: 1px solid %2;}")
//                 .arg(normal.name()).arg(highlight.name())
//                 .arg(base.name()));

//     m_cursorDrawer.setVisible(visualizerConfiguration.show3dCursor());
// }

void FrmMain::applyUIConfiguration(ConfigurationUI &uiConfiguration)
{
    ui->program->setAutoScroll(uiConfiguration.autoScrollGCode());
    ui->actViewDarkMode->setChecked(uiConfiguration.darkTheme());
    ThemeManager& tm = ThemeManager::instance();
    tm.setFontSize(uiConfiguration.fontSize());
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

void FrmMain::addDockableWindow(const QString title, QWidget *widget, Qt::DockWidgetArea area, Qt::Orientation orientation)
{
    QDockWidget *dock = new QDockWidget(tr(title.toStdString().c_str()));
    dock->setObjectName(title);
    dock->setMinimumHeight(200);
    dock->setWidget(widget);
    Utils::setDockableLocked(dock, m_configuration.uiModule().lockWindows());
    dock->setTitleBarWidget(new DockableTitle(dock));
    addDockWidget(area, dock, orientation);

    QAction* action = new QAction(title, ui->menuCentralWidget);
    action->setCheckable(true);
    action->setChecked(false);
    connect(action, &QAction::triggered, this, [this, action]() {
        switchCentralWidget(action);
    });
    ui->menuCentralWidget->addAction(action);

    m_centralWidgets.append({
        widget,
        dock,
        action,
        title
    });
}

void FrmMain::applySettings()
{
    ConfigurationVisualizer &visualizerConfiguration = m_configuration.visualizerModule();
    ConfigurationHeightmap &heightmapConfiguration = m_configuration.heightmapModule();
    ConfigurationMachine &machineConfiguration = m_configuration.machineModule();
    ConfigurationUI &uiConfiguration = m_configuration.uiModule();
    ConfigurationJogging &joggingConfiguration = m_configuration.joggingModule();

    ui->visualizer->applyVisualizerConfiguration(visualizerConfiguration, machineConfiguration);

    // @TODO watch for changes is communicator?
    // m_communicator->stopUpdatingState();
    // m_communicator->startUpdatingState(m_configuration.connectionModule().queryStateInterval());

    applySpindleConfiguration(machineConfiguration);
    applyJoggingConfiguration(joggingConfiguration);
    applyOverridesConfiguration(machineConfiguration);
    applyUIConfiguration(uiConfiguration);
    applyRecentFilesConfiguration(uiConfiguration);

    if (!m_connection || m_connection->supportedMode() != m_configuration.connectionModule().connectionMode()) {
        initializeConnection(m_configuration.connectionModule().connectionMode());

        if (m_communicator->connection()) {
            if (!m_communicator->execute(new ReconnectingBehavior(m_connection))) {
                ui->console->appendSystem("Couldn't update connection. Restart application.");
            }
        } else {
            m_communicator->setConnection(m_connection, false);
        }
    }
}

// void FrmMain::openPortIfNeeded()
// {
//     assert(m_communicator != nullptr);

//     if (m_connection->state() == ConnectionState::Connecting || m_connection->state() == ConnectionState::Connected) {
//         return;
//     }

//     if (m_connection->open()) {
//         ui->state->setStatusText(tr("Port opened"), "palette(button)", "palette(text)");
//     }
// }

void FrmMain::updateParser()
{
    assert(m_communicator->isMachineConfigurationReady());

    GCodeViewParser *viewParse = ui->visualizer->getCurrentParser();

    GcodeParser parser;
    parser.setTraverseSpeed(m_communicator->machineConfiguration().maxRate().x()); // uses only x axis speed
    if (m_configuration.visualizerModule().ignoreZ()) {
        parser.reset(QVector3D(qQNaN(), qQNaN(), 0));
    }

    ui->program->setTableUpdatesEnabled(false);

    QString stripped;
    QList<QString> args;

    QProgressDialog progress(tr("Updating..."), tr("Abort"), 0, m_currentModel->rowCount() - 2, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setFixedSize(progress.sizeHint());

    if (m_currentModel->rowCount() > PROGRESSMINLINES) {
        progress.show();
        progress.setStyleSheet("QProgressBar {text-align: center; qproperty-format: \"\"}");
    }

    for (int i = 0; i < m_currentModel->rowCount() - 1; i++) {
        // Get stored args
        args = (*m_currentProgram)[i].args;

        // Store args if none
        if (args.isEmpty()) {
            stripped = GcodePreprocessorUtils::removeComment((*m_currentProgram)[i].command);
            args = GcodePreprocessorUtils::splitCommand(stripped);
            (*m_currentProgram)[i].args = args;
        }

        // Add command to parser
        parser.addCommand(args);

        // Update table model
        (*m_currentProgram)[i].state = GCodeItem::InQueue;
        (*m_currentProgram)[i].response = QString();
        (*m_currentProgram)[i].lineNumber = parser.getCommandNumber();

        if (progress.isVisible() && (i % PROGRESSSTEP == 0)) {
            progress.setValue(i);
            qApp->processEvents();
            if (progress.wasCanceled()) break;
        }
    }
    progress.close();

    ui->program->setTableUpdatesEnabled(true);

    viewParse->reset();

    ConfigurationParser &configurationParser = m_configuration.parserModule();

    // updateProgramEstimatedTime(
        // viewParse->getLinesFromParser(
        //     &parser,
        //     configurationParser.arcApproximationValue(),
        //     configurationParser.arcApproximationMode() == ConfigurationParser::ParserArcApproximationMode::ByAngle
        // )
    // );
    ui->visualizer->updateCurrentDrawerGeometry();
    ui->visualizer->updateGCodeExtremes();
    updateControlsState();

    if (m_currentModel == &m_programModel) m_fileChanged = true;
}

// @TODO scripting only??
// void FrmMain::storeOffsetsVars(QString response)
// {
//     static QRegularExpression gx("\\[(G5[4-9]|G28|G30|G92|PRB):([\\d\\.\\-]+),([\\d\\.\\-]+),([\\d\\.\\-]+)");
//     static QRegularExpression tx("\\[(TLO):([\\d\\.\\-]+)");

//     int p = 0;
//     while ((p = gx.indexIn(response, p)) != -1) {
//         m_storedVars.setCoords(gx.cap(1), QVector3D(
//             gx.cap(2).toDouble(),
//             gx.cap(3).toDouble(),
//             gx.cap(4).toDouble()
//         ));

//         p += gx.matchedLength();
//     }

//     if (tx.indexIn(response) != -1) {
//         m_storedVars.setCoords(tx.cap(1), QVector3D(
//             0,
//             0,
//             tx.cap(2).toDouble()
//         ));
//     }
// }

void FrmMain::loadFile(QString filePath)
{
    GCodeThreadedLoader *loader = new GCodeThreadedLoader(this);
    int progressIndex = ui->console->appendProgress("Loading " + filePath);
    connect(loader, &GCodeThreadedLoader::progress, this, [this, progressIndex](int progress) {
        ui->console->setProgress(progressIndex, progress);
    });
    connect(loader, &GCodeThreadedLoader::cancelled, this, [this, loader]() {
        ui->console->appendSystem("Cancelled loading");
        loader->deleteLater();
    });
    connect(loader, &GCodeThreadedLoader::finished, this, [this, loader, filePath](GCodeLoaderData *data) {
        ui->console->appendSystem("Finished loading");
        this->applyLoaderGCode(data);
        delete data;
        loader->deleteLater();

        FilesManager& filesManager = FilesManager::instance();
        filesManager.setGcodeFilePath(filePath);
    });

    GCodeLoaderConfiguration configuration(m_configuration.parserModule());
    loader->loadFromFile(filePath, configuration);
}

void FrmMain::applyLoaderGCode(GCodeLoaderData *data)
{
    qDebug() << "Finished loading file" << data->gcode->count();

    assert(m_communicator->isMachineConfigurationReady());
    if (!m_communicator->isMachineConfigurationReady()) {
        return;
    }

    // Reset tables
    clearTable();
    m_probeModel.clear();
    m_programHeightmapModel.clear();
    // updateCurrentModel(&m_programModel);

    // Reset parsers
    m_viewParser.reset();
    m_probeParser.reset();

    // Reset code drawer
    ui->visualizer->useCodeDrawer();
    m_viewParser = *data->viewParser;
    ui->visualizer->loadNewProgram();

    // Update interface
    ui->heightmap->resetUseHeighmap();
    ui->grpHeightmap->setProperty("overrided", false);
    style()->unpolish(ui->grpHeightmap);
    ui->grpHeightmap->ensurePolished();

    // Reset tableview
    QByteArray headerState = ui->program->saveProgramHeaderState();
    ui->program->setProgramTableModel(NULL);

    // // Prepare parser
    // GcodeParser parser;
    // parser.setTraverseSpeed(m_communicator->machineConfiguration().maxRate().x()); // uses only x axis speed
    // if (m_codeDrawer->getIgnoreZ()) parser.reset(QVector3D(qQNaN(), qQNaN(), 0));

    // Block parser updates on table changes
    m_programLoading = true;

    // Prepare model
    m_program.clear();
    m_program.reset();
    m_program << *data->gcode;

    m_programModel.insertRow(m_programModel.rowCount());

    updateProgramEstimatedTime(data->viewParser->getLines());

    m_programLoading = false;

    // Set table model
    ui->program->setProgramModel(&m_programModel);
    ui->program->restoreHeaderState(headerState);

    // Update tableview
    // connect(ui->tblProgram->selectionModel(), &QItemSelectionModel::currentChanged, this, &FrmMain::onTableCurrentChanged);
    ui->program->selectFirstRow();

    //  Update code drawer
    ui->visualizer->updateCodeDrawer();
    ui->visualizer->fitCodeDrawer();

    resetHeightmap();
    updateControlsState();
}

void FrmMain::loadLines(QList<std::string> data)
{
    assert(m_communicator->isMachineConfigurationReady());
    if (!m_communicator->isMachineConfigurationReady()) {
        return;
    }

    // Reset tables
    clearTable();
    m_probeModel.clear();
    m_programHeightmapModel.clear();
    // updateCurrentModel(&m_programModel);

    // Reset parsers
    m_viewParser.reset();
    m_probeParser.reset();

    // Reset code drawer
    ui->visualizer->resetVisualization();

    QList<LineSegment> list;
    updateProgramEstimatedTime(list);

    // Update interface
    ui->heightmap->resetUseHeighmap();
    ui->grpHeightmap->setProperty("overrided", false);
    style()->unpolish(ui->grpHeightmap);
    ui->grpHeightmap->ensurePolished();

    // Reset tableview
    QByteArray headerState = ui->program->saveProgramHeaderState();
    ui->program->setProgramTableModel(NULL);

    // Prepare parser
    GcodeParser parser;
    parser.setTraverseSpeed(m_communicator->machineConfiguration().maxRate().x()); // uses only x axis speed
    if (m_configuration.visualizerModule().ignoreZ()) {
        parser.reset(QVector3D(qQNaN(), qQNaN(), 0));
    }

    // Block parser updates on table changes
    m_programLoading = true;

    // Prepare model
    m_program.clear();
    m_program.reserve(data.count());

    QProgressDialog progress(tr("Opening file..."), tr("Abort"), 0, data.count(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setFixedSize(progress.sizeHint());
    if (data.count() > PROGRESSMINLINES) {
        progress.show();
        progress.setStyleSheet("QProgressBar {text-align: center; qproperty-format: \"\"}");
    }

    std::string command;
    std::string stripped;
    std::string trimmed;
    QList<QString> args;
    GCodeItem item;

    QList<std::string>::iterator dataIterator = data.begin();
    int remaining = data.count();
    for (dataIterator = data.begin(); dataIterator != data.end(); ++dataIterator)
    {
        command = *dataIterator; // data.takeFirst

        // Trim command

        trimmed = GcodePreprocessorUtils::trimCommand(command);

        if (!trimmed.empty()) {
            // Split command
            stripped = GcodePreprocessorUtils::removeComment(command);
            args = GcodePreprocessorUtils::splitCommand(stripped);

            parser.addCommand(args);

            item.command = QString::fromStdString(trimmed);
            item.state = GCodeItem::InQueue;
            item.lineNumber = parser.getCommandNumber();
            item.args = args;

            m_program << item;
        }

        remaining--;

        if (progress.isVisible() && (remaining % PROGRESSSTEP == 0)) {
            progress.setValue(progress.maximum() - remaining);
            qApp->processEvents();
            if (progress.wasCanceled()) break;
        }
    }
    progress.close();
    qApp->processEvents();

    m_programModel.insertRow(m_programModel.rowCount());

    updateProgramEstimatedTime(
        m_viewParser.getLinesFromParser(
            &parser,
            m_configuration.parserModule().arcApproximationValue(),
            m_configuration.parserModule().arcApproximationMode() == ConfigurationParser::ParserArcApproximationMode::ByAngle
        )
    );

    m_programLoading = false;

    // Set table model
    ui->program->setProgramModel(&m_programModel);
    ui->program->restoreHeaderState(headerState);

    // Update tableview
    // connect(ui->tblProgram->selectionModel(), &QItemSelectionModel::currentChanged, this, &FrmMain::onTableCurrentChanged);
    ui->program->selectFirstRow();

    //  Update code drawer
    ui->visualizer->updateCodeDrawer();
    ui->visualizer->fitCodeDrawer();

    // m_codeDrawer->update();
    // m_codeDrawer->updateData();
    ui->visualizer->exportCodeDrawerToFile("vertexdata.js");

    resetHeightmap();
    updateControlsState();
}

bool FrmMain::saveChanges(bool heightMapMode)
{
    if ((!heightMapMode && m_fileChanged)) {
        int res = QMessageBox::warning(this, this->windowTitle(), tr("G-code program file was changed. Save?"),
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (res == QMessageBox::Cancel) return false;
        else if (res == QMessageBox::Yes) on_actFileSave_triggered();
        m_fileChanged = false;
    }

    if (m_heightmapChanged) {
        int res = QMessageBox::warning(this, this->windowTitle(), tr("Heightmap file was changed. Save?"),
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (res == QMessageBox::Cancel) return false;
        else if (res == QMessageBox::Yes) {
            m_heightmapMode = true;
            on_actFileSave_triggered();
            m_heightmapMode = heightMapMode;
            updateRecentFilesMenu(); // Restore g-code files recent menu
        }

        m_fileChanged = false;
    }

    return true;
}

bool FrmMain::saveProgramToFile(QString fileName, GCode &model)
{
    QFile file(fileName);
    QDir dir;

    if (file.exists()) dir.remove(file.fileName());
    if (!file.open(QIODevice::WriteOnly)) return false;

    QTextStream textStream(&file);

    for (int i = 0; i < model.count() - 1; i++) {
        textStream << model[i].command << "\r\n";
    }

    file.close();

    return true;
}

void FrmMain::clearTable()
{
    m_programModel.clear();
    m_programModel.insertRow(0);
}

void FrmMain::resetHeightmap()
{
    // delete m_heightmapInterpolationDrawer.data();
    ui->visualizer->updateHeightmapInterpolation(true);

    ui->program->setHeightMapModel(NULL);
    m_heightmapModel.resize(1, 1);

    ui->heightmap->fileClosed();
    FilesManager::instance().resetHeightmapFile();
    m_heightmapChanged = false;
}

void FrmMain::newFile()
{
    // Reset tables
    clearTable();
    m_probeModel.clear();
    m_programHeightmapModel.clear();
    // updateCurrentModel(&m_programModel);

    // Reset parsers
    m_viewParser.reset();
    m_probeParser.reset();

    // Reset code drawer
    ui->visualizer->reset();

    QList<LineSegment> list;
    updateProgramEstimatedTime(list);

    FilesManager::instance().resetGcodeFile();
    ui->heightmap->resetUseHeighmap();
    //TODO heightmap
    // ui->grpHeightMap->setProperty("overrided", false);
    // Utils::refreshStyle(ui->grpHeightMap);

    // Reset tableview
    QByteArray headerState = ui->program->saveHeaderState();
    ui->program->setProgramModel(NULL);

    // Set table model
    ui->program->setProgramModel(&m_programModel);
    ui->program->restoreHeaderState(headerState);

    // Update tableview
    // connect(ui->tblProgram->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onTableCurrentChanged(QModelIndex,QModelIndex)));
    ui->program->selectFirstRow();

    resetHeightmap();

    updateControlsState();
}

void FrmMain::newHeightmap()
{
    m_heightmapModel.clear();
    onFileReset();
    ui->heightmap->setOpenFile(tr("Untitled"));
    FilesManager::instance().resetHeightmapFile();

    //TODO heightmap
    // updateHeightmapBorderDrawer();
    updateHeightmapGrid();

    m_heightmapChanged = false;

    updateControlsState();
}

void FrmMain::updateControlsState()
{
    bool portOpened = m_connection && m_connection->isConnected();
    bool process = (m_communicator->senderState() == SenderState::Transferring) || (m_communicator->senderState() == SenderState::Stopping);
    bool paused = (m_communicator->senderState() == SenderState::Pausing) || (m_communicator->senderState() == SenderState::Pausing2) || (m_communicator->senderState() == SenderState::Paused) || (m_communicator->senderState() == SenderState::ChangingTool);
    SenderState senderState = m_communicator->senderState();

    ui->grpState->setEnabled(portOpened);
    ui->control->setEnabled(portOpened);
    ui->spindle->setEnabled(portOpened);
    ui->jog->setEnabled(portOpened && ((senderState == SenderState::Stopped)
        || (senderState == SenderState::ChangingTool)));

    ui->console->setEnabled(portOpened && (!ui->jog->keyboardControl()));
    // ui->cmdCommandSend->setEnabled(portOpened);

    ui->control->updateControlsState(portOpened, process);

    //ui->spindle->...
    // ui->cmdSpindle->setEnabled(!process);

    ui->actFileNew->setEnabled(senderState == SenderState::Stopped);
    ui->actFileOpen->setEnabled(senderState == SenderState::Stopped);
    ui->program->setOpenButtonEnabled(senderState == SenderState::Stopped);
    ui->program->setResetButtonEnabled((senderState == SenderState::Stopped) && m_programModel.rowCount() > 1);
    ui->program->setSendButtonEnabled(portOpened && (senderState == SenderState::Stopped) && m_programModel.rowCount() > 1);
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
    ui->program->setAbortButtonEnabled(senderState != SenderState::Stopped && senderState != SenderState::Stopping);
    ui->menuRecent->setEnabled(
        (senderState == SenderState::Stopped) &&
        ((m_configuration.uiModule().hasAnyRecentFiles() && !m_heightmapMode) || (m_configuration.uiModule().hasAnyRecentHeightmaps() && m_heightmapMode))
    );
    ui->actFileSave->setEnabled(m_programModel.rowCount() > 1);
    ui->actFileSaveAs->setEnabled(m_programModel.rowCount() > 1);

    ui->program->setProgramTableEditTriggers((senderState != SenderState::Stopped) ? QAbstractItemView::NoEditTriggers :
        QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked |
        QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);

    if (!portOpened) {
        ui->state->setStatusText(tr("Not connected"), "palette(button)", "palette(text)");
        emit machineStateChanged(-1);
    }

    if (!process) ui->jog->restoreKeyboardControl();

#ifdef WINDOWS
    // if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7 && m_taskBarProgress) {
    //     m_taskBarProgress->setPaused(paused);
    //     if (m_communicator->senderState() == SenderStopped) m_taskBarProgress->hide();
    // }
#endif

    ui->program->updateButtonStyles();

    // Heightmap
    // m_heightmapBorderDrawer.setVisible(ui->chkHeightMapBorderShow->isChecked() && m_heightmapMode);
    // m_heightmapGridDrawer.setVisible(true);//ui->chkHeightMapGridShow->isChecked() && m_heightmapMode);
    ui->visualizer->setHeightmapInterpolationVisible(ui->heightmap->showInterpolationGrid() && m_heightmapMode);

    ui->centralWidgetTitle->setTitle(m_heightmapMode ? tr("Heightmap") : tr("G-code program"));
    ui->centralWidgetTitle->setProperty("overrided", m_heightmapMode);

    // ui->cboJogStep->setEditable(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogFeed->setEditable(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogStep->setEnabled(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogFeed->setEnabled(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogStep->setStyleSheet(QString("font-size: %1").arg(m_configuration.uiModule().fontSize()));
    // ui->cboJogFeed->setStyleSheet(ui->cboJogStep->styleSheet());

    ui->program->setHeightMapVisible(m_heightmapMode);
    ui->program->setProgramVisible(!m_heightmapMode);

    ui->program->setSendButtonText(m_heightmapMode ? tr("Probe") : tr("Send"));

    ui->heightmap->updateControlsState(
        !process && m_programModel.rowCount() > 1,
        m_heightmapMode
    );

    ui->actFileSaveTransformedAs->setVisible(ui->heightmap->useMap());

    ui->program->setSendMenuFirstActionEnabled(!ui->heightmap->heightmapMode());

    ui->visualizer->setSelectionVisible(!ui->heightmap->heightmapMode());
}

void FrmMain::updateLayouts()
{
    this->update();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void FrmMain::updateRecentFilesMenu()
{
    ui->menuRecent->clear();
    QMenu *fileOpenMenu = ui->program->getFileOpenMenu();
    fileOpenMenu->clear();

    QStringList files = !m_heightmapMode ? m_configuration.uiModule().recentFiles() : m_configuration.uiModule().recentHeightmaps();
    if (!files.empty())
    {
        QStringList::const_iterator it = files.constEnd();
        while (it != files.constBegin()) {
            --it;
            QAction *action = new QAction(*it, this);
            connect(action, &QAction::triggered, this, &FrmMain::onActRecentFileTriggered);
            ui->menuRecent->addAction(action);
            fileOpenMenu->addAction(action);
        }

        ui->menuRecent->addSeparator();
        fileOpenMenu->addSeparator();

        QAction *clearAction = new QAction(tr("&Clear"), this);
        connect(clearAction, &QAction::triggered, this, &FrmMain::onActRecentClearTriggered);

        ui->menuRecent->addAction(clearAction);
        fileOpenMenu->addAction(clearAction);
    }

    updateControlsState();
}

void FrmMain::updateOverride(SliderBox *slider, int value, char command)
{
    slider->setCurrentValue(value);

    int target = slider->isChecked() ? slider->value() : 100;
    bool smallStep = abs(target - slider->currentValue()) < 10 || m_configuration.connectionModule().queryStateInterval() < 100;

    if (slider->currentValue() < target) {
        m_connection->sendByteArray(QByteArray(1, char(smallStep ? command + 2 : command)));
    } else if (slider->currentValue() > target) {
        m_connection->sendByteArray(QByteArray(1, char(smallStep ? command + 3 : command + 1)));
    }
}

void FrmMain::updateJogTitle()
{
    if (ui->grpJog->isChecked() || !ui->jog->keyboardControl()) {
        ui->grpJog->setTitle(tr("Jog"));
    } else if (ui->jog->keyboardControl()) {
        ui->grpJog->setTitle(tr("Jog") + QString(tr(" (%1/%2)"))
                                             .arg((ui->jog->stepSize() != JoggingContinuous) ? QString::number(ui->jog->stepSize()) : tr("C"))
                            .arg(ui->jog->feedRate()));
    }
}

void FrmMain::addRecentFile(QString fileName)
{
    m_configuration.uiModule().addRecentFile(fileName);
    m_configuration.save();
}

void FrmMain::addRecentHeightmap(QString fileName)
{
    m_configuration.uiModule().addRecentHeightmap(fileName);
    m_configuration.save();
}

//TODO heightmap
// void FrmMain::updateHeightmapBorderDrawer()
// {
//     if (m_settingsLoading) return;

//     ui->visualizer->setHeightmapBorderRect(ui->heightmap->borderRectFromTextboxes());
// }

bool FrmMain::updateHeightmapGrid()
{
    if (m_settingsLoading) return true;

    // Grid map changing warning
    bool nan = true;
    for (int i = 0; i < m_heightmapModel.rowCount(); i++)
        for (int j = 0; j < m_heightmapModel.columnCount(); j++)
            if (!qIsNaN(m_heightmapModel.data(m_heightmapModel.index(i, j), Qt::UserRole).toDouble())) {
                nan = false;
                break;
            }
    if (!nan && QMessageBox::warning(this, this->windowTitle(), tr("Changing grid settings will reset probe data. Continue?"),
                                                           QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return false;

    // Update grid drawer
    QRectF borderRect = ui->heightmap->borderRectFromTextboxes();
    // ui->visualizer->heightmapGridDrawer()->setBorderRect(borderRect);
    // ui->visualizer->heightmapGridDrawer()->setGridSize(QPointF(ui->txtHeightMapGridX->value(), ui->txtHeightMapGridY->value()));
    // ui->visualizer->heightmapGridDrawer()->setZBottom(ui->txtHeightMapGridZBottom->value());
    // ui->visualizer->heightmapGridDrawer()->setZTop(ui->txtHeightMapGridZTop->value());

    // Reset model
    int gridPointsX = m_heightmap.gridSize().width();
    int gridPointsY = m_heightmap.gridSize().height();

    m_heightmapModel.resize(gridPointsX, gridPointsY);
    ui->program->setHeightMapModel(NULL);
    ui->program->setHeightMapModel(&m_heightmapModel);
    resizeTableHeightmapSections();

    // Update interpolation
    ui->visualizer->updateHeightmapInterpolation(true);

    // Generate probe program
    double gridStepX = gridPointsX > 1 ? borderRect.width() / (gridPointsX - 1) : 0;
    double gridStepY = gridPointsY > 1 ? borderRect.height() / (gridPointsY - 1) : 0;

    m_programLoading = true;
    m_probeModel.clear();
    m_probeModel.insertRow(0);

    m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G21G90F%1G0Z%2").
                    arg(m_heightmap.probeFeed()).arg(m_heightmap.zBottomTop().top));
    m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G0X0Y0"));
    m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G38.2Z%1")
                         .arg(m_heightmap.zBottomTop().bottom));
    m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G0Z%1")
                         .arg(m_heightmap.zBottomTop().top));

    double x, y;

    for (int i = 0; i < gridPointsY; i++) {
        y = borderRect.top() + gridStepY * i;
        for (int j = 0; j < gridPointsX; j++) {
            x = borderRect.left() + gridStepX * (i % 2 ? gridPointsX - 1 - j : j);
            m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G0X%1Y%2")
                                 .arg(x, 0, 'f', 3).arg(y, 0, 'f', 3));
            m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G38.2Z%1")
                                 .arg(m_heightmap.zBottomTop().bottom));
            m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G0Z%1")
                                 .arg(m_heightmap.zBottomTop().top));
        }
    }

    m_programLoading = false;

    if (ui->visualizer->isCurrentDrawerProbeMode()) updateParser();

    m_heightmapChanged = true;
    return true;
}

// void FrmMain::updateHeightmapGrid(double arg1)
// {
//     if (sender()->property("previousValue").toDouble() != arg1 && !updateHeightmapGrid())
//         static_cast<QDoubleSpinBox*>(sender())->setValue(sender()->property("previousValue").toDouble());
//     else sender()->setProperty("previousValue", arg1);
// }

void FrmMain::resizeTableHeightmapSections()
{
    ui->program->resizeHeightMapSections();
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

        if ((m_communicator->senderState() != SenderState::Transferring) && (m_communicator->senderState() != SenderState::Stopping)
            && ui->jog->keyboardControl() && !ev->isAutoRepeat())
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
//     m_program->setModel(m_currentModel);
// }

// Updates tool position in visualizer and marks toolpath segments as drawn when tool reaches them
// during active program execution (excluding check mode)
void FrmMain::updateToolPositionAndToolpathShadowing(QVector3D toolPosition)
{
    SenderState senderState = m_communicator->senderState();
    MachineState deviceState = m_communicator->machineState();

    if (((senderState == SenderState::Transferring) || (senderState == SenderState::Stopping)
         || (senderState == SenderState::Pausing) || (senderState == SenderState::Pausing2) || (senderState == SenderState::Paused))
         && deviceState != MachineState::Check) {
        int lineIndex = m_currentModel->data(m_currentModel->index(m_program.processedCommandIndex(), 4)).toInt();
        ui->visualizer->updateToolTracking(toolPosition, lineIndex, m_program);
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

//     if ((m_communicator->m_senderState != SenderState::Stopping) && m_program.processedCommandIndex() < m_currentModel->rowCount() - 1) {
//         int i;
//         QList<int> drawnLines;

//         for (i = m_lastDrawnLineIndex; i < list.count()
//                                                && list[i].getLineNumber()
//                                                 <= (m_currentModel->data(m_currentModel->index(m_program.processedCommandIndex(), 4)).toInt()); i++) {
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
\
QString FrmMain::lastWorkingDirectory()
{
    return m_configuration.uiModule().currentWorkingDirectory();
}

QTime FrmMain::updateProgramEstimatedTime(QList<LineSegment>& lines)
{
    double time = 0;
    PartMainOverride::Overrides overrides = ui->overrides->overrides();

    for (int i = 0; i < lines.count(); i++) {
        LineSegment& ls = lines[i];
        double length = (ls.getEnd() - ls.getStart()).length();

        if (!qIsNaN(length) && !qIsNaN(ls.getSpeed()) && ls.getSpeed() != 0) time +=
                length / ((overrides.feedOverridden && !ls.isFastTraverse())
                          ? (ls.getSpeed() * overrides.feed / 100) :
                            (overrides.rapidOverridden && ls.isFastTraverse())
                             ? (ls.getSpeed() * overrides.rapid / 100) : ls.getSpeed());
    }

    time *= 60;

    QTime t;

    t.setHMS(0, 0, 0);
    t = t.addSecs(time);

    ui->visualizer->setSpendTime(QTime(0, 0, 0));
    ui->visualizer->setEstimatedTime(t);

    return t;
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



// int FrmMain::buttonSize()
// {
//     return ui->cmdHome->minimumWidth();
// }

void FrmMain::onTransferCompleted()
{
    // Shadow last segment and reset
    ui->visualizer->finalizeTransfer();

    updateControlsState();

    // Show message box
    qApp->beep();
    // m_communicator->stopUpdatingState();
    // m_timerConnection.stop();

    QMessageBox::information(this, qApp->applicationDisplayName(), tr("Job done.\nTime elapsed: %1")
                                .arg(ui->visualizer->spendTime().toString("hh:mm:ss")));

    // m_timerConnection.start();
    // m_communicator->startUpdatingState();
}

QString FrmMain::getLineInitCommands(int row)
{
    int commandIndex = row;
    int lineNumber = m_currentModel->data(m_currentModel->index(commandIndex, 4)).toInt();

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

void FrmMain::initializeCentralWidgets()
{
    m_centralWidgets = {
        {ui->program, ui->dockProgram, ui->actViewCentralProgram, "G-code program"},
        {ui->visualizer, ui->dockVisualizer, ui->actViewCentralVisualizer, "Visualizer"}
    };
}

void FrmMain::switchCentralWidget(QAction* action)
{
    // If action is being unchecked, re-check it and return
    if (!action->isChecked()) {
        const QSignalBlocker blocker(action);
        action->setChecked(true);
        return;
    }

    // Find requested widget config
    CentralWidgetConfig* requestedConfig = nullptr;
    for (auto& config : m_centralWidgets) {
        if (config.action == action) {
            requestedConfig = &config;
            break;
        }
    }

    if (!requestedConfig) {
        return;
    }

    // Find and undock current central widget
    CentralWidgetConfig* currentConfig = nullptr;
    for (auto& config : m_centralWidgets) {
        if (config.widget->parentWidget() == ui->centralWidget) {
            currentConfig = &config;
            break;
        }
    }

    if (!currentConfig || currentConfig == requestedConfig) {
        return;
    }

    // Uncheck all other actions
    for (auto& config : m_centralWidgets) {
        if (config.action != action) {
            const QSignalBlocker blocker(config.action);
            config.action->setChecked(false);
        }
    }

    // Remember visibility state of requested dock
    bool dockWasVisible = requestedConfig->dock->isVisible();

    // Undock requested widget
    requestedConfig->widget->setParent(nullptr);
    requestedConfig->dock->hide();

    // Remove current widget from central
    ui->centralWidget->layout()->removeWidget(currentConfig->widget);

    // Dock current widget
    currentConfig->dock->setWidget(currentConfig->widget);
    currentConfig->dock->setVisible(dockWasVisible);

    // Add requested widget to central
    ui->centralWidget->layout()->addWidget(requestedConfig->widget);
    ui->centralWidgetTitle->setTitle(requestedConfig->title);
}
