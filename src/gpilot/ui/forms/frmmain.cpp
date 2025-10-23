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
#include <QScrollBar>
#include <QShortcut>
#include <QAction>
#include <QLayout>
#include <QDrag>
#include <QMimeData>
#include <QTranslator>
#include <QDockWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include "core/globals.h"
#include "ui/forms/frmmain.h"
#include "utils/utils.h"
#include "ui/forms/partials/main/partmainjog.h"
#include "ui/forms/partials/main/partmaincontrol.h"
#include "ui/forms/partials/main/partmainvirtualsettings.h"
#include "modules/pendant/pendant.h"
#include "modules/camera/camera.h"
#include "ui_frmmain.h"
#include "ui_frmsettings.h"
#include "ui_partmainoverride.h"
#include "ui/widgets/widgetmimedata.h"
#include "io/connection/connectionmanager.h"
#include "ui/drawers/vertexdataexporter.h"
#include "core/gcode/loader/gcodethreadedloader.h"

#define FILE_FILTER_TEXT "G-Code files (*.nc *.ncc *.ngc *.tap *.gc *.gcode *.txt)"

frmMain::frmMain(Configuration &configuration, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmMain),
    m_heightmapModel(m_heightmap),
    m_programModel(m_program),
    m_probeModel(m_program),
    m_programHeightmapModel(m_program),
    m_connectionManager(this, configuration.connectionModule()),
    m_connection(nullptr),
    m_configuration(configuration)
{
    // Loading settings
    m_settingsFileName = qApp->applicationDirPath() + "/settings.ini";
    preloadSettings();

    initializeCommunicator();

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

    Utils::setVisualMode(this, m_configuration.uiModule().darkTheme());

    ui->jog->initialize(m_configuration.joggingModule());

    ui->console->initialize(m_configuration.consoleModule());
    connect(ui->console, &partMainConsole::newCommand, this, &frmMain::onConsoleNewCommand);
    ui->console->append(QString("G-Candle %1 started").arg( qApp->applicationVersion()));
    ui->console->append("---");

    connect(ui->chkHideComments, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        m_programModel.setCommentsVisible(state != Qt::Checked);
    });

    connect(ui->control, &partMainControl::unlock, this, [this]() {
        // m_communicator->m_updateSpindleSpeed = true;
        // m_communicator->sendCommand(CommandSource::GeneralUI, "$X", TABLE_INDEX_UI);
        m_communicator->unlock();
    });
    connect(ui->control, &partMainControl::home, this, [this]() {
        // m_communicator->m_homing = true;
        // m_communicator->m_updateSpindleSpeed = true;
        // m_communicator->sendCommand(CommandSource::GeneralUI, "$H", TABLE_INDEX_UI);

        m_communicator->home();
    });
    connect(ui->control, &partMainControl::probe, this, [this]() {
        m_communicator->probe();
    });
    connect(ui->control, &partMainControl::reset, this, [this]() {
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

    connect(ui->jog, &partMainJog::jog, this, [this](JoggindDir dir, QVector3D jog) {
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
    connect(ui->jog, &partMainJog::stop, this, [this]() {
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

#ifdef WINDOWS
    // if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7) {
    //     m_taskBarButton = NULL;
    //     m_taskBarProgress = NULL;
    // }
#endif

//    ui->scrollArea->updateMinimumWidth();

    m_heightmapMode = false;
    m_lastDrawnLineIndex = 0;
    m_program.resetProcessed();
    m_programLoading = false;
    //updateCurrentModel(&m_programModel);

    // Dock widgets
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    ui->widgetHeightmapSettings->setVisible(false);

    // ui->cmdToggleProjection->setParent(ui->glwVisualizer);
    // ui->cmdFit->setParent(ui->glwVisualizer);
    // ui->cmdIsometric->setParent(ui->glwVisualizer);
    // ui->cmdTop->setParent(ui->glwVisualizer);
    // ui->cmdFront->setParent(ui->glwVisualizer);
    // ui->cmdLeft->setParent(ui->glwVisualizer);
    // ui->cmdRotationCube->setParent(ui->glwVisualizer);

    ui->cmdHeightMapBorderAuto->setMinimumHeight(ui->chkHeightMapBorderShow->sizeHint().height());
    ui->cmdHeightMapCreate->setMinimumHeight(ui->cmdFileOpen->sizeHint().height());
    ui->cmdHeightMapLoad->setMinimumHeight(ui->cmdFileOpen->sizeHint().height());
    ui->cmdHeightMapMode->setMinimumHeight(ui->cmdFileOpen->sizeHint().height());

    // ui->cboJogStep->setValidator(new QDoubleValidator(0, 10000, 2));
    // ui->cboJogFeed->setValidator(new QIntValidator(0, 100000));
    // connect(ui->cboJogStep, &ComboBoxKey::currentTextChanged, this, &frmMain::updateJogTitle);
    // connect(ui->cboJogFeed, &ComboBoxKey::currentTextChanged, this, &frmMain::updateJogTitle);

    QMenu *menu;

    // Prepare Open and Send menus
    menu = ui->cmdFileOpen->menu();
    menu->addAction(tr("Open G-Code file"), this, SLOT(on_cmdFileOpen_clicked()));
    menu->addAction(tr("Open Heightmap file"), this, SLOT(on_cmdHeightMapLoad_clicked()));

    menu = ui->cmdFileSend->menu();
    menu->addAction(tr("Send from current line"), this, SLOT(onActSendFromLineTriggered()));

    foreach (StyledToolButton* button, this->findChildren<StyledToolButton*>(QRegularExpression("cmdUser\\d"))) {
        connect(button, SIGNAL(clicked(bool)), this, SLOT(onCmdUserClicked(bool)));
    }

    m_originDrawer = new OriginDrawer();
    m_codeDrawer = new GcodeDrawer();
    m_codeDrawer->setViewParser(&m_viewParser);
    m_probeDrawer = new GcodeDrawer();
    m_probeDrawer->setViewParser(&m_probeParser);
    m_probeDrawer->setVisible(false);
    m_heightmapGridDrawer.setModel(&m_heightmapModel);
    m_currentDrawer = m_codeDrawer;

    m_tableMenu = new QMenu(this);
    m_tableMenu->addAction(tr("&Insert line"), this, SLOT(onTableInsertLine()), QKeySequence(Qt::Key_Insert));
    m_tableMenu->addAction(tr("&Delete lines"), this, SLOT(onTableDeleteLines()), QKeySequence(Qt::Key_Delete));

    initializeVisualizer();

    connect(ui->glwVisualizer, &GLContainer::resized, this, &frmMain::placeVisualizerButtons);
    connect(ui->glwVisualizer, &GLContainer::cursorPosChanged, this, &frmMain::onVisualizerCursorPosChanged);
    connect(ui->glwVisualizer, &GLContainer::entered, this, [this]() {
        m_cursorDrawer.setVisible(true);
    });
    connect(ui->glwVisualizer, &GLContainer::left, this, [this]() {
        m_cursorDrawer.setVisible(false);
    });
    connect(ui->glwVisualizer, &GLContainer::goToCursor, this, [this](QPointF pos) {
        m_communicator->execute(new GoToBehavior(pos, m_configuration.joggingModule().feed()));
    });
    connect(&m_programModel, &QAbstractItemModel::dataChanged, this, &frmMain::onTableCellChanged);
    connect(&m_programHeightmapModel, &QAbstractItemModel::dataChanged, this, &frmMain::onTableCellChanged);
    connect(&m_probeModel, &QAbstractItemModel::dataChanged, this, &frmMain::onTableCellChanged);
    connect(&m_heightmapModel, SIGNAL(dataChangedByUserInput()), this, SLOT(updateHeightMapInterpolationDrawer()));

    ui->tblProgram->setModel(&m_programModel);
    ui->tblProgram->setItemDelegate(&m_programItemDelegate);
    ui->tblProgram->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    connect(ui->tblProgram->verticalScrollBar(), &QAbstractSlider::actionTriggered, this, &frmMain::onScroolBarAction);
    connect(ui->tblProgram->selectionModel(), &QItemSelectionModel::currentChanged, this, &frmMain::onTableCurrentChanged);
    clearTable();

    m_senderErrorBox = new QMessageBox(QMessageBox::Warning, qApp->applicationDisplayName(), QString(),
                                       QMessageBox::Ignore | QMessageBox::Abort, this);
    m_senderErrorBox->setCheckBox(new QCheckBox(tr("Don't show again")));

    // Loading settings
    loadSettings();
    ui->tblProgram->hideColumn(4);
    ui->tblProgram->hideColumn(5);

    updateControlsState();

    // Setting up spindle slider box
    // ui->slbSpindle->setTitle(tr("Speed:"));
    // ui->slbSpindle->setCheckable(false);
    // ui->slbSpindle->setChecked(true);
    // connect(ui->slbSpindle, &SliderBox::valueUserChanged, this, &frmMain::onSlbSpindleValueUserChanged);
    // connect(ui->slbSpindle, &SliderBox::valueChanged, this, &frmMain::onSlbSpindleValueChanged);

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

    // Camera
    addWindow(
        "Camera",
        new Camera(),
        Qt::TopDockWidgetArea,
        Qt::Horizontal
    );

    // After everything is set up, restore layout
    restoreDockableLayoutState();
}

frmMain::~frmMain()
{
    delete m_communicator;
    delete m_connection;
    delete m_senderErrorBox;
    delete ui; ui = nullptr;
}

void frmMain::initializeCommunicator()
{
    m_communicator = new Communicator(
        this,
        nullptr,
        &m_configuration
    );
    //m_program = new GCode();
    // @TODO temporary!
    // m_communicator->streamCommands(m_program);

    connect(m_communicator, &Communicator::machinePosChanged, this, &frmMain::onMachinePosChanged);
    connect(m_communicator, &Communicator::workPosChanged, this, &frmMain::onWorkPosChanged);
    connect(m_communicator, &Communicator::machineStateReceived, this, &frmMain::onMachineStateReceived);
    connect(m_communicator, &Communicator::machineStateChanged, this, &frmMain::onMachineStateChanged);
    connect(m_communicator, &Communicator::senderStateReceived, this, &frmMain::onSenderStateReceived);
    connect(m_communicator, SIGNAL(spindleStateReceived(bool)), this, SLOT(onSpindleStateReceived(bool)));
    connect(m_communicator, &Communicator::floodStateReceived, this, &frmMain::onFloodStateReceived);
    connect(m_communicator, &Communicator::commandSent, this, &frmMain::onCommandSent);
    connect(m_communicator, &Communicator::commandResponseReceived, this, &frmMain::onCommandResponseReceived);
    connect(m_communicator, &Communicator::parserStateReceived, this, &frmMain::onParserStateReceived);
    connect(m_communicator, &Communicator::welcomeMessageReceived, this, [this](QString message) {
        ui->console->appendSystem(message);
    });
    connect(m_communicator, &Communicator::log, this, [this](QString message) {
        ui->console->append(message);
    });
    connect(m_communicator, &Communicator::pinStateReceived, this, &frmMain::onPinStateReceived);
    connect(m_communicator, &Communicator::spindleSpeedReceived, this, &frmMain::onSpindleSpeedReceived);
    connect(m_communicator, &Communicator::commandProcessed, this, &frmMain::onCommandProcessed);
    connect(m_communicator, SIGNAL(feedSpindleSpeedReceived(int,int)), this, SLOT(onFeedSpindleSpeedReceived(int,int)));
    connect(m_communicator, &Communicator::overridesReceived, this, &frmMain::onOverridesReceived);
    connect(m_communicator, &Communicator::toolPositionReceived, this, &frmMain::onToolPositionReceived);
    connect(m_communicator, &Communicator::transferCompleted, this, &frmMain::onTransferCompleted);
    connect(m_communicator, &Communicator::aborted, this, &frmMain::onAborted);
    connect(m_communicator, &Communicator::machineConfigurationReceived, this, [this](PhysicalMachineConfiguration configuration) {
        m_partMainVirtualSettings->deviceConfigurationReceived(configuration);
    });
    // connect(m_communicator, &Communicator::statusReceived, this, [this]() {
    //     jogContinuous();
    // });
    connect(m_communicator, &Communicator::stateBehaviorChanged, this, &frmMain::onStateBehaviorChanged);
    connect(m_communicator, &Communicator::connectionChanged, this, [this](Connection *connection) {
        ui->state->setConName(connection->name());
    });
}

void frmMain::initializeVisualizer()
{
    *ui->glwVisualizer << &m_tableSurfaceDrawer << m_originDrawer << m_codeDrawer << m_probeDrawer
                       << &m_toolDrawer << &m_cursorDrawer << &m_heightmapBorderDrawer
                       << &m_heightmapGridDrawer << &m_heightmapInterpolationDrawer
                       << &m_selectionDrawer << &m_machineBoundsDrawer;

    ui->glwVisualizer->fitDrawable();
}

void frmMain::showEvent(QShowEvent *se)
{
    Q_UNUSED(se)

    placeVisualizerButtons();

#ifdef WINDOWS
    // if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7) {
    //     if (m_taskBarButton == NULL) {
    //         m_taskBarButton = new QWinTaskbarButton(this);
    //         m_taskBarButton->setWindow(this->windowHandle());
    //         m_taskBarProgress = m_taskBarButton->progress();
    //     }
    // }
#endif

    if (m_firstShow) {
        Utils::positionDialog(this, m_configuration.uiModule().mainFormGeometry(), m_configuration.uiModule().mainFormMaximized());
        m_firstShow = false;
    }
}

void frmMain::hideEvent(QHideEvent *he)
{
    Q_UNUSED(he)
}

void frmMain::resizeEvent(QResizeEvent *re)
{
    QMainWindow::resizeEvent(re);

    placeVisualizerButtons();
    resizeTableHeightmapSections();

    if (!m_firstShow) {
        m_configuration.uiModule().setMainFormGeometry(this);
    }
}

void frmMain::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_timerToolAnimation.timerId()) {
        // m_toolDrawer.rotate((m_communicator->m_spindleCW ? -40 : 40) * (double)(ui->slbSpindle->currentValue())
        //                     / (ui->slbSpindle->maximum()));
        // m_cursorDrawer.rotate();
    } else {
        QMainWindow::timerEvent(te);
    }
}

void frmMain::closeEvent(QCloseEvent *ce)
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
    m_connection->close();

    m_communicator->clearCommandsAndQueue();
    // moved to communicator
    // if (m_communicator->m_queue.length() > 0) {
    //     m_communicator->m_commands.clear();
    //     m_communicator->m_queue.clear();
    // }

    saveSettings();
}

void frmMain::dragEnterEvent(QDragEnterEvent *dee)
{
    if (m_communicator->senderState() != SenderState::Stopped) return;

    if (dee->mimeData()->hasFormat("application/widget")) return;

    if (dee->mimeData()->hasFormat("text/plain") && !m_heightmapMode) dee->acceptProposedAction();
    else if (dee->mimeData()->hasFormat("text/uri-list") && dee->mimeData()->urls().count() == 1) {
        QString fileName = dee->mimeData()->urls().at(0).toLocalFile();

        if ((!m_heightmapMode && Utils::isGCodeFile(fileName)) || (m_heightmapMode && Utils::isHeightmapFile(fileName)))
            dee->acceptProposedAction();
    }
}

void frmMain::dropEvent(QDropEvent *de)
{
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
            m_programFileName.clear();
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

void frmMain::changeEvent(QEvent *ce)
{
    QMainWindow::changeEvent(ce);
    if (ce->type() == QEvent::WindowStateChange) {
        m_configuration.uiModule().setMainFormGeometry(this);
    }
}

void frmMain::moveEvent(QMoveEvent *me)
{
    QMainWindow::moveEvent(me);
    if (!m_firstShow) {
        m_configuration.uiModule().setMainFormGeometry(this);
    }
}

QMenu *frmMain::createPopupMenu()
{
    QMenu *menu = QMainWindow::createPopupMenu();

    foreach (QAction *a, menu->actions()) {
        if (a->text().contains("_spacer")) {
            a->setVisible(false);
        }
    }

    return menu;
}

void frmMain::on_actFileNew_triggered()
{
    if (!saveChanges(m_heightmapMode)) return;

    if (!m_heightmapMode) {
        newFile();
    } else {
        newHeightmap();
    }
}

void frmMain::on_actFileOpen_triggered()
{
    on_cmdFileOpen_clicked();
}

void frmMain::on_actFileSave_triggered()
{
    if (!m_heightmapMode) {
        // G-code saving
        if (m_programFileName.isEmpty()) on_actFileSaveAs_triggered(); else {
            saveProgramToFile(m_programFileName, m_program);
            m_fileChanged = false;
        }
    } else {
        // Height map saving
        if (m_heightmapFileName.isEmpty()) on_actFileSaveAs_triggered(); else saveHeightmap(m_heightmapFileName);
    }
}

void frmMain::on_actFileSaveAs_triggered()
{
    if (!m_heightmapMode) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save file as"), "", tr(FILE_FILTER_TEXT));

        if (!fileName.isEmpty()) if (saveProgramToFile(fileName, m_program)) {
            m_programFileName = fileName;
            m_fileChanged = false;

            addRecentFile(fileName);
            updateRecentFilesMenu();

            updateControlsState();
        }
    } else {
        QString fileName = (QFileDialog::getSaveFileName(this, tr("Save file as"), lastWorkingDirectory(), tr("Heightmap files (*.map)")));

        if (!fileName.isEmpty()) if (saveHeightmap(fileName)) {
            ui->txtHeightMap->setText(fileName.mid(fileName.lastIndexOf("/") + 1));
            m_heightmapFileName = fileName;
            m_heightmapChanged = false;

            addRecentHeightmap(fileName);
            updateRecentFilesMenu();

            updateControlsState();
        }
    }
}

void frmMain::on_actFileSaveTransformedAs_triggered()
{
    QString fileName = (QFileDialog::getSaveFileName(this, tr("Save file as"), lastWorkingDirectory(), tr(FILE_FILTER_TEXT)));

    if (!fileName.isEmpty()) {
//        saveProgramToFile(fileName, &m_programHeightmapModel);
    }
}

void frmMain::onActRecentClearTriggered()
{
    if (!m_heightmapMode) m_configuration.uiModule().clearRecentFiles();
        else m_configuration.uiModule().clearRecentHeightmaps();
    m_configuration.save();
    updateRecentFilesMenu();
}

void frmMain::on_actFileExit_triggered()
{
    close();
}

void frmMain::on_actServiceSettings_triggered()
{
    QList<QAction*> acts = findChildren<QAction*>(QRegularExpression("act.*"));

    // QTableWidget *table = m_settings->ui->tblShortcuts;

    // table->clear();
    // table->setColumnCount(3);
    // table->setRowCount(acts.count());
    // table->setHorizontalHeaderLabels(QStringList() << tr("Command") << tr("Text") << tr("Shortcuts"));

    // table->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    // table->verticalHeader()->setFixedWidth(table->verticalHeader()->sizeHint().width() + 11);

    // qSort(acts.begin(), acts.end(), frmMain::actionLessThan);
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

    QScopedPointer<frmSettings> form(new frmSettings(this, m_configuration));
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

void frmMain::on_actServiceConfigureGRBL_triggered()
{
    frmGrblConfigurator *form = new frmGrblConfigurator(this, m_configuration.uiModule(), m_communicator);
    form->exec();
    form->deleteLater();
}

void frmMain::on_actAbout_triggered()
{
    frmAbout *form = new frmAbout(this);
    form->exec();
    form->deleteLater();
}

void frmMain::on_actSpindleSpeedPlus_triggered()
{
    // ui->slbSpindle->setSliderPosition(ui->slbSpindle->sliderPosition() + 1);
}

void frmMain::on_actSpindleSpeedMinus_triggered()
{
    // ui->slbSpindle->setSliderPosition(ui->slbSpindle->sliderPosition() - 1);
}

void frmMain::on_actViewLockWindows_toggled(bool checked)
{
    QList<QDockWidget*> dl = findChildren<QDockWidget*>();

    foreach (QDockWidget *dock, dl) {
        Utils::setDockableLocked(dock, checked);
    }

    m_configuration.uiModule().setLockWindows(checked);
}

void frmMain::on_cmdFileOpen_clicked()
{
    if (!m_communicator->isMachineConfigurationReady()) {
        qWarning() << "[UI] Machine configuration is not ready";

        return;
    }

    if (!m_heightmapMode) {
        if (!saveChanges(false)) return;

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), "",
                                   tr(FILE_FILTER_TEXT";;All files (*.*)"));

        if (!fileName.isEmpty()) {
            m_configuration.uiModule().currentWorkingDirectory(fileName.left(fileName.lastIndexOf(QRegularExpression("[/\\\\]+"))));
        }

        if (fileName != "") {
            addRecentFile(fileName);
            updateRecentFilesMenu();

            loadFile(fileName);
        }
    } else {
        if (!saveChanges(true)) return;

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), lastWorkingDirectory(), tr("Heightmap files (*.map)"));

        if (fileName != "") {
            addRecentHeightmap(fileName);
            updateRecentFilesMenu();
            loadHeightmap(fileName);
        }
    }
}

