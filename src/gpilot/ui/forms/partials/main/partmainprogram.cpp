#include "partmainprogram.h"
#include "ui_partmainprogram.h"
#include <QScrollBar>
#include <QHeaderView>
#include <QEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QAbstractItemView>
#include <QMessageBox>
#include "utils/utils.h"
#include "core/gcode/gcode.h"
#include "core/heightmap/heightmap.h"
#include "ui/tables/heightmapitemdelegate.h"
#include "ui/utils/thememanager.h"
#include "ui/utils/uipermissions.h"

PartMainProgram::PartMainProgram(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PartMainProgram)
    , m_programModel(nullptr)
    , m_probeModel(nullptr)
    , m_programHeightmapModel(nullptr)
    , m_currentModel(nullptr)
    , m_heightmapModel(nullptr)
{
    ui->setupUi(this);
    setupUi();
}

PartMainProgram::~PartMainProgram()
{
    delete m_probeModel;
    delete m_programHeightmapModel;
    delete m_heightmapModel;
    delete ui;
}

void PartMainProgram::setupUi()
{
    // Connect buttons
    connect(ui->cmdFileOpen, &QPushButton::clicked, this, &PartMainProgram::openClicked);
    connect(ui->cmdFileSend, &QPushButton::clicked, this, &PartMainProgram::startClicked);
    connect(ui->cmdFilePauseResume, &QPushButton::clicked, this, &PartMainProgram::pauseClicked);
    connect(ui->cmdFileAbort, &QPushButton::clicked, this, &PartMainProgram::abortClicked);
    connect(ui->cmdFileReset, &QPushButton::clicked, this, &PartMainProgram::resetClicked);

    setupTableContextMenu();

    // Connect table signals
    connect(ui->tblProgram->verticalScrollBar(), &QAbstractSlider::actionTriggered, this, &PartMainProgram::onScrollBarAction);
    connect(ui->tblProgram, &QWidget::customContextMenuRequested, this, &PartMainProgram::onTableContextMenuRequested);
    ui->tblProgram->installEventFilter(this);

    auto* viewMenu = new QMenu(this);
    m_actionAutoScroll = viewMenu->addAction(tr("Auto scroll"));
    m_actionAutoScroll->setCheckable(true);
    m_actionShowComments = viewMenu->addAction(tr("Comments"));
    m_actionShowComments->setCheckable(true);
    m_actionShowComments->setChecked(true);

    connect(m_actionShowComments, &QAction::toggled, this, [this](bool checked) {
        m_programModel.setCommentsVisible(checked);
    });
    connect(ui->btnViewOptions, &QPushButton::clicked, this, [this, viewMenu]() {
        viewMenu->popup(ui->btnViewOptions->mapToGlobal(QPoint(0, ui->btnViewOptions->height())));
    });

    // Debounce filter input so every keystroke does not trigger a full
    // O(n) rebuild of the filter mapping on large programs.
    m_filterDebounceTimer.setSingleShot(true);
    m_filterDebounceTimer.setInterval(200);
    // connect(&m_filterDebounceTimer, &QTimer::timeout, this, [this]() {
    //     m_programModel.setFilter(ui->txtFilter->text());
    // });
    // connect(ui->txtFilter, &QLineEdit::textChanged, this, [this]() {
    //     m_filterDebounceTimer.start();
    // });

    connect(&m_programModel, &GCodeTableModel::dataChanged, this, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight) {
        // emit currentChanged(topLeft, bottomRight);
        qDebug() << "Program model data changed from" << topLeft.row() << "to" << bottomRight.row();
    });

    connect(ui->btnProgram, &QPushButton::clicked, this, [this]() {
        //don't allow unchecking
        if (!ui->btnProgram->isChecked()) {
            QSignalBlocker blocker(ui->btnProgram);
            ui->btnProgram->setChecked(true);
            return;
        }
        ui->btnHeightmap->setChecked(false);
        ui->panProgram->setVisible(true);
        showProgramTable();
    });
    connect(ui->btnHeightmap, &QPushButton::clicked, this, [this]() {
        //don't allow unchecking
        if (!ui->btnHeightmap->isChecked()) {
            QSignalBlocker blocker(ui->btnHeightmap);
            ui->btnHeightmap->setChecked(true);
            return;
        }
        ui->btnProgram->setChecked(false);
        ui->panProgram->setVisible(false);
        showHeightmapTable();
    });

    setHeightMapVisible(false);

    auto* hh = ui->tblHeightmap->horizontalHeader();
    hh->setSectionResizeMode(QHeaderView::Interactive);
    auto* vh = ui->tblHeightmap->verticalHeader();
    vh->setSectionResizeMode(QHeaderView::Interactive);

    refreshHeightmapSections();
    connect(&ThemeManager::instance(), &ThemeManager::scaleChanged, this, &PartMainProgram::refreshHeightmapSections);
}

