#include "partmainprogram.h"
#include "ui_partmainprogram.h"
#include <QScrollBar>
#include <QHeaderView>
#include <QEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QAbstractItemView>

PartMainProgram::PartMainProgram(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PartMainProgram)
{
    ui->setupUi(this);
    setupUi();
}

PartMainProgram::~PartMainProgram()
{
    delete ui;
}

void PartMainProgram::setupUi()
{
    // Connect buttons
    connect(ui->cmdFileOpen, &QPushButton::clicked, this, &PartMainProgram::openClicked);
    connect(ui->cmdFileSend, &QPushButton::clicked, this, &PartMainProgram::startClicked);
    connect(ui->cmdFilePause, &QPushButton::clicked, this, &PartMainProgram::pauseClicked);
    connect(ui->cmdFileAbort, &QPushButton::clicked, this, &PartMainProgram::abortClicked);
    connect(ui->cmdFileReset, &QPushButton::clicked, this, &PartMainProgram::resetClicked);

    // Connect table signals
    connect(ui->tblProgram->verticalScrollBar(), &QAbstractSlider::actionTriggered, this, &PartMainProgram::onScrollBarAction);
    connect(ui->tblProgram, &QWidget::customContextMenuRequested, this, &PartMainProgram::customContextMenuRequested);
    ui->tblProgram->installEventFilter(this);

    // Connect checkbox signals
    connect(ui->chkHideComments, &QCheckBox::checkStateChanged, this, [this](int state) {
        emit hideCommentsChanged(state == Qt::Checked);
    });
}

void PartMainProgram::setProgramModel(QAbstractItemModel* model)
{
    ui->tblProgram->setModel(model);

    // Setup table columns after model is set
    if (model) {
        ui->tblProgram->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
        ui->tblProgram->hideColumn(4);
        ui->tblProgram->hideColumn(5);
    }

    if (ui->tblProgram->selectionModel()) {
        connect(ui->tblProgram->selectionModel(), &QItemSelectionModel::currentChanged, this, &PartMainProgram::currentChanged);
    }
}

void PartMainProgram::setProgramItemDelegate(QAbstractItemDelegate* delegate)
{
    ui->tblProgram->setItemDelegate(delegate);
}

void PartMainProgram::setHeightMapModel(QAbstractItemModel* model)
{
    ui->tblHeightMap->setModel(model);
    // Logic from FrmMain for heightmap table header
    if (ui->tblHeightMap->horizontalHeader()->defaultSectionSize() * ui->tblHeightMap->horizontalHeader()->count() < width()) {
         ui->tblHeightMap->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    } else {
         ui->tblHeightMap->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    }
}

void PartMainProgram::setAutoScroll(bool enabled)
{
    ui->chkAutoScrollGCode->setChecked(enabled);
}

bool PartMainProgram::isAutoScroll() const
{
    return ui->chkAutoScrollGCode->isChecked();
}

void PartMainProgram::setHideComments(bool enabled)
{
    ui->chkHideComments->setChecked(enabled);
}

bool PartMainProgram::isHideComments() const
{
    return ui->chkHideComments->isChecked();
}