void frmMain::on_cmdFileSend_clicked()
{
    m_program.reset();
    m_communicator->execute(new RunningBehavior(
        m_program
    ));

//     if (m_currentModel->rowCount() == 1) return;

//     on_cmdFileReset_clicked();

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

void frmMain::on_cmdFilePause_clicked(bool checked)
{
    static SenderState s;

    if (checked) {
        //PAUSE
        s = m_communicator->senderState();
        // setSenderState(SenderPaused);
        m_communicator->setSenderStateAndEmitSignal(SenderState::Pausing);
        ui->cmdFilePause->setText(tr("Pausing..."));
        ui->cmdFilePause->setEnabled(false);
    } else {
        //RESUME
        if (m_communicator->senderState() == SenderState::ChangingTool) {
            m_communicator->setSenderStateAndEmitSignal(SenderState::Transferring);
        } else {
            if (m_configuration.senderModule().usePauseCommands()) {
                m_communicator->sendCommands(CommandSource::ProgramAdditionalCommands, m_configuration.senderModule().afterPauseCommands());
            }
            m_communicator->setSenderStateAndEmitSignal(s);
        }
        updateControlsState();
    }
}

void frmMain::on_cmdFileAbort_clicked()
{
    ui->cmdFileAbort->setEnabled(false);
    m_communicator->abort();
}

void frmMain::on_cmdFileReset_clicked()
{
    m_program.reset();
    m_lastDrawnLineIndex = 0;
    m_communicator->m_probeIndex = -1;

    if (!m_heightmapMode) {
        QList<LineSegment>& list = m_viewParser.getLineSegmentList();

        QList<int> indexes;
        for (int i = 0; i < list.count(); i++) {
            list[i].setDrawn(false);
            indexes.append(i);
        }
        m_codeDrawer->update(indexes);

        ui->tblProgram->setUpdatesEnabled(false);

        for (int i = 0; i < m_currentProgram->count() - 1; i++) {
            (*m_currentProgram)[i].state = GCodeItem::InQueue;
            (*m_currentProgram)[i].response = QString();
        }
        ui->tblProgram->setUpdatesEnabled(true);

        ui->tblProgram->scrollTo(m_currentModel->index(0, 0));
        ui->tblProgram->clearSelection();
        ui->tblProgram->selectRow(0);

        ui->glwVisualizer->setSpendTime(QTime(0, 0, 0));
    } else {
        ui->txtHeightMapGridX->setEnabled(true);
        ui->txtHeightMapGridY->setEnabled(true);
        ui->txtHeightMapGridZBottom->setEnabled(true);
        ui->txtHeightMapGridZTop->setEnabled(true);

        delete m_heightmapInterpolationDrawer.data();
        m_heightmapInterpolationDrawer.setData(NULL);

        m_heightmapModel.clear();
        updateHeightmapGrid();
    }
}

// void frmMain::on_cmdCommandSend_clicked()
// {
//     QString command = ui->cboCommand->currentText();
//     if (command.isEmpty()) return;

//     ui->cboCommand->storeText();
//     ui->cboCommand->setCurrentText("");
//     m_communicator->sendCommand(command, COMMAND_TI_UI);
// }

// void frmMain::on_cmdClearConsole_clicked()
// {
//     ui->txtConsole->clear();
// }

// void frmMain::on_cmdHome_clicked()
// {
//     m_communicator->m_homing = true;
//     m_communicator->m_updateSpindleSpeed = true;
//     m_communicator->sendCommand(CommandSource::GeneralUI, "$H", COMMAND_TI_UI);
// }

// void frmMain::on_cmdCheck_clicked(bool checked)
// {
//     if (checked) {
//         m_communicator->storeParserState();
//         m_communicator->sendCommand(CommandSource::GeneralUI, "$C", COMMAND_TI_UI);
//     } else {
//         m_communicator->m_aborting = true;
//         m_communicator->reset();
//     };
// }

// void frmMain::on_cmdReset_clicked()
// {
//     m_communicator->reset();
//     //grblReset();
// }

// void frmMain::on_cmdUnlock_clicked()
// {
//     m_communicator->m_updateSpindleSpeed = true;
//     m_communicator->sendCommand(CommandSource::GeneralUI, "$X", COMMAND_TI_UI);
// }

// void frmMain::on_cmdHold_clicked(bool checked)
// {
//     m_connection->sendByteArray(QByteArray(1, checked ? (char)'!' : (char)'~'));
// }

// void frmMain::on_cmdSleep_clicked()
// {
//     m_communicator->sendCommand(CommandSource::GeneralUI, "$SLP", COMMAND_TI_UI);
// }

// void frmMain::on_cmdDoor_clicked()
// {
//     m_connection->sendByteArray(QByteArray(1, (char)0x84));
// }

// void frmMain::on_cmdFlood_clicked()
// {
//     m_connection->sendByteArray(QByteArray(1, (char)0xa0));
// }

// void frmMain::on_cmdSpindle_toggled(bool checked)
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

void frmMain::on_cmdSpindle_clicked(bool checked)
{
    if (ui->control->hold()) {
        m_connection->sendByteArray(QByteArray(1, char(0x9e)));
    } else {
        // m_communicator->sendCommand(CommandSource::GeneralUI, checked ? QString("M3 S%1").arg(ui->slbSpindle->value()) : "M5", TABLE_INDEX_UI);
    }
}

void frmMain::on_cmdTop_clicked()
{
    ui->glwVisualizer->setTopView();
}

void frmMain::on_cmdFront_clicked()
{
    ui->glwVisualizer->setFrontView();
}

void frmMain::on_cmdLeft_clicked()
{
    ui->glwVisualizer->setLeftView();
}

void frmMain::on_cmdIsometric_clicked()
{
    ui->glwVisualizer->setIsometricView();
}

void frmMain::on_cmdRotationCube_clicked()
{
    ui->glwVisualizer->toggleRotationCube();
}

void frmMain::on_cmdToggleProjection_clicked()
{
    ui->glwVisualizer->toggleProjectionType();
}

void frmMain::on_cmdFit_clicked()
{
    ui->glwVisualizer->fitDrawable(m_currentDrawer);
}

void frmMain::on_grpOverriding_toggled(bool checked)
{
    partMainOverride::Overrides overrides = ui->overrides->overrides();

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

void frmMain::on_grpSpindle_toggled(bool checked)
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

void frmMain::on_grpJog_toggled(bool checked)
{
    updateJogTitle();
    updateLayouts();

    ui->jog->setVisible(checked);
}

void frmMain::on_grpHeightMap_toggled(bool arg1)
{
    ui->widgetHeightMap->setVisible(arg1);
}

void frmMain::on_chkKeyboardControl_toggled(bool checked)
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

void frmMain::on_chkHeightMapBorderShow_toggled(bool checked)
{
    Q_UNUSED(checked)

    updateControlsState();
}

void frmMain::on_chkHeightMapInterpolationShow_toggled(bool checked)
{
    Q_UNUSED(checked)

    updateControlsState();
}

void frmMain::on_chkHeightMapUse_clicked(bool checked)
{
// //    static bool fileChanged;

//     // Reset table view
//     QByteArray headerState = ui->tblProgram->horizontalHeader()->saveState();
//     ui->tblProgram->setModel(NULL);

//     CancelException cancel;

//     if (checked) try {

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
//     }
//     catch (CancelException) {                       // Cancel modification
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
//     }

//     // Update groupbox title
//     ui->grpHeightMap->setProperty("overrided", checked);
//     style()->unpolish(ui->grpHeightMap);
//     ui->grpHeightMap->ensurePolished();

//     // Update menu
//     ui->actFileSaveTransformedAs->setVisible(checked);
}

void frmMain::on_chkHeightMapGridShow_toggled(bool checked)
{
    Q_UNUSED(checked)

    updateControlsState();
}

void frmMain::on_txtHeightMapBorderX_valueChanged(double arg1)
{
    updateHeightmapBorderDrawer();
    updateHeightmapGrid(arg1);
}

void frmMain::on_txtHeightMapBorderWidth_valueChanged(double arg1)
{
    updateHeightmapBorderDrawer();
    updateHeightmapGrid(arg1);
}

void frmMain::on_txtHeightMapBorderY_valueChanged(double arg1)
{
    updateHeightmapBorderDrawer();
    updateHeightmapGrid(arg1);
}

void frmMain::on_txtHeightMapBorderHeight_valueChanged(double arg1)
{
    updateHeightmapBorderDrawer();
    updateHeightmapGrid(arg1);
}

void frmMain::on_txtHeightMapGridX_valueChanged(double arg1)
{
    updateHeightmapGrid(arg1);
}

void frmMain::on_txtHeightMapGridY_valueChanged(double arg1)
{
    updateHeightmapGrid(arg1);
}

void frmMain::on_txtHeightMapGridZBottom_valueChanged(double arg1)
{
    updateHeightmapGrid(arg1);
}

void frmMain::on_txtHeightMapGridZTop_valueChanged(double arg1)
{
    updateHeightmapGrid(arg1);
}

void frmMain::on_txtHeightMapInterpolationStepX_valueChanged(double arg1)
{
    Q_UNUSED(arg1)

    updateHeightMapInterpolationDrawer();
}

void frmMain::on_txtHeightMapInterpolationStepY_valueChanged(double arg1)
{
    Q_UNUSED(arg1)

    updateHeightMapInterpolationDrawer();
}

void frmMain::on_cmdHeightMapMode_toggled(bool checked)
{
    // Update flag
    m_heightmapMode = checked;

    // Reset file progress
    m_program.reset();
    m_lastDrawnLineIndex = 0;

    // Reset/restore g-code program modification on edit mode enter/exit
    if (ui->chkHeightMapUse->isChecked()) {
        on_chkHeightMapUse_clicked(!checked); // Update gcode program parser
    }

    if (checked) {
        ui->tblProgram->setModel(&m_probeModel);
        resizeTableHeightmapSections();
        //updateCurrentModel(&m_programModel);
        m_currentDrawer = m_probeDrawer;
        updateParser();  // Update probe program parser
    } else {
        m_probeParser.reset();
        if (!ui->chkHeightMapUse->isChecked()) {
            ui->tblProgram->setModel(&m_programModel);
            connect(ui->tblProgram->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onTableCurrentChanged(QModelIndex,QModelIndex)));
            ui->tblProgram->selectRow(0);

            resizeTableHeightmapSections();
            // updateCurrentModel(&m_programModel);
            m_currentDrawer = m_codeDrawer;

            if (!ui->chkHeightMapUse->isChecked()) {
                ui->glwVisualizer->updateExtremes(m_codeDrawer);
//                updateProgramEstimatedTime(m_currentDrawer->viewParser()->getLineSegmentList());
            }
        }
    }

    // Shadow toolpath
    QList<LineSegment>& list = m_viewParser.getLineSegmentList();
    QList<int> indexes;
    for (int i = m_lastDrawnLineIndex; i < list.count(); i++) {
        list[i].setDrawn(checked);
        list[i].setIsHightlight(false);
        indexes.append(i);
    }
    // Update only vertex color.
    // If chkHeightMapUse was checked codeDrawer updated via updateParser
    if (!ui->chkHeightMapUse->isChecked()) m_codeDrawer->update(indexes);

    updateRecentFilesMenu();
    updateControlsState();
}

void frmMain::on_cmdHeightMapCreate_clicked()
{
    ui->cmdHeightMapMode->setChecked(true);
    on_actFileNew_triggered();
}

void frmMain::on_cmdHeightMapLoad_clicked()
{
    if (!saveChanges(true)) {
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), lastWorkingDirectory(), tr("Heightmap files (*.map)"));

    if (fileName != "") {
        addRecentHeightmap(fileName);
        loadHeightmap(fileName);

        // If using heightmap
        if (ui->chkHeightMapUse->isChecked() && !m_heightmapMode) {
            // Restore original file
            on_chkHeightMapUse_clicked(false);
            // Apply heightmap
            on_chkHeightMapUse_clicked(true);
        }

        updateRecentFilesMenu();
        updateControlsState(); // Enable 'cmdHeightMapMode' button
    }
}