void PartMainProgram::setupTableContextMenu()
{
    m_tableMenu = new QMenu(this);
    m_tableMenu->addAction(tr("&Insert lines"), QKeySequence(Qt::Key_Insert), this, SLOT(onInsertLinesTriggered()));
    m_tableMenu->addAction(tr("&Insert lines after"), QKeySequence(Qt::Key_Insert), this, SLOT(onInsertLinesAfterTriggered()));
    m_tableMenu->addAction(tr("&Delete selected"), QKeySequence(Qt::Key_Delete), this, SLOT(onDeleteSelectedTriggered()));
    m_tableMenu->addAction(tr("&Edit selected"), QKeySequence("E"), this, SLOT(onEditSelectedTriggered()));
}

void PartMainProgram::setProgram(GCode* program)
{
    m_programModel.setProgram(program);
    m_probeModel = new GCodeTableModel(program, this);
    m_programHeightmapModel = new GCodeTableModel(program, this);
}

void PartMainProgram::setHeightmap(Heightmap* heightmap)
{
    m_heightmapModel = new HeightmapTableModel(heightmap, this);
    ui->tblHeightmap->setModel(m_heightmapModel);
    ui->tblHeightmap->setItemDelegate(new HeightmapItemDelegate(-5.0, 5.0, this));
    refreshHeightmapSections();
}

void PartMainProgram::refreshHeightmapSections()
{
    float s = ThemeManager::instance().scaleF();
    int rowH = qRound(36 * s);
    auto* hh = ui->tblHeightmap->horizontalHeader();
    auto* vh = ui->tblHeightmap->verticalHeader();
    hh->setDefaultSectionSize(qRound(60 * s));
    vh->setDefaultSectionSize(rowH);
    for (int i = 0; i < vh->count(); ++i) {
        vh->resizeSection(i, rowH);
    }
    ui->tblHeightmap->resizeColumnsToContents();
}

void PartMainProgram::setAutoScroll(bool enabled)
{
    m_actionAutoScroll->setChecked(enabled);
}

bool PartMainProgram::isAutoScroll() const
{
    return m_actionAutoScroll->isChecked();
}

void PartMainProgram::setHeightMapVisible(bool visible)
{
    ui->tblHeightmap->setVisible(visible);
}

void PartMainProgram::selectFirstRow()
{
    if (ui->tblProgram->model() && ui->tblProgram->model()->rowCount() > 0) {
        ui->tblProgram->selectRow(0);
    }
}

void PartMainProgram::resetToFirstRow()
{
    if (ui->tblProgram->model() && ui->tblProgram->model()->rowCount() > 0) {
        ui->tblProgram->scrollTo(ui->tblProgram->model()->index(0, 0));
        ui->tblProgram->clearSelection();
        ui->tblProgram->selectRow(0);
    }
}

void PartMainProgram::scrollToIndex(int index)
{
    if (!isAutoScroll()) {
        return;
    }
    auto* model = ui->tblProgram->model();
    if (model) {
        const int viewRow = m_programModel.mapFromSource(index);
        const QModelIndex ind = currentModelIndex((viewRow < 0 ? 0 : viewRow) + 2, 0);
        ui->tblProgram->scrollTo(ind);
        // ui->tblProgram->setCurrentIndex(index);
    }
}

void PartMainProgram::setTableUpdatesEnabled(bool enable)
{
    ui->tblProgram->setUpdatesEnabled(enable);
}

void PartMainProgram::setProgramTableModel(QAbstractItemModel* model)
{
    ui->tblProgram->setModel(model);
}