void PartMainProgram::setHeightMapVisible(bool visible)
{
    ui->tblHeightMap->setVisible(visible);
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

void PartMainProgram::scrollToCurrentIndex(const QModelIndex& index)
{
    ui->tblProgram->scrollTo(index);
    ui->tblProgram->setCurrentIndex(index);
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

void PartMainProgram::abortClicked() { emit abort(); }
void PartMainProgram::startClicked() { emit start(); }
void PartMainProgram::openClicked() { emit open(); }
void PartMainProgram::resetClicked() { emit reset(); }
void PartMainProgram::setProgramVisible(bool visible)
{
    ui->tblProgram->setVisible(visible);
}

void PartMainProgram::resizeHeightMapSections()
{
    if (ui->tblHeightMap->horizontalHeader()->defaultSectionSize() * ui->tblHeightMap->horizontalHeader()->count() < width()) {
         ui->tblHeightMap->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    } else {
         ui->tblHeightMap->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    }
}

QByteArray PartMainProgram::saveHeaderState() const
{
    return ui->tblProgram->horizontalHeader()->saveState();
}

void PartMainProgram::restoreHeaderState(const QByteArray& state)
{
    ui->tblProgram->horizontalHeader()->restoreState(state);
}

void PartMainProgram::setFileButtonsEnabled(bool open, bool reset, bool send, bool pause, bool abort)
{
    ui->cmdFileOpen->setEnabled(open);
    ui->cmdFileReset->setEnabled(reset);
    ui->cmdFileSend->setEnabled(send);
    ui->cmdFilePause->setEnabled(pause);
    ui->cmdFileAbort->setEnabled(abort);
}

void PartMainProgram::setOpenButtonEnabled(bool enabled) { ui->cmdFileOpen->setEnabled(enabled); }
void PartMainProgram::setResetButtonEnabled(bool enabled) { ui->cmdFileReset->setEnabled(enabled); }
void PartMainProgram::setSendButtonEnabled(bool enabled) { ui->cmdFileSend->setEnabled(enabled); }
void PartMainProgram::setAbortButtonEnabled(bool enabled) { ui->cmdFileAbort->setEnabled(enabled); }
void PartMainProgram::setPauseButtonEnabled(bool enabled) { ui->cmdFilePause->setEnabled(enabled); }

void PartMainProgram::setPauseButtonText(const QString& text)
{
    ui->cmdFilePause->setText(text);
}

void PartMainProgram::setPauseButtonChecked(bool checked)
{
    ui->cmdFilePause->setChecked(checked);
}

void PartMainProgram::setPauseButtonFocus()
{
    ui->cmdFilePause->setFocus();
}

void PartMainProgram::setSendButtonText(const QString& text)
{
    ui->cmdFileSend->setText(text);
}

void PartMainProgram::updateButtonStyles()
{
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
}

void PartMainProgram::setSendMenuFirstActionEnabled(bool enabled)
{
    QMenu* menu = ui->cmdFileSend->menu();
    if (menu && !menu->actions().isEmpty()) {
        menu->actions().first()->setEnabled(enabled);
    }
}

QMenu* PartMainProgram::getFileOpenMenu()
{
    return ui->cmdFileOpen->menu();
}

void PartMainProgram::setupFileOpenMenu(QObject* receiver, const char* openGCodeSlot, const char* openHeightmapSlot)
{
    QMenu* menu = ui->cmdFileOpen->menu();
    menu->addAction(tr("Open G-Code file"), receiver, openGCodeSlot);
    menu->addAction(tr("Open Heightmap file"), receiver, openHeightmapSlot);
}

void PartMainProgram::setupFileSendMenu(QObject* receiver, const char* sendFromLineSlot)
{
    QMenu* menu = ui->cmdFileSend->menu();
    menu->addAction(tr("Send from current line"), receiver, sendFromLineSlot);
}

void PartMainProgram::showTableContextMenu(const QPoint& pos, QMenu* menu, bool hasSelection, int selectedRow, int totalRows)
{
    if (hasSelection) {
        menu->actions().at(0)->setEnabled(true);
        menu->actions().at(1)->setEnabled(selectedRow != totalRows - 1);
    } else {
        menu->actions().at(0)->setEnabled(false);
        menu->actions().at(1)->setEnabled(false);
    }
    menu->popup(ui->tblProgram->viewport()->mapToGlobal(pos));
}

QModelIndexList PartMainProgram::getSelectedRows() const
{
    if (ui->tblProgram->selectionModel())
        return ui->tblProgram->selectionModel()->selectedRows();
    return QModelIndexList();
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

void PartMainProgram::pauseClicked(bool checked) {
    emit pause(checked);
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


void PartMainProgram::onScrollBarAction(int action)
{
    Q_UNUSED(action)
    emit manualScrollRequested();
}