void frmMain::on_cmdHeightMapBorderAuto_clicked()
{
    QRectF rect = borderRectFromExtremes();

    if (!qIsNaN(rect.width()) && !qIsNaN(rect.height())) {
        ui->txtHeightMapBorderX->setValue(rect.x());
        ui->txtHeightMapBorderY->setValue(rect.y());
        ui->txtHeightMapBorderWidth->setValue(rect.width());
        ui->txtHeightMapBorderHeight->setValue(rect.height());
    }
}

void frmMain::on_tblProgram_customContextMenuRequested(const QPoint &pos)
{
    if (m_communicator->senderState() != SenderState::Stopped) return;

    if (ui->tblProgram->selectionModel()->selectedRows().count() > 0) {
        m_tableMenu->actions().at(0)->setEnabled(true);
        m_tableMenu->actions().at(1)->setEnabled(ui->tblProgram->selectionModel()->selectedRows()[0].row() != m_currentModel->rowCount() - 1);
    } else {
        m_tableMenu->actions().at(0)->setEnabled(false);
        m_tableMenu->actions().at(1)->setEnabled(false);
    }
    m_tableMenu->popup(ui->tblProgram->viewport()->mapToGlobal(pos));
}

void frmMain::on_menuViewWindows_aboutToShow()
{
    QAction *a;
    QList<QAction*> al;

    foreach (QDockWidget *d, findChildren<QDockWidget*>()) {
        a = new QAction(d->windowTitle(), ui->menuViewWindows);
        a->setCheckable(true);
        a->setChecked(d->isVisible());
        connect(a, &QAction::triggered, d, &QDockWidget::setVisible);
        al.append(a);
    }

    std::sort(al.begin(), al.end(), frmMain::actionTextLessThan);

    ui->menuViewWindows->clear();
    ui->menuViewWindows->addActions(al);
}

void frmMain::on_menuViewPanels_aboutToShow()
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

void frmMain::on_dockVisualizer_visibilityChanged(bool visible)
{
    ui->glwVisualizer->setUpdatesEnabled(visible);
}