void PartMainProgram::setProgramTableEditTriggers(QAbstractItemView::EditTriggers triggers)
{
    ui->tblProgram->setEditTriggers(triggers);
}

QByteArray PartMainProgram::saveProgramHeaderState() const
{
    return ui->tblProgram->horizontalHeader()->saveState();
}

void PartMainProgram::setCurrentIndex(const QModelIndex& index)
{
    ui->tblProgram->setCurrentIndex(index);
}

void PartMainProgram::abortClicked() { emit abortRequested(); }
void PartMainProgram::startClicked() { emit startRequested(); }
void PartMainProgram::openClicked() { emit openFile(); }
void PartMainProgram::resetClicked() { emit programResetRequested(); }

void PartMainProgram::setProgramVisible(bool visible)
{
    ui->tblProgram->setVisible(visible);
}

void PartMainProgram::showProgramTable()
{
    ui->tblProgram->setVisible(true);
    ui->tblHeightmap->setVisible(false);
}

void PartMainProgram::showHeightmapTable()
{
    ui->tblHeightmap->setVisible(true);
    ui->tblProgram->setVisible(false);
}

QByteArray PartMainProgram::saveHeaderState() const
{
    return ui->tblProgram->horizontalHeader()->saveState();
}

void PartMainProgram::restoreHeaderState(const QByteArray& state)
{
    ui->tblProgram->horizontalHeader()->restoreState(state);
}

void PartMainProgram::updateControlsState(const UiState& state)
{
    // ui->program->setOpenButtonEnabled(canOpenFile);
    // ui->program->setResetButtonEnabled(idle && !program().empty());
    // ui->program->setSendButtonEnabled(sb->canExecute(Action::Type::Run) && !program().empty());
    const auto t = state.sb->type();
    ui->cmdFileOpen->setEnabled(UiPermissions::isAllowed(UiPermission::OpenFile, t));
    ui->cmdFileReset->setEnabled(
        state.files.gcodeOpened &&
        state.sb->is(AbstractStateBehavior::Type::Idle)
    );
    ui->cmdFileSend->setEnabled(
        state.files.gcodeOpened &&
        state.sb->is(AbstractStateBehavior::Type::Idle)
    );
}

// void PartMainProgram::setFileButtonsEnabled(bool open, bool reset, bool send, bool pause, bool abort)
// {
//     ui->cmdFileOpen->setEnabled(open);
//     ui->cmdFileReset->setEnabled(reset);
//     ui->cmdFileSend->setEnabled(send);
//     ui->cmdFilePauseResume->setEnabled(pause);
//     ui->cmdFileAbort->setEnabled(abort);
// }

// void PartMainProgram::setOpenButtonEnabled(bool enabled) { ui->cmdFileOpen->setEnabled(enabled); }
// void PartMainProgram::setResetButtonEnabled(bool enabled) { ui->cmdFileReset->setEnabled(enabled); }
// void PartMainProgram::setSendButtonEnabled(bool enabled) { ui->cmdFileSend->setEnabled(enabled); }
// void PartMainProgram::setAbortButtonEnabled(bool enabled) { ui->cmdFileAbort->setEnabled(enabled); }
// void PartMainProgram::setPauseButtonEnabled(bool enabled) { ui->cmdFilePauseResume->setEnabled(enabled); }

void PartMainProgram::setPauseButtonText(const QString& text)
{
    ui->cmdFilePauseResume->setText(text);
}

void PartMainProgram::setPauseButtonChecked(bool checked)
{
    ui->cmdFilePauseResume->setChecked(checked);
}

void PartMainProgram::setPauseButtonFocus()
{
    ui->cmdFilePauseResume->setFocus();
}

void PartMainProgram::setSendButtonText(const QString& text)
{
    ui->cmdFileSend->setText(text);
}

void PartMainProgram::updateButtonStyles()
{
    Utils::refreshStyle({ui->cmdFileOpen, ui->cmdFileReset, ui->cmdFileSend, ui->cmdFilePauseResume, ui->cmdFileAbort});
}

void PartMainProgram::setSendMenuFirstActionEnabled(bool enabled)
{
    QMenu* menu = ui->cmdFileSend->menu();
    if (menu && !menu->actions().isEmpty()) {
        menu->actions().first()->setEnabled(enabled);
    }
}