void frmMain::onConnectionError(QString error)
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

void frmMain::onMachinePosChanged(QVector3D pos)
{
    ui->state->setMachineCoordinates(pos);
}

void frmMain::onWorkPosChanged(QVector3D pos)
{
    ui->state->setWorkCoordinates(pos);
}

void frmMain::onMachineStateChanged(MachineState state)
{
    ui->state->setState(state);

    ui->control->updateControlsState(m_communicator->senderState(), state);

    // ui->spindle->...
    // ui->cmdSpindle->setEnabled(state == DeviceHold0 || ((m_communicator->senderState() != SenderTransferring) &&                                                        (m_communicator->senderState() != SenderStopping)));
}

void frmMain::onMachineStateReceived(MachineState state)
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
        ui->glwVisualizer->setSpendTime(time);
    }

    updateControlsState();
}

void frmMain::onSenderStateReceived(SenderState state)
{
    Q_UNUSED(state);

    updateControlsState();
}

void frmMain::onSpindleStateReceived(bool state)
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

void frmMain::onFloodStateReceived(bool state)
{
    ui->control->setFlood(state);
}

void frmMain::onParserStateReceived(QString state)
{
    ui->glwVisualizer->setParserState(state);
}

void frmMain::onPinStateReceived(QString state)
{
    ui->glwVisualizer->setPinState(state);
}

void frmMain::onFeedSpindleSpeedReceived(int feedRate, int spindleSpeed)
{
    ui->glwVisualizer->setSpeedState((QString(tr("F/S: %1 / %2")).arg(feedRate, spindleSpeed)));
}

void frmMain::onSpindleSpeedReceived(int spindleSpeed)
{
    // ui->slbSpindle->setCurrentValue(spindleSpeed);
}

void frmMain::onOverridesReceived(int feedOverride, int spindleOverride, int rapidOverride)
{
    updateOverride(ui->overrides->ui->slbFeed, feedOverride, '\x91');
    updateOverride(ui->overrides->ui->slbSpindle, spindleOverride, '\x9a');

    partMainOverride::Overrides overrides = ui->overrides->overrides();

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

void frmMain::onAborted()
{
    //ui->cmdFileAbort->setEnabled(false);
    updateControlsState();
}

void frmMain::onResponseReceived(QString command, int tableIndex, QString response)
{
    Q_UNUSED(command)
    Q_UNUSED(tableIndex)
    Q_UNUSED(response)

    updateToolpathShadowingOnCheckMode();
}

void frmMain::onCommandResponseReceived(CommandAttributes commandAttributes)
{
    ui->console->appendResponse(commandAttributes);
}

void frmMain::onCommandSent(CommandAttributes commandAttributes)
{
    ui->console->appendFiltered(commandAttributes);
}

void frmMain::onCommandProcessed(int tableIndex, QString response)
{
    if (ui->chkAutoScrollGCode->isChecked()) {
        // scroll to NEXT command (+1)
        ui->tblProgram->scrollTo(m_currentModel->index(tableIndex + 1, 0));      // TODO: Update by timer
        ui->tblProgram->setCurrentIndex(m_currentModel->index(tableIndex + 1, 1));
    }

    if (tableIndex > -1) {
        m_currentModel->setData(m_currentModel->index(tableIndex, 2), GCodeItem::Processed);
        m_currentModel->setData(m_currentModel->index(tableIndex, 3), response);
    }
}

void frmMain::onConfigurationReceived(PhysicalMachineConfiguration configuration)
{
    ui->state->setUnits(configuration.units());
}

void frmMain::onToolPositionReceived(QVector3D pos)
{
    updateToolPositionAndToolpathShadowing(pos);
}

void frmMain::onConsoleNewCommand(QString command)
{
    m_communicator->sendCommand(CommandSource::Console, command, TABLE_INDEX_UI);
}

void frmMain::onStateBehaviorChanged(StateBehavior *sb)
{
    ui->state->setStatusText(sb->name(), "black", "white");
    ui->console->appendSystem(QString("State: %1").arg(sb->name()));
}

void frmMain::onTimerConnection()
{
    // /openPortIfNeeded();

    // @TODO move it completely to communicator
    m_communicator->processConnectionTimer();
}

// @todo another way to update visualizer??
// void frmMain::onTimerStateQuery()
// {
//     if (m_connection->isConnected() && m_communicator->m_resetCompleted && m_communicator->m_statusReceived) {
//         m_connection->sendByteArray(QByteArray(1, '?'));
//         m_communicator->m_statusReceived = false;
//     }

//     ui->glwVisualizer->setBufferState(QString(tr("Buffer: %1 / %2 / %3")).arg(bufferLength()).arg(m_communicator->m_commands.length()).arg(m_communicator->m_queue.length()));
// }

void frmMain::onTableInsertLine()
{
    if (ui->tblProgram->selectionModel()->selectedRows().count() == 0 ||
        (m_communicator->senderState() == SenderState::Transferring) || (m_communicator->senderState() == SenderState::Stopping)) return;

    int row = ui->tblProgram->selectionModel()->selectedRows()[0].row();

    m_currentModel->insertRow(row);
    m_currentModel->setData(m_currentModel->index(row, 2), GCodeItem::InQueue);

    updateParser();

    ui->tblProgram->selectRow(row);
}

void frmMain::onTableDeleteLines()
{
    if (ui->tblProgram->selectionModel()->selectedRows().count() == 0 ||
        (m_communicator->senderState() == SenderState::Transferring) || (m_communicator->senderState() == SenderState::Stopping) ||
        QMessageBox::warning(this, this->windowTitle(), tr("Delete lines?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    QModelIndex firstRow = ui->tblProgram->selectionModel()->selectedRows()[0];
    int rowsCount = ui->tblProgram->selectionModel()->selectedRows().count();
    if (ui->tblProgram->selectionModel()->selectedRows().last().row() == m_currentModel->rowCount() - 1) rowsCount--;

    if (firstRow.row() != m_currentModel->rowCount() - 1) {
        m_currentModel->removeRows(firstRow.row(), rowsCount);
    } else return;

    // Drop heightmap cache
    if (m_currentModel == &m_programModel) m_programHeightmapModel.clear();

    updateParser();

    ui->tblProgram->selectRow(firstRow.row());
}

void frmMain::onTableCellChanged(QModelIndex i1, QModelIndex i2)
{
    Q_UNUSED(i2)

    GCodeTableModel *model = (GCodeTableModel*)sender();

    if (i1.column() != 1) return;
    // Inserting new line at end
    if (i1.row() == (model->rowCount() - 1) && model->data(model->index(i1.row(), 1)).toString() != "") {
        model->setData(model->index(model->rowCount() - 1, 2), GCodeItem::InQueue);
        model->insertRow(model->rowCount());
        if (!m_programLoading) ui->tblProgram->setCurrentIndex(model->index(i1.row() + 1, 1));
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

void frmMain::onTableCurrentChanged(QModelIndex currentIndex, QModelIndex previousIndex)
{
    if (m_currentProgram->empty()) {
        return;
    }

    int rowCurrent = qMin(currentIndex.row(), m_currentProgram->lastCommandIndex());
    int rowPrevious = qMax(qMin(previousIndex.row(), m_currentProgram->lastCommandIndex()), 0);

    // Update toolpath hightlighting
    GCodeViewParser *parser = m_currentDrawer->viewParser();
    QList<LineSegment>& list = parser->getLineSegmentList();
    QVector<QList<int>> lineIndexes = parser->getLinesIndexes();

    // Update linesegments on cell changed
    if (!m_currentDrawer->geometryUpdated()) {
        int lineCurrent = (*m_currentProgram)[rowCurrent].lineNumber;
        for (int i = 0; i < list.count(); i++) {
            list[i].setIsHightlight(list[i].getLineNumber() <= lineCurrent);
        }
    // Update vertices on current cell changed
    } else {
        int lineCurrent = (*m_currentProgram)[rowCurrent].lineNumber;
        int linePrevious = (*m_currentProgram)[rowPrevious].lineNumber;
        if (linePrevious < lineCurrent) qSwap(linePrevious, lineCurrent);

        QList<int> indexes;
        for (int i = lineCurrent + 1; i <= linePrevious; i++) {
            foreach (int l, lineIndexes.at(i)) {
                list[l].setIsHightlight(rowCurrent > rowPrevious);
                indexes.append(l);
            }
        }

        m_selectionDrawer.setEndPosition(indexes.isEmpty() ? QVector3D(sNan, sNan, sNan) :
            (m_configuration.visualizerModule().ignoreZ() ? QVector3D(list[indexes.last()].getEnd().x(), list[indexes.last()].getEnd().y(), 0)
                                        : list[indexes.last()].getEnd()));
        m_selectionDrawer.update();

        if (!indexes.isEmpty()) m_currentDrawer->update(indexes);
    }

    // Update selection marker
    int line = (*m_currentProgram)[rowCurrent].lineNumber;
    if (line > 0 && line < lineIndexes.count() && !lineIndexes.at(line).isEmpty()) {
        QVector3D pos = list[lineIndexes.at(line).last()].getEnd();
        m_selectionDrawer.setEndPosition(m_configuration.visualizerModule().ignoreZ() ? QVector3D(pos.x(), pos.y(), 0) : pos);
    } else {
        m_selectionDrawer.setEndPosition(QVector3D(sNan, sNan, sNan));
    }
    m_selectionDrawer.update();
}

void frmMain::onOverridingToggled(bool checked)
{
    Q_UNUSED(checked)

    partMainOverride::Overrides overrides = ui->overrides->overrides();

    ui->grpOverriding->setProperty("overrided", overrides.feedOverridden | overrides.rapidOverridden | overrides.spindleOverridden);
    style()->unpolish(ui->grpOverriding);
    ui->grpOverriding->ensurePolished();
}

void frmMain::onOverrideChanged()
{
//    updateProgramEstimatedTime(m_currentDrawer->viewParser()->getLineSegmentList());
}

void frmMain::onActRecentFileTriggered()
{
    QAction *action = static_cast<QAction*>(sender());
    QString fileName = action->text();

    if (action != NULL) {
        if (!saveChanges(m_heightmapMode)) return;
        if (!m_heightmapMode) loadFile(fileName); else loadHeightmap(fileName);
    }
}

void frmMain::onActSendFromLineTriggered()
{
    if (m_currentModel->rowCount() == 1) return;

    //Line to start from
    int commandIndex = ui->tblProgram->currentIndex().row();

    // Set parser state
    if (m_configuration.senderModule().setParserStateBeforeSendingFromSelectedLine()) {
        QString commands = getLineInitCommands(commandIndex);

        QMessageBox box(this);
        box.setIcon(QMessageBox::Information);
        box.setText(tr("Following commands will be sent before selected line:\n") + commands);
        box.setWindowTitle(qApp->applicationDisplayName());
        box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        box.addButton(tr("Skip"), QMessageBox::DestructiveRole);

        int res = box.exec();
        if (res == QMessageBox::Cancel) return;
        else if (res == QMessageBox::Ok) {
            // foreach (QString command, commands) {
            //     sendCommand(command, COMMAND_TI_UI);
            // }
            m_communicator->sendCommands(CommandSource::ProgramAdditionalCommands, commands, TABLE_INDEX_UI);
        }
    }

    m_program.reset(commandIndex);
    m_lastDrawnLineIndex = 0;
    m_communicator->m_probeIndex = -1;

    QList<LineSegment>& list = m_viewParser.getLineSegmentList();

    QList<int> indexes;
    for (int i = 0; i < list.count(); i++) {
        list[i].setDrawn(list[i].getLineNumber() < (*m_currentProgram)[commandIndex].lineNumber);
        indexes.append(i);
    }
    m_codeDrawer->update(indexes);

    ui->tblProgram->setUpdatesEnabled(false);

    for (int i = 0; i < m_currentProgram->count() - 1; i++) {
        (*m_currentProgram)[i].state = i < commandIndex ? GCodeItem::Skipped : GCodeItem::InQueue;
        (*m_currentProgram)[i].response = QString();
    }
    ui->tblProgram->setUpdatesEnabled(true);
    ui->glwVisualizer->setSpendTime(QTime(0, 0, 0));

    m_startTime = QDateTime::currentSecsSinceEpoch();

    m_communicator->setSenderStateAndEmitSignal(SenderState::Transferring);

    ui->jog->storeAndResetKeyboardControl();
    // m_storedKeyboardControl = ui->chkKeyboardControl->isChecked();
    // ui->chkKeyboardControl->setChecked(false);

    m_communicator->storeParserState();

#ifdef WINDOWS
    // if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7) {
    //     if (m_taskBarProgress) {
    //         m_taskBarProgress->setMaximum(m_currentModel->rowCount() - 2);
    //         m_taskBarProgress->setValue(commandIndex);
    //         m_taskBarProgress->show();
    //     }
    // }
#endif

    updateControlsState();
    ui->cmdFilePause->setFocus();

    m_program.reset(commandIndex);
    // m_communicator->sendStreamerCommandsUntilBufferIsFull();
}

void frmMain::onSlbSpindleValueUserChanged()
{
    m_communicator->m_updateSpindleSpeed = true;
}

void frmMain::onSlbSpindleValueChanged()
{
    // if (!ui->grpSpindle->isChecked() && ui->cmdSpindle->isChecked())
    //     ui->grpSpindle->setTitle(tr("Spindle") + QString(tr(" (%1)")).arg(ui->slbSpindle->value()));
}

// void frmMain::onCboCommandReturnPressed()
// {
//     QString command = ui->cboCommand->currentText();
//     if (command.isEmpty()) return;

//     ui->cboCommand->setCurrentText("");
//     m_communicator->sendCommand(command, COMMAND_TI_UI);
// }

void frmMain::onDockTopLevelChanged(bool topLevel)
{
    Q_UNUSED(topLevel)
    static_cast<QWidget*>(sender())->setStyleSheet("");
}

void frmMain::onScroolBarAction(int action)
{
    Q_UNUSED(action)

    if ((m_communicator->senderState() == SenderState::Transferring) || (m_communicator->senderState() == SenderState::Stopping))
        ui->chkAutoScrollGCode->setChecked(false);
}

void frmMain::onVisualizerCursorPosChanged(QPointF pos)
{
    m_cursorDrawer.setPosition(pos);
}

void frmMain::updateHeightMapInterpolationDrawer(bool reset)
{
    if (m_settingsLoading) return;

    QRectF borderRect = borderRectFromTextboxes();
    m_heightmapInterpolationDrawer.setBorderRect(borderRect);

    QVector<QVector<double>> *interpolationData = new QVector<QVector<double>>;

    int interpolationPointsX = ui->txtHeightMapInterpolationStepX->value();// * (ui->txtHeightMapGridX->value() - 1) + 1;
    int interpolationPointsY = ui->txtHeightMapInterpolationStepY->value();// * (ui->txtHeightMapGridY->value() - 1) + 1;

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

    if (m_heightmapInterpolationDrawer.data() != NULL) {
        delete m_heightmapInterpolationDrawer.data();
    }
    m_heightmapInterpolationDrawer.setData(interpolationData);

    // Update grid drawer
    m_heightmapGridDrawer.update();

    // Heightmap changed by table user input
    if (sender() == &m_heightmapModel) m_heightmapChanged = true;

    // Reset heightmapped program model
    m_programHeightmapModel.clear();
}

void frmMain::placeVisualizerButtons()
{
    ui->cmdToggleProjection->setParent(ui->glwVisualizer);
    ui->cmdFit->setParent(ui->glwVisualizer);
    ui->cmdIsometric->setParent(ui->glwVisualizer);
    ui->cmdTop->setParent(ui->glwVisualizer);
    ui->cmdFront->setParent(ui->glwVisualizer);
    ui->cmdLeft->setParent(ui->glwVisualizer);
    ui->cmdRotationCube->setParent(ui->glwVisualizer);

    int w = ui->glwVisualizer->width();
    ui->cmdIsometric->move(w - ui->cmdIsometric->width() - 8, 8);
    ui->cmdTop->move(ui->cmdIsometric->geometry().left() - ui->cmdTop->width() - 8, 8);
    ui->cmdLeft->move(w - ui->cmdLeft->width() - 8, ui->cmdIsometric->geometry().bottom() + 8);
    ui->cmdFront->move(ui->cmdLeft->geometry().left() - ui->cmdFront->width() - 8, ui->cmdIsometric->geometry().bottom() + 8);
    ui->cmdFit->move(w - ui->cmdFit->width() - 8, ui->cmdLeft->geometry().bottom() + 8);
    ui->cmdToggleProjection->move(ui->cmdFit->geometry().left() - ui->cmdToggleProjection->width() - 8, ui->cmdLeft->geometry().bottom() + 8);
    ui->cmdRotationCube->move(w - ui->cmdRotationCube->width() - 8, ui->cmdFit->geometry().bottom() + 8);
}

void frmMain::preloadSettings()
{
    ConfigurationUI &uiConfiguration = m_configuration.uiModule();
    ConfigurationVisualizer &visualizerConfiguration = m_configuration.visualizerModule();

    qApp->setStyleSheet(
        QString(qApp->styleSheet()).replace(QRegularExpression("font-size:[^;^\\}]+"), QString("font-size: %1pt").arg(uiConfiguration.fontSize()))
    );

    // Update v-sync in glformat
    // QGLFormat fmt = QGLFormat::defaultFormat();
    // fmt.setSwapInterval(visualizerConfiguration.vsync() ? 1 : 0);
    // QGLFormat::setDefaultFormat(fmt);
}

void frmMain::applyHeightmapConfiguration(ConfigurationHeightmap &configurationHeightmap)
{
    ui->txtHeightMapBorderX->setValue(configurationHeightmap.borderX());
    ui->txtHeightMapBorderY->setValue(configurationHeightmap.borderY());
    ui->txtHeightMapBorderWidth->setValue(configurationHeightmap.borderWidth());
    ui->txtHeightMapBorderHeight->setValue(configurationHeightmap.borderHeight());
    ui->chkHeightMapBorderShow->setChecked(configurationHeightmap.borderShow());

    ui->txtHeightMapGridX->setValue(configurationHeightmap.gridX());
    ui->txtHeightMapGridY->setValue(configurationHeightmap.gridY());
    ui->txtHeightMapGridZTop->setValue(configurationHeightmap.gridZTop());
    ui->txtHeightMapGridZBottom->setValue(configurationHeightmap.gridZBottom());
    ui->txtHeightMapProbeFeed->setValue(configurationHeightmap.probeFeed());
    ui->chkHeightMapGridShow->setChecked(configurationHeightmap.gridShow());

    ui->txtHeightMapInterpolationStepX->setValue(configurationHeightmap.interpolationStepX());
    ui->txtHeightMapInterpolationStepY->setValue(configurationHeightmap.interpolationStepY());
    ui->cboHeightMapInterpolationType->setCurrentIndex(configurationHeightmap.interpolationType());
    ui->chkHeightMapInterpolationShow->setChecked(configurationHeightmap.interpolationShow());
}

void frmMain::applyOverridesConfiguration(ConfigurationMachine &machineConfiguration)
{
    // ui->slbFeedOverride->setChecked(machineConfiguration.overrideFeed());
    // ui->slbFeedOverride->setValue(machineConfiguration.overrideFeedValue());

    // ui->slbRapid->setChecked(machineConfiguration.overrideRapid());
    // ui->slbRapid->setValue(machineConfiguration.overrideRapidValue());

    // ui->slbSpindle->setChecked(machineConfiguration.overrideSpindleSpeed());
    // ui->slbSpindle->setValue(machineConfiguration.overrideSpindleSpeedValue());
}

void frmMain::applySpindleConfiguration(ConfigurationMachine &machineConfiguration)
{
    // ui->slbSpindle->setRatio(machineConfiguration.spindleSpeedRatio());
    // ui->slbSpindle->setMinimum(machineConfiguration.spindleSpeedRange().min);
    // ui->slbSpindle->setMaximum(machineConfiguration.spindleSpeedRange().max);
    // ui->slbSpindle->setValue(machineConfiguration.spindleSpeed());
}

void frmMain::applyRecentFilesConfiguration(ConfigurationUI &uiConfiguration)
{
    updateRecentFilesMenu();
}

void frmMain::loadSettings()
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
    ShortcutsMap m;
    QByteArray ba = set.value("shortcuts").toByteArray();
    QDataStream s(&ba, QIODevice::ReadOnly);

    s >> m;
    for (int i = 0; i < m.count(); i++) {
        QAction *a = findChild<QAction*>(m.keys().at(i));
        if (a) a->setShortcuts(m.values().at(i));
    }

    // Menu
    ConfigurationUI &uiConfiguration = m_configuration.uiModule();
    ui->actViewLockWindows->setChecked(uiConfiguration.lockWindows());
    ui->actViewLockPanels->setChecked(uiConfiguration.lockPanels());
    // @TODO move to configuration form
    //m_settings->restoreGeometry(set.value("formSettingsGeometry", m_settings->saveGeometry()).toByteArray());

    m_settingsLoading = false;

    emit settingsLoaded();
}

void frmMain::restoreDockableLayoutState()
{
    QSettings set(m_settingsFileName, QSettings::IniFormat);

    ui->tblProgram->horizontalHeader()->restoreState(set.value("header", QByteArray()).toByteArray());

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
        // connect(w, &QDockWidget::topLevelChanged, this, &frmMain::onDockTopLevelChanged);
    }

    // Panels
    ui->scrollContentsDevice->restoreState(this, set.value("panelsDevice").toStringList());
    ui->scrollContentsModification->restoreState(this, set.value("panelsModification").toStringList());
    ui->scrollContentsUser->restoreState(this, set.value("panelsUser").toStringList());

    QStringList hiddenPanels = set.value("hiddenPanels").toStringList();
    foreach (QString s, hiddenPanels) {
        QGroupBox *b = findChild<QGroupBox*>(s);
        if (b) b->setHidden(true);
    }

    QStringList collapsedPanels = set.value("collapsedPanels").toStringList();
    foreach (QString s, collapsedPanels) {
        QGroupBox *b = findChild<QGroupBox*>(s);
        if (b) b->setChecked(false);
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

void frmMain::saveSettings()
{
    QSettings set(m_settingsFileName, QSettings::IniFormat);

    emit settingsAboutToSave();

    ConfigurationUI &uiConfiguration = m_configuration.uiModule();
    // ConfigurationJogging &joggingConfiguration = m_configuration.joggingModule();

    // m_configuration.machineModule().setSpindleSpeed(ui->slbSpindle->value());
    uiConfiguration.setAutoScrollGCode(ui->chkAutoScrollGCode->isChecked());

    set.setValue("header", ui->tblProgram->horizontalHeader()->saveState());
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
    set.setValue("panelsDevice", QVariant::fromValue(ui->scrollContentsDevice->saveState()));
    set.setValue("panelsModification", QVariant::fromValue(ui->scrollContentsModification->saveState()));
    set.setValue("panelsUser", QVariant::fromValue(ui->scrollContentsUser->saveState()));

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
    set.setValue("hiddenPanels", hiddenPanels);
    set.setValue("collapsedPanels", collapsedPanels);

    // Menu
    // set.setValue("lockWindows", ui->actViewLockWindows->isChecked());
    // set.setValue("lockPanels", ui->actViewLockPanels->isChecked());
    uiConfiguration.setLockPanels(ui->actViewLockPanels->isChecked());
    uiConfiguration.setLockWindows(ui->actViewLockWindows->isChecked());

    m_configuration.save();

    emit settingsSaved();
}

void frmMain::initializeConnection(ConfigurationConnection::ConnectionMode mode)
{
    m_connection = m_connectionManager.createConnection(mode);

    //connect(m_connection, SIGNAL(lineReceived(QString)), this, SLOT(onConnectionLineReceived(QString)));
    connect(m_connection, SIGNAL(error(QString)), this, SLOT(onConnectionError(QString)));
}

void frmMain::applyVisualizerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
{
    ui->glwVisualizer->setLineWidth(visualizerConfiguration.lineWidth());
    ui->glwVisualizer->setAntialiasing(visualizerConfiguration.antialiasing());
    ui->glwVisualizer->setMsaa(visualizerConfiguration.msaa());
    ui->glwVisualizer->setZBuffer(visualizerConfiguration.zBuffer());
    ui->glwVisualizer->setFov(visualizerConfiguration.fieldOfView());
    ui->glwVisualizer->setNearPlane(visualizerConfiguration.nearPlane());
    ui->glwVisualizer->setFarPlane(visualizerConfiguration.farPlane());
    ui->glwVisualizer->setVsync(visualizerConfiguration.vsync());
    ui->glwVisualizer->setFps(visualizerConfiguration.fpsLock());
    ui->glwVisualizer->setColorBackground(visualizerConfiguration.backgroundColor());
    ui->glwVisualizer->setColorText(visualizerConfiguration.textColor());

    // Adapt visualizer buttons colors
    const int LIGHTBOUND = 127;
    const int NORMALSHIFT = 40;
    const int HIGHLIGHTSHIFT = 80;

    QColor base = visualizerConfiguration.backgroundColor();
    bool light = base.value() > LIGHTBOUND;

    ui->cmdToggleProjection->setIcon(QIcon(":/images/toggle.png"));
    ui->cmdFit->setIcon(QIcon(":/images/fit_1.png"));
    ui->cmdIsometric->setIcon(QIcon(":/images/cube.png"));
    ui->cmdFront->setIcon(QIcon(":/images/visualizer_front.png"));
    ui->cmdLeft->setIcon(QIcon(":/images/visualizer_left.png"));
    ui->cmdTop->setIcon(QIcon(":/images/visualizer_top.png"));

    if (!light) {
        Utils::invertButtonIconColors(ui->cmdToggleProjection);
        Utils::invertButtonIconColors(ui->cmdFit);
        Utils::invertButtonIconColors(ui->cmdIsometric);
        Utils::invertButtonIconColors(ui->cmdFront);
        Utils::invertButtonIconColors(ui->cmdLeft);
        Utils::invertButtonIconColors(ui->cmdTop);
    }

    QColor normal, highlight;

    normal.setHsv(base.hue(), base.saturation(), base.value() + (light ? -NORMALSHIFT : NORMALSHIFT));
    highlight.setHsv(base.hue(), base.saturation(), base.value() + (light ? -HIGHLIGHTSHIFT : HIGHLIGHTSHIFT));

    ui->glwVisualizer->setStyleSheet(QString("QToolButton {border: 1px solid %1; \
                background-color: %3} QToolButton:hover {border: 1px solid %2;}")
                .arg(normal.name()).arg(highlight.name())
                .arg(base.name()));

    m_cursorDrawer.setVisible(visualizerConfiguration.show3dCursor());
}

void frmMain::applyCodeDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
{
    m_codeDrawer->setLineWidth(visualizerConfiguration.lineWidth());
    m_codeDrawer->setSimplify(visualizerConfiguration.simplifyGeometry());
    m_codeDrawer->setSimplifyPrecision(visualizerConfiguration.simplifyGeometryPrecision());
    m_codeDrawer->setColorNormal(visualizerConfiguration.normalToolpathColor());
    m_codeDrawer->setColorDrawn(visualizerConfiguration.drawnToolpathColor());
    m_codeDrawer->setColorHighlight(visualizerConfiguration.hightlightToolpathColor());
    m_codeDrawer->setColorZMovement(visualizerConfiguration.zMovementColor());
    m_codeDrawer->setColorRapidMovement(visualizerConfiguration.rapidMovementColor());
    m_codeDrawer->setColorStart(visualizerConfiguration.startPointColor());
    m_codeDrawer->setColorEnd(visualizerConfiguration.endPointColor());
    m_codeDrawer->setIgnoreZ(visualizerConfiguration.ignoreZ());
    m_codeDrawer->setGrayscaleSegments(visualizerConfiguration.grayscaleSegments());
    m_codeDrawer->setGrayscaleCode(visualizerConfiguration.grayscaleSegmentsBySCode() ? GcodeDrawer::S : GcodeDrawer::Z);
    m_codeDrawer->setGrayscaleMin(m_configuration.machineModule().laserPowerRange().min);
    m_codeDrawer->setGrayscaleMax(m_configuration.machineModule().laserPowerRange().max);
    m_codeDrawer->update();
}

void frmMain::applyToolDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
{
    m_toolDrawer.setToolDiameter(visualizerConfiguration.toolDiameter());
    m_toolDrawer.setToolLength(visualizerConfiguration.toolLength());
    m_toolDrawer.setLineWidth(visualizerConfiguration.lineWidth());
    m_toolDrawer.setToolAngle(visualizerConfiguration.toolType() == ConfigurationVisualizer::ToolType::Conic ? 180 : visualizerConfiguration.toolAngle());
    m_toolDrawer.setColor(visualizerConfiguration.toolColor());
    m_toolDrawer.update();
}

void frmMain::applyTableSurfaceDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
{
    m_tableSurfaceDrawer.setGridColor(visualizerConfiguration.tableSurfaceGridColor());
    m_tableSurfaceDrawer.update();
}

void frmMain::applyHeightmapDrawerConfiguration(ConfigurationVisualizer &visualizerConfiguration)
{
    m_heightmapBorderDrawer.setLineWidth(visualizerConfiguration.lineWidth());
    m_heightmapGridDrawer.setLineWidth(0.1);
    m_heightmapInterpolationDrawer.setLineWidth(visualizerConfiguration.lineWidth());
}

void frmMain::applyUIConfiguration(ConfigurationUI &uiConfiguration)
{
    ui->chkAutoScrollGCode->setChecked(uiConfiguration.autoScrollGCode());
    qApp->setStyleSheet(
        QString(qApp->styleSheet()).replace(QRegularExpression("font-size:[^;^\\}]+"), QString("font-size: %1pt").arg(uiConfiguration.fontSize()))
    );
}

void frmMain::applyJoggingConfiguration(ConfigurationJogging &joggingConfiguration)
{
    ui->jog->configurationUpdated();
}

void frmMain::appendPanel(DropWidget *dockPanel, const QString name, const QString title, QWidget *panel)
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

void frmMain::appendSpacer(DropWidget *dockPanel)
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

void frmMain::addWindow(const QString title, QWidget *window, Qt::DockWidgetArea area, Qt::Orientation orientation)
{
    QDockWidget *dock = new QDockWidget(tr(title.toStdString().c_str()));
    dock->setMinimumHeight(200);
    dock->setObjectName("Camera");
    dock->setWidget(window);
    Utils::setDockableLocked(dock, m_configuration.uiModule().lockWindows());
    addDockWidget(area, dock, orientation);
}

void frmMain::applySettings()
{
    ConfigurationVisualizer &visualizerConfiguration = m_configuration.visualizerModule();
    ConfigurationHeightmap &heightmapConfiguration = m_configuration.heightmapModule();
    ConfigurationMachine &machineConfiguration = m_configuration.machineModule();
    ConfigurationUI &uiConfiguration = m_configuration.uiModule();
    ConfigurationJogging &joggingConfiguration = m_configuration.joggingModule();

    m_originDrawer->setLineWidth(visualizerConfiguration.lineWidth());

    // @TODO watch for changes is communicator?
    m_communicator->stopUpdatingState();
    m_communicator->startUpdatingState(m_configuration.connectionModule().queryStateInterval());

    applyToolDrawerConfiguration(visualizerConfiguration);
    applyCodeDrawerConfiguration(visualizerConfiguration);
    applyVisualizerConfiguration(visualizerConfiguration);
    applyTableSurfaceDrawerConfiguration(visualizerConfiguration);
    applyHeightmapConfiguration(heightmapConfiguration);
    applyHeightmapDrawerConfiguration(visualizerConfiguration);
    applySpindleConfiguration(machineConfiguration);
    applyJoggingConfiguration(joggingConfiguration);
    applyOverridesConfiguration(machineConfiguration);
    applyUIConfiguration(uiConfiguration);
    applyRecentFilesConfiguration(uiConfiguration);

    m_selectionDrawer.setColor(visualizerConfiguration.hightlightToolpathColor());

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

// void frmMain::openPortIfNeeded()
// {
//     assert(m_communicator != nullptr);

//     if (m_connection->state() == ConnectionState::Connecting || m_connection->state() == ConnectionState::Connected) {
//         return;
//     }

//     if (m_connection->open()) {
//         ui->state->setStatusText(tr("Port opened"), "palette(button)", "palette(text)");
//     }
// }

void frmMain::updateParser()
{
    assert(m_communicator->isMachineConfigurationReady());

    GCodeViewParser *viewParse = m_currentDrawer->viewParser();

    GcodeParser parser;
    parser.setTraverseSpeed(m_communicator->machineConfiguration().maxRate().x()); // uses only x axis speed
    if (m_configuration.visualizerModule().ignoreZ()) {
        parser.reset(QVector3D(qQNaN(), qQNaN(), 0));
    }

    ui->tblProgram->setUpdatesEnabled(false);

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

    ui->tblProgram->setUpdatesEnabled(true);

    viewParse->reset();

    ConfigurationParser &configurationParser = m_configuration.parserModule();

    // updateProgramEstimatedTime(
        // viewParse->getLinesFromParser(
        //     &parser,
        //     configurationParser.arcApproximationValue(),
        //     configurationParser.arcApproximationMode() == ConfigurationParser::ParserArcApproximationMode::ByAngle
        // )
    // );
    m_currentDrawer->update();
    ui->glwVisualizer->updateExtremes(m_currentDrawer);
    updateControlsState();

    if (m_currentModel == &m_programModel) m_fileChanged = true;
}

// @TODO scripting only??
// void frmMain::storeOffsetsVars(QString response)
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

void frmMain::loadFile(QString fileName)
{
    GCodeThreadedLoader *loader = new GCodeThreadedLoader(this);
    int progressIndex = ui->console->appendProgress("Loading " + fileName);
    connect(loader, &GCodeThreadedLoader::progress, this, [this, progressIndex](int progress) {
        ui->console->setProgress(progressIndex, progress);
    });
    connect(loader, &GCodeThreadedLoader::cancelled, this, [this, loader]() {
        ui->console->appendSystem("Cancelled loading");
        loader->deleteLater();
    });
    connect(loader, &GCodeThreadedLoader::finished, this, [this, loader](GCodeLoaderData *data) {
        ui->console->appendSystem("Finished loading");
        this->applyLoaderGCode(data);
        delete data;
        loader->deleteLater();
    });

    GCodeLoaderConfiguration configuration(m_configuration.parserModule());
    loader->loadFromFile(fileName, configuration);

    // QFile file(fileName);

    // if (!file.open(QIODevice::ReadOnly)) {
    //     QMessageBox::critical(this, this->windowTitle(), tr("Can't open file:\n") + fileName);
    //     return;
    // }

    // // Set filename
    // m_programFileName = fileName;

    // // Prepare text stream
    // QTextStream textStream(&file);

    // // Read lines
    // QList<std::string> data;
    // while (!textStream.atEnd()) {
    //     data.append(textStream.readLine().toStdString());
    // }

    // qDebug() << "Lines: " << data.count();

    // loadLines(data);
}

void frmMain::applyLoaderGCode(GCodeLoaderData *data)
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
    m_currentDrawer = m_codeDrawer;
    m_viewParser = *data->viewParser;
    m_codeDrawer->update();
    ui->glwVisualizer->fitDrawable(m_codeDrawer);

    // Update interface
    ui->chkHeightMapUse->setChecked(false);
    ui->grpHeightMap->setProperty("overrided", false);
    style()->unpolish(ui->grpHeightMap);
    ui->grpHeightMap->ensurePolished();

    // Reset tableview
    QByteArray headerState = ui->tblProgram->horizontalHeader()->saveState();
    ui->tblProgram->setModel(NULL);

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
    ui->tblProgram->setModel(&m_programModel);
    ui->tblProgram->horizontalHeader()->restoreState(headerState);

    // Update tableview
    connect(ui->tblProgram->selectionModel(), &QItemSelectionModel::currentChanged, this, &frmMain::onTableCurrentChanged);
    ui->tblProgram->selectRow(0);

    //  Update code drawer
    m_codeDrawer->update();
    ui->glwVisualizer->fitDrawable(m_codeDrawer);

    resetHeightmap();
    updateControlsState();
}

void frmMain::loadLines(QList<std::string> data)
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
    m_currentDrawer = m_codeDrawer;
    m_codeDrawer->update();
    ui->glwVisualizer->fitDrawable(m_codeDrawer);

    QList<LineSegment> list;
    updateProgramEstimatedTime(list);

    // Update interface
    ui->chkHeightMapUse->setChecked(false);
    ui->grpHeightMap->setProperty("overrided", false);
    style()->unpolish(ui->grpHeightMap);
    ui->grpHeightMap->ensurePolished();

    // Reset tableview
    QByteArray headerState = ui->tblProgram->horizontalHeader()->saveState();
    ui->tblProgram->setModel(NULL);

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
    ui->tblProgram->setModel(&m_programModel);
    ui->tblProgram->horizontalHeader()->restoreState(headerState);

    // Update tableview
    connect(ui->tblProgram->selectionModel(), &QItemSelectionModel::currentChanged, this, &frmMain::onTableCurrentChanged);
    ui->tblProgram->selectRow(0);

    //  Update code drawer
    m_codeDrawer->update();
    ui->glwVisualizer->fitDrawable(m_codeDrawer);

    ui->glwVisualizer->updateDrawer(m_codeDrawer);

    // m_codeDrawer->update();
    // m_codeDrawer->updateData();
    VertexDataExporter::exportToJsFile("vertexdata.js", m_codeDrawer->lines());

    resetHeightmap();
    updateControlsState();
}

bool frmMain::saveChanges(bool heightMapMode)
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

bool frmMain::saveProgramToFile(QString fileName, GCode &model)
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

void frmMain::clearTable()
{
    m_programModel.clear();
    m_programModel.insertRow(0);
}

void frmMain::resetHeightmap()
{
    delete m_heightmapInterpolationDrawer.data();
    m_heightmapInterpolationDrawer.setData(NULL);

    ui->tblHeightMap->setModel(NULL);
    m_heightmapModel.resize(1, 1);

    ui->txtHeightMap->clear();
    m_heightmapFileName.clear();
    m_heightmapChanged = false;
}

void frmMain::newFile()
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
    m_codeDrawer->update();
    m_currentDrawer = m_codeDrawer;
    ui->glwVisualizer->fitDrawable();

    QList<LineSegment> list;
    updateProgramEstimatedTime(list);

    m_programFileName = "";
    ui->chkHeightMapUse->setChecked(false);
    ui->grpHeightMap->setProperty("overrided", false);
    style()->unpolish(ui->grpHeightMap);
    ui->grpHeightMap->ensurePolished();

    // Reset tableview
    QByteArray headerState = ui->tblProgram->horizontalHeader()->saveState();
    ui->tblProgram->setModel(NULL);

    // Set table model
    ui->tblProgram->setModel(&m_programModel);
    ui->tblProgram->horizontalHeader()->restoreState(headerState);

    // Update tableview
    connect(ui->tblProgram->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onTableCurrentChanged(QModelIndex,QModelIndex)));
    ui->tblProgram->selectRow(0);

    // Clear selection marker
    m_selectionDrawer.setEndPosition(QVector3D(sNan, sNan, sNan));
    m_selectionDrawer.update();

    resetHeightmap();

    updateControlsState();
}

void frmMain::newHeightmap()
{
    m_heightmapModel.clear();
    on_cmdFileReset_clicked();
    ui->txtHeightMap->setText(tr("Untitled"));
    m_heightmapFileName.clear();

    updateHeightmapBorderDrawer();
    updateHeightmapGrid();

    m_heightmapChanged = false;

    updateControlsState();
}

void frmMain::updateControlsState()
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
    ui->cmdFileOpen->setEnabled(senderState == SenderState::Stopped);
    ui->cmdFileReset->setEnabled((senderState == SenderState::Stopped) && m_programModel.rowCount() > 1);
    ui->cmdFileSend->setEnabled(portOpened && (senderState == SenderState::Stopped) && m_programModel.rowCount() > 1);
    switch (senderState) {
        case SenderState::Pausing:
        case SenderState::Pausing2:
            ui->cmdFilePause->setText(tr("Pausing..."));
            break;
        case SenderState::Paused:
        case SenderState::ChangingTool:
            ui->cmdFilePause->setText(tr("Resume"));
            break;
        default:
            ui->cmdFilePause->setText(tr("Pause"));
            break;
    }
    ui->cmdFilePause->setEnabled(portOpened && (process || paused) && (senderState != SenderState::Pausing) && (senderState != SenderState::Pausing2));
    ui->cmdFilePause->setChecked(paused);
    ui->cmdFileAbort->setEnabled(senderState != SenderState::Stopped && senderState != SenderState::Stopping);
    ui->menuRecent->setEnabled(
        (senderState == SenderState::Stopped) &&
        ((m_configuration.uiModule().hasAnyRecentFiles() && !m_heightmapMode) || (m_configuration.uiModule().hasAnyRecentHeightmaps() && m_heightmapMode))
    );
    ui->actFileSave->setEnabled(m_programModel.rowCount() > 1);
    ui->actFileSaveAs->setEnabled(m_programModel.rowCount() > 1);

    ui->tblProgram->setEditTriggers((senderState != SenderState::Stopped) ? QAbstractItemView::NoEditTriggers :
        QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked |
        QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);

    if (!portOpened) {
        ui->state->setStatusText(tr("Not connected"), "palette(button)", "palette(text)");
        emit machineStateChanged(-1);
    }

    this->setWindowTitle(m_programFileName.isEmpty() ? qApp->applicationDisplayName()
                                                     : m_programFileName.mid(m_programFileName.lastIndexOf("/") + 1) + " - " + qApp->applicationDisplayName());

    if (!process) ui->jog->restoreKeyboardControl();

#ifdef WINDOWS
    // if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7 && m_taskBarProgress) {
    //     m_taskBarProgress->setPaused(paused);
    //     if (m_communicator->senderState() == SenderStopped) m_taskBarProgress->hide();
    // }
#endif

    style()->unpolish(ui->cmdFileOpen);
    style()->unpolish(ui->cmdFileReset);
    style()->unpolish(ui->cmdFileSend);
    style()->unpolish(ui->cmdFilePause);
    style()->unpolish(ui->cmdFileAbort);
    ui->cmdFileOpen->ensurePolished();
    ui->cmdFileReset->ensurePolished();
    ui->cmdFileSend->ensurePolished();
    ui->cmdFilePause->ensurePolished();
    ui->cmdFileAbort->ensurePolished();

    // Heightmap
    m_heightmapBorderDrawer.setVisible(ui->chkHeightMapBorderShow->isChecked() && m_heightmapMode);
    m_heightmapGridDrawer.setVisible(ui->chkHeightMapGridShow->isChecked() && m_heightmapMode);
    m_heightmapInterpolationDrawer.setVisible(ui->chkHeightMapInterpolationShow->isChecked() && m_heightmapMode);

    ui->grpProgram->setText(m_heightmapMode ? tr("Heightmap") : tr("G-code program"));
    ui->grpProgram->setProperty("overrided", m_heightmapMode);
    style()->unpolish(ui->grpProgram);
    ui->grpProgram->ensurePolished();

    // ui->cboJogStep->setEditable(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogFeed->setEditable(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogStep->setEnabled(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogFeed->setEnabled(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogStep->setStyleSheet(QString("font-size: %1").arg(m_configuration.uiModule().fontSize()));
    // ui->cboJogFeed->setStyleSheet(ui->cboJogStep->styleSheet());

    ui->tblHeightMap->setVisible(m_heightmapMode);
    ui->tblProgram->setVisible(!m_heightmapMode);

    ui->widgetHeightMap->setEnabled(!process && m_programModel.rowCount() > 1);
    ui->cmdHeightMapMode->setEnabled(!ui->txtHeightMap->text().isEmpty());

    ui->cmdFileSend->setText(m_heightmapMode ? tr("Probe") : tr("Send"));

    ui->chkHeightMapUse->setEnabled(!m_heightmapMode && !ui->txtHeightMap->text().isEmpty());

    ui->actFileSaveTransformedAs->setVisible(ui->chkHeightMapUse->isChecked());

    ui->cmdFileSend->menu()->actions().first()->setEnabled(!ui->cmdHeightMapMode->isChecked());

    m_selectionDrawer.setVisible(!ui->cmdHeightMapMode->isChecked());
}

void frmMain::updateLayouts()
{
    this->update();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void frmMain::updateRecentFilesMenu()
{
    ui->menuRecent->clear();
    QMenu *fileOpenMenu = ui->cmdFileOpen->menu();
    fileOpenMenu->clear();

    QStringList files = !m_heightmapMode ? m_configuration.uiModule().recentFiles() : m_configuration.uiModule().recentHeightmaps();
    if (!files.empty())
    {
        QStringList::const_iterator it = files.constEnd();
        while (it != files.constBegin()) {
            --it;
            QAction *action = new QAction(*it, this);
            connect(action, &QAction::triggered, this, &frmMain::onActRecentFileTriggered);
            ui->menuRecent->addAction(action);
            fileOpenMenu->addAction(action);
        }

        ui->menuRecent->addSeparator();
        fileOpenMenu->addSeparator();

        QAction *clearAction = new QAction(tr("&Clear"), this);
        connect(clearAction, &QAction::triggered, this, &frmMain::onActRecentClearTriggered);

        ui->menuRecent->addAction(clearAction);
        fileOpenMenu->addAction(clearAction);
    }

    updateControlsState();
}

void frmMain::updateOverride(SliderBox *slider, int value, char command)
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

void frmMain::updateJogTitle()
{
    if (ui->grpJog->isChecked() || !ui->jog->keyboardControl()) {
        ui->grpJog->setTitle(tr("Jog"));
    } else if (ui->jog->keyboardControl()) {
        ui->grpJog->setTitle(tr("Jog") + QString(tr(" (%1/%2)"))
                                             .arg((ui->jog->stepSize() != JoggingContinuous) ? QString::number(ui->jog->stepSize()) : tr("C"))
                            .arg(ui->jog->feedRate()));
    }
}

void frmMain::addRecentFile(QString fileName)
{
    m_configuration.uiModule().addRecentFile(fileName);
    m_configuration.save();
}

void frmMain::addRecentHeightmap(QString fileName)
{
    m_configuration.uiModule().addRecentHeightmap(fileName);
    m_configuration.save();
}

QRectF frmMain::borderRectFromTextboxes()
{
    QRectF rect;

    rect.setX(ui->txtHeightMapBorderX->value());
    rect.setY(ui->txtHeightMapBorderY->value());
    rect.setWidth(ui->txtHeightMapBorderWidth->value());
    rect.setHeight(ui->txtHeightMapBorderHeight->value());

    return rect;
}

QRectF frmMain::borderRectFromExtremes()
{
    QRectF rect;

    rect.setX(m_codeDrawer->getMinimumExtremes().x());
    rect.setY(m_codeDrawer->getMinimumExtremes().y());
    rect.setWidth(m_codeDrawer->getSizes().x());
    rect.setHeight(m_codeDrawer->getSizes().y());

    return rect;
}

void frmMain::updateHeightmapBorderDrawer()
{
    if (m_settingsLoading) return;

    m_heightmapBorderDrawer.setBorderRect(borderRectFromTextboxes());
}

bool frmMain::updateHeightmapGrid()
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
    QRectF borderRect = borderRectFromTextboxes();
    m_heightmapGridDrawer.setBorderRect(borderRect);
    m_heightmapGridDrawer.setGridSize(QPointF(ui->txtHeightMapGridX->value(), ui->txtHeightMapGridY->value()));
    m_heightmapGridDrawer.setZBottom(ui->txtHeightMapGridZBottom->value());
    m_heightmapGridDrawer.setZTop(ui->txtHeightMapGridZTop->value());

    // Reset model
    int gridPointsX = ui->txtHeightMapGridX->value();
    int gridPointsY = ui->txtHeightMapGridY->value();

    m_heightmapModel.resize(gridPointsX, gridPointsY);
    ui->tblHeightMap->setModel(NULL);
    ui->tblHeightMap->setModel(&m_heightmapModel);
    resizeTableHeightmapSections();

    // Update interpolation
    updateHeightMapInterpolationDrawer(true);

    // Generate probe program
    double gridStepX = gridPointsX > 1 ? borderRect.width() / (gridPointsX - 1) : 0;
    double gridStepY = gridPointsY > 1 ? borderRect.height() / (gridPointsY - 1) : 0;

    m_programLoading = true;
    m_probeModel.clear();
    m_probeModel.insertRow(0);

    m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G21G90F%1G0Z%2").
                         arg(ui->txtHeightMapProbeFeed->value()).arg(ui->txtHeightMapGridZTop->value()));
    m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G0X0Y0"));
    m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G38.2Z%1")
                         .arg(ui->txtHeightMapGridZBottom->value()));
    m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G0Z%1")
                         .arg(ui->txtHeightMapGridZTop->value()));

    double x, y;

    for (int i = 0; i < gridPointsY; i++) {
        y = borderRect.top() + gridStepY * i;
        for (int j = 0; j < gridPointsX; j++) {
            x = borderRect.left() + gridStepX * (i % 2 ? gridPointsX - 1 - j : j);
            m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G0X%1Y%2")
                                 .arg(x, 0, 'f', 3).arg(y, 0, 'f', 3));
            m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G38.2Z%1")
                                 .arg(ui->txtHeightMapGridZBottom->value()));
            m_probeModel.setData(m_probeModel.index(m_probeModel.rowCount() - 1, 1), QString("G0Z%1")
                                 .arg(ui->txtHeightMapGridZTop->value()));
        }
    }

    m_programLoading = false;

    if (m_currentDrawer == m_probeDrawer) updateParser();

    m_heightmapChanged = true;
    return true;
}