// void PartMainProgram::setupFileOpenMenu(QObject* receiver, const char* openGCodeSlot, const char* openHeightmapSlot)
// {
//     QMenu* menu = ui->cmdFileOpen->menu();
//     menu->addAction(tr("Open G-Code file"), receiver, openGCodeSlot);
//     menu->addAction(tr("Open Heightmap file"), receiver, openHeightmapSlot);
// }

void PartMainProgram::setupFileSendMenu(QObject* receiver, const char* sendFromLineSlot)
{
    QMenu* menu = ui->cmdFileSend->menu();
    menu->addAction(tr("Send from current line"), receiver, sendFromLineSlot);
}

void PartMainProgram::insertLines(bool before)
{
    // Editing is disabled when a filter is active (see onTableContextMenuRequested).
    if (m_programModel.isFilterActive()) {
        return;
    }

    QModelIndex current = ui->tblProgram->currentIndex();
    const int sourceRow = m_programModel.mapToSource(current.row());
    if (sourceRow < 0) {
        return;
    }

    emit insertLinesRequested(sourceRow, before);
}

void PartMainProgram::onEditSelectedTriggered()
{
    if (m_programModel.isFilterActive()) {
        return;
    }

    SelRange range = getSelectedRange();
    if (!range.count) {
        return;
    }

    const int fromSrc = m_programModel.mapToSource(range.from);
    const int toSrc = m_programModel.mapToSource(range.to);
    if (fromSrc < 0 || toSrc < 0) {
        return;
    }

    emit editLinesRequested(fromSrc, toSrc);
}

void PartMainProgram::onDeleteSelectedTriggered()
{
    if (m_programModel.isFilterActive()) {
        return;
    }

    SelRange range = getSelectedRange();
    if (!range.count) {
        return;
    }

    if (QMessageBox::warning(this, this->windowTitle(), tr("Delete lines?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }

    const int fromSrc = m_programModel.mapToSource(range.from);
    const int toSrc = m_programModel.mapToSource(range.to);
    if (fromSrc < 0 || toSrc < 0) {
        return;
    }

    emit deleteLinesRequested(fromSrc, toSrc);
}

void PartMainProgram::onTableContextMenuRequested(const QPoint& pos)
{
    QModelIndexList selectedRows = getSelectedRows();
    bool hasSelection = !selectedRows.isEmpty();
    int selectedRow = hasSelection ? selectedRows[0].row() : -1;
    int totalRows = ui->tblProgram->model() ? ui->tblProgram->model()->rowCount() : 0;

    // Editing actions are disabled while a filter is active. A contiguous
    // selection in the filtered view may map to a non-contiguous source range,
    // so insert/edit/delete on the underlying GCode would touch hidden rows.
    const bool editingAllowed = !m_programModel.isFilterActive();

    // 0 - Insert (before)
    // 1 - Insert after
    // 2 - Delete selected
    // 3 - Edit selected
    if (hasSelection && editingAllowed) {
        m_tableMenu->actions().at(0)->setEnabled(true);
        m_tableMenu->actions().at(1)->setEnabled(true);
        m_tableMenu->actions().at(3)->setEnabled(true);
        // Do not delete last row (placeholder for new line)
        m_tableMenu->actions().at(2)->setEnabled(selectedRow != totalRows - 1);
    } else {
        m_tableMenu->actions().at(0)->setEnabled(false);
        m_tableMenu->actions().at(1)->setEnabled(false);
        m_tableMenu->actions().at(2)->setEnabled(false);
        m_tableMenu->actions().at(3)->setEnabled(false);
    }

    m_tableMenu->popup(ui->tblProgram->viewport()->mapToGlobal(pos));
}

void PartMainProgram::onInsertLinesTriggered()
{
    insertLines(true);
}

void PartMainProgram::onInsertLinesAfterTriggered()
{
    insertLines(false);
}

QModelIndexList PartMainProgram::getSelectedRows() const
{
    if (ui->tblProgram->selectionModel()) {
        return ui->tblProgram->selectionModel()->selectedRows();
    }

    return QModelIndexList();
}

PartMainProgram::SelRange PartMainProgram::getSelectedRange() const
{
    QModelIndexList rows = getSelectedRows();
    SelRange range = { -1, -1, 0 };

    if (rows.isEmpty()) {
        return range;
    }

    range.from = rows.first().row();
    range.to = rows.last().row();

    // exclude last row (placeholder for new line) from selection
    if (ui->tblProgram->model() && range.to == ui->tblProgram->model()->rowCount() - 1) {
        range.to--;
    }

    range.count = range.to - range.from + 1;

    return range;
}

int PartMainProgram::getFirstSelectedRow() const
{
    QModelIndexList rows = getSelectedRows();

    return rows.isEmpty() ? -1 : rows[0].row();
}

void PartMainProgram::selectRow(int row)
{
    if (ui->tblProgram->model() && row >= 0 && row < ui->tblProgram->model()->rowCount()) {
        ui->tblProgram->selectRow(row);
    }
}

void PartMainProgram::setRecentFiles(QStringList files)
{
    if (files.empty()) {
        ui->cmdFileOpen->setMenu(nullptr);
        return;
    }

    // menu() returns menu even if we set it to nullptr before
    QMenu* menu = ui->cmdFileOpen->menu();
    menu->clear();

    QAction* lastAction = nullptr;
    for (auto& file : files) {
        QAction *action = new QAction(file, this);
        connect(action, &QAction::triggered, this, &PartMainProgram::openRecentFile);
        menu->insertAction(lastAction, action);
        lastAction = action;
    }

    menu->addSeparator();

    QAction *clearAction = new QAction(tr("&Clear"), this);
    connect(clearAction, &QAction::triggered, this, [this]() {
        emit clearRecentFiles();
    });

    menu->addAction(clearAction);

    ui->cmdFileOpen->setMenu(menu);
}

/*
 * Unload program and heightmap, close drawers and visualizer state
 */
void PartMainProgram::close()
{
    m_programModel.setProgram(nullptr);
    delete m_probeModel; m_probeModel = nullptr;
    delete m_programHeightmapModel; m_programHeightmapModel = nullptr;
}

void PartMainProgram::pauseClicked(bool checked) {
    emit pause(checked);
}

void PartMainProgram::openRecentFile()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        emit openFile(action->text());
    }
}

bool PartMainProgram::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->tblProgram) {
        if (event->type() == QEvent::KeyPress) {
             QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
             if (keyEvent->key() == Qt::Key_PageDown || keyEvent->key() == Qt::Key_PageUp
                        || keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_Up) {
                emit manualScrollRequested();
             }
        }
    }

    return QWidget::eventFilter(obj, event);
}

void PartMainProgram::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void PartMainProgram::onScrollBarAction(int action)
{
    Q_UNUSED(action)
    emit manualScrollRequested();
}

void PartMainProgram::initialize(GCode* program, Heightmap* heightmap)
{

    // Initialize models with data sources
    if (program) {
        setProgram(program);
    }
    if (heightmap) {
        setHeightmap(heightmap);
    }

    m_currentModel = &m_programModel;

    // Connect model signals
    // connect(m_programModel, &QAbstractItemModel::dataChanged, this, &PartMainProgram::modelDataChanged);
    // connect(m_programHeightmapModel, &QAbstractItemModel::dataChanged, this, &PartMainProgram::modelDataChanged);
    // connect(m_probeModel, &QAbstractItemModel::dataChanged, this, &PartMainProgram::modelDataChanged);
    // connect(m_heightmapModel, &HeightmapTableModel::dataChangedByUserInput, this, &PartMainProgram::heightmapDataChangedByUser);

    connect(program, &GCode::loaded, this, [this]() {
        m_programModel.update();
    });
    connect(program, &GCode::linesUpdated, this, [this](int from, int to) {
        m_programModel.updateLines(from, to);
    });

    // Set models to UI
    ui->tblProgram->setModel(&m_programModel);
    ui->tblProgram->setItemDelegate(&m_programItemDelegate);
    ui->tblHeightmap->setModel(m_heightmapModel);

    // Setup table columns, expand last column
    ui->tblProgram->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    // Connect selection changes
    if (ui->tblProgram->selectionModel()) {
        connect(ui->tblProgram->selectionModel(), &QItemSelectionModel::currentChanged, this, &PartMainProgram::currentChanged);
    }
}