void frmMain::updateHeightmapGrid(double arg1)
{
    if (sender()->property("previousValue").toDouble() != arg1 && !updateHeightmapGrid())
        static_cast<QDoubleSpinBox*>(sender())->setValue(sender()->property("previousValue").toDouble());
    else sender()->setProperty("previousValue", arg1);
}

void frmMain::resizeTableHeightmapSections()
{
    if (ui->tblHeightMap->horizontalHeader()->defaultSectionSize()
            * ui->tblHeightMap->horizontalHeader()->count() < ui->glwVisualizer->width())
        ui->tblHeightMap->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); else {
        ui->tblHeightMap->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    }
}

bool frmMain::eventFilter(QObject *obj, QEvent *event)
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
    } else if (obj == ui->tblProgram && ((m_communicator->senderState() == SenderState::Transferring) || (m_communicator->senderState() == SenderState::Stopping))) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_PageDown || keyEvent->key() == Qt::Key_PageUp
                    || keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_Up) {
            ui->chkAutoScrollGCode->setChecked(false);
        }
    }

    // Visualizer updates
    if (obj == this && event->type() == QEvent::WindowStateChange) {
        ui->glwVisualizer->setUpdatesEnabled(!isMinimized() && ui->dockVisualizer->isVisible());
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

// void frmMain::updateCurrentModel(GCodeTableModel *m_currentModel)
// {
//     this->m_currentModel = m_currentModel;
//     m_program->setModel(m_currentModel);
// }

void frmMain::updateToolPositionAndToolpathShadowing(QVector3D toolPosition)
{
    m_toolDrawer.setToolPosition(m_configuration.visualizerModule().ignoreZ() ? QVector3D(toolPosition.x(), toolPosition.y(), 0) : toolPosition);

    SenderState senderState = m_communicator->senderState();
    MachineState deviceState = m_communicator->machineState();
    if (((senderState == SenderState::Transferring) || (senderState == SenderState::Stopping)
         || (senderState == SenderState::Pausing) || (senderState == SenderState::Pausing2) || (senderState == SenderState::Paused)) && deviceState != MachineState::Check) {
        GCodeViewParser *parser = m_currentDrawer->viewParser();

        bool toolOntoolpath = false;

        QList<int> drawnLines;
        QList<LineSegment>& list = parser->getLineSegmentList();

        for (
            int i = m_lastDrawnLineIndex;
            i < list.count() && list[i].getLineNumber() <= (m_currentModel->data(m_currentModel->index(m_program.processedCommandIndex(), 4)).toInt() + 1);
            i++
        ) {
            if (list[i].contains(toolPosition)) {
                toolOntoolpath = true;
                m_lastDrawnLineIndex = i;
                break;
            }
            drawnLines << i;
        }

        if (toolOntoolpath) {
            foreach (int i, drawnLines) {
                list[i].setDrawn(true);
            }
            if (!drawnLines.isEmpty()) m_currentDrawer->update(drawnLines);
        }
    }
}

void frmMain::updateToolpathShadowingOnCheckMode()
{
    GCodeViewParser *parser = m_currentDrawer->viewParser();
    QList<LineSegment> list = parser->getLineSegmentList();

    if ((m_communicator->m_senderState != SenderState::Stopping) && m_program.processedCommandIndex() < m_currentModel->rowCount() - 1) {
        int i;
        QList<int> drawnLines;

        for (i = m_lastDrawnLineIndex; i < list.count()
                                               && list[i].getLineNumber()
                                                <= (m_currentModel->data(m_currentModel->index(m_program.processedCommandIndex(), 4)).toInt()); i++) {
            drawnLines << i;
        }

        if (!drawnLines.isEmpty() && (i < list.count())) {
            m_lastDrawnLineIndex = i;
            QVector3D vec = list[i].getEnd();
            m_toolDrawer.setToolPosition(vec);
        }

        foreach (int i, drawnLines) {
            list[i].setDrawn(true);
        }
        if (!drawnLines.isEmpty()) m_currentDrawer->update(drawnLines);
    } else {
        for (auto& s : list) {
            if (!qIsNaN(s.getEnd().length())) {
                m_toolDrawer.setToolPosition(s.getEnd());
                break;
            }
        }
    }
}

QString frmMain::lastWorkingDirectory()
{
    return m_configuration.uiModule().currentWorkingDirectory();
}

QTime frmMain::updateProgramEstimatedTime(QList<LineSegment>& lines)
{
    double time = 0;
    partMainOverride::Overrides overrides = ui->overrides->overrides();

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

    ui->glwVisualizer->setSpendTime(QTime(0, 0, 0));
    ui->glwVisualizer->setEstimatedTime(t);

    return t;
}

QList<LineSegment*> frmMain::subdivideSegment(LineSegment* segment)
{
    QList<LineSegment*> list;

    QRectF borderRect = borderRectFromTextboxes();

    double interpolationStepX = borderRect.width() / (ui->txtHeightMapInterpolationStepX->value() - 1);
    double interpolationStepY = borderRect.height() / (ui->txtHeightMapInterpolationStepY->value() - 1);

    double length;

    QVector3D vec = segment->getEnd() - segment->getStart();

    if (qIsNaN(vec.length())) return QList<LineSegment*>();

    if (fabs(vec.x()) / fabs(vec.y()) < interpolationStepX / interpolationStepY) length = interpolationStepY / (vec.y() / vec.length());
    else length = interpolationStepX / (vec.x() / vec.length());

    length = fabs(length);

    if (qIsNaN(length)) {
        return QList<LineSegment*>();
    }

    QVector3D seg = vec.normalized() * length;
    // int count = trunc(vec.length() / length);
    int count = (vec.length() / length);

    if (count == 0) return QList<LineSegment*>();

    for (int i = 0; i < count; i++) {
        LineSegment* line = new LineSegment(segment);
        line->setStart(i == 0 ? segment->getStart() : list[i - 1]->getEnd());
        line->setEnd(line->getStart() + seg);
        list.append(line);
    }

    if (list.count() > 0 && list.last()->getEnd() != segment->getEnd()) {
        LineSegment* line = new LineSegment(segment);
        line->setStart(list.last()->getEnd());
        line->setEnd(segment->getEnd());
        list.append(line);
    }

    return list;
}



// int frmMain::buttonSize()
// {
//     return ui->cmdHome->minimumWidth();
// }

void frmMain::onTransferCompleted()
{
    // Shadow last segment
    GCodeViewParser *parser = m_currentDrawer->viewParser();
    QList<LineSegment> list = parser->getLineSegmentList();
    if (m_lastDrawnLineIndex < list.count()) {
        list[m_lastDrawnLineIndex].setDrawn(true);
        m_currentDrawer->update(QList<int>() << m_lastDrawnLineIndex);
    }

    // Update state
    m_lastDrawnLineIndex = 0;
    updateControlsState();

    // Show message box
    qApp->beep();
    m_communicator->stopUpdatingState();
    m_timerConnection.stop();

    QMessageBox::information(this, qApp->applicationDisplayName(), tr("Job done.\nTime elapsed: %1")
                                .arg(ui->glwVisualizer->spendTime().toString("hh:mm:ss")));

    m_timerConnection.start();
    m_communicator->startUpdatingState();
}

QString frmMain::getLineInitCommands(int row)
{
    int commandIndex = row;

    GCodeViewParser *parser = m_currentDrawer->viewParser();
    QList<LineSegment>& list = parser->getLineSegmentList();
    QVector<QList<int>> lineIndexes = parser->getLinesIndexes();
    QString commands;
    int lineNumber = m_currentModel->data(m_currentModel->index(commandIndex, 4)).toInt();

    if (lineNumber != -1) {
        LineSegment& firstSegment = list[lineIndexes.at(lineNumber).first()];
        LineSegment& lastSegment = list[lineIndexes.at(lineNumber).last()];
        LineSegment& feedSegment = lastSegment;
        LineSegment& plungeSegment = lastSegment;
        int segmentIndex = list.indexOf(feedSegment);
        while (feedSegment.isFastTraverse() && (segmentIndex > 0))
            feedSegment = list.at(--segmentIndex);
        while (!(plungeSegment.isZMovement() && !plungeSegment.isFastTraverse()) && (segmentIndex > 0))
            plungeSegment = list.at(--segmentIndex);


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
    }

    return commands;
}

bool frmMain::actionLessThan(const QAction *a1, const QAction *a2)
{
    return a1->objectName() < a2->objectName();
}

bool frmMain::actionTextLessThan(const QAction *a1, const QAction *a2)
{
    return a1->text() < a2->text();
}