void PartMainProgram::insertRowInCurrentModel(int row)
{
    if (m_currentModel) {
        m_currentModel->insertRow(row);
    }
}

void PartMainProgram::removeRowsFromCurrentModel(int row, int count)
{
    if (m_currentModel) {
        m_currentModel->removeRows(row, count);
    }
}

int PartMainProgram::currentModelRowCount() const
{
    return m_currentModel ? m_currentModel->rowCount() : 0;
}

QModelIndex PartMainProgram::currentModelIndex(int row, int column) const
{
    return m_currentModel ? m_currentModel->index(row, column) : QModelIndex();
}

QVariant PartMainProgram::currentModelData(const QModelIndex& index) const
{
    return m_currentModel ? m_currentModel->data(index) : QVariant();
}

void PartMainProgram::setCurrentModelData(const QModelIndex& index, const QVariant& value)
{
    if (m_currentModel) {
        m_currentModel->setData(index, value);
    }
}

// Program model operations
void PartMainProgram::clearProgramModel()
{
    m_programModel.clear();
}


int PartMainProgram::programModelRowCount() const
{
    return m_programModel.rowCount();
}

void PartMainProgram::insertProgramModelRow(int row)
{
    m_programModel.insertRow(row);
}

void PartMainProgram::switchToProgramModel()
{
    if (m_currentModel == &m_programModel) {
        return;
    }

    m_currentModel = &m_programModel;
    ui->tblProgram->setModel(&m_programModel);
}

// Probe model operations
void PartMainProgram::clearProbeModel()
{
    if (m_probeModel) {
        m_probeModel->clear();
    }
}

void PartMainProgram::addProbeModelRow()
{
    if (m_probeModel) {
        m_probeModel->insertRow(m_probeModel->rowCount());
    }
}

void PartMainProgram::setProbeModelData(int row, int column, const QVariant& value)
{
    if (m_probeModel) {
        m_probeModel->setData(m_probeModel->index(row, column), value);
    }
}

void PartMainProgram::insertProbeModelRow(int row)
{
    if (m_probeModel) {
        m_probeModel->insertRow(row);
    }
}

int PartMainProgram::probeModelRowCount() const
{
    return m_probeModel ? m_probeModel->rowCount() : 0;
}

void PartMainProgram::switchToProbeModel()
{
    if (m_currentModel == m_probeModel) return;
    m_currentModel = m_probeModel;
    ui->tblProgram->setModel(m_probeModel);
}

// Heightmap model operations
void PartMainProgram::clearHeightmapModel()
{
    if (m_heightmapModel) {
        m_heightmapModel->clear();
    }
}

void PartMainProgram::resizeHeightmapModel(int rows, int cols)
{
    if (m_heightmapModel) {
        m_heightmapModel->resize(rows, cols);
    }
}

bool PartMainProgram::hasHeightmapData() const
{
    if (!m_heightmapModel) return false;

    for (int i = 0; i < m_heightmapModel->rowCount(); i++) {
        for (int j = 0; j < m_heightmapModel->columnCount(); j++) {
            if (!qIsNaN(m_heightmapModel->data(m_heightmapModel->index(i, j), Qt::UserRole).toDouble())) {
                return true;
            }
        }
    }
    return false;
}

QVariant PartMainProgram::heightmapModelData(int row, int column, int role) const
{
    if (m_heightmapModel) {
        return m_heightmapModel->data(m_heightmapModel->index(row, column), role);
    }
    return QVariant();
}

void PartMainProgram::scrollToHeightmapCell(int x, int y)
{
    if (!m_heightmapModel) return;
    int tableRow = m_heightmapModel->rowCount() - 1 - y;
    QModelIndex idx = m_heightmapModel->index(tableRow, x);
    if (!idx.isValid()) return;
    ui->tblHeightmap->setCurrentIndex(idx);
    ui->tblHeightmap->scrollTo(idx, QAbstractItemView::PositionAtCenter);
}

// ProgramHeightmap model operations
void PartMainProgram::clearProgramHeightmapModel()
{
    if (m_programHeightmapModel) {
        m_programHeightmapModel->clear();
    }
}

// Current model operations
bool PartMainProgram::isCurrentModelProgramModel() const
{
    return m_currentModel == &m_programModel;
}
