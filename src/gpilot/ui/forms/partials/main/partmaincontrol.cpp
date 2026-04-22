#include "partmaincontrol.h"
#include "ui_partmaincontrol.h"
#include "ui/utils/thememanager.h"
#include "utils/utils.h"
#include <QDebug>
#include <QActionGroup>
#include <QToolButton>

PartMainControl::PartMainControl(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainControl)
{
    ui->setupUi(this);
    setupProbeMenu();
}

void PartMainControl::setupProbeMenu()
{
    auto *menu = new QMenu(this);

    m_actSingleProbe = menu->addAction(tr("Single probe"));
    m_actSingleProbe->setCheckable(true);
    m_actSingleProbe->setChecked(true);

    m_actDualProbe = menu->addAction(tr("Dual probe"));
    m_actDualProbe->setCheckable(true);

    // Use exclusive action group so only one mode is checked at a time
    auto *group = new QActionGroup(this);
    group->addAction(m_actSingleProbe);
    group->addAction(m_actDualProbe);
    group->setExclusive(true);

    connect(m_actSingleProbe, &QAction::triggered, this, [this]() {
        m_probeMode = ProbeMode::Single;
        updateProbeIcon();
    });
    connect(m_actDualProbe, &QAction::triggered, this, [this]() {
        m_probeMode = ProbeMode::Dual;
        updateProbeIcon();
    });

    // Show menu on right-click; left-click still fires clicked() -> probe
    ui->cmdProbe->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->cmdProbe->setMenu(menu);
    // connect(ui->cmdProbe, &QWidget::customContextMenuRequested, this, [this, menu](const QPoint &pos) {
    //     menu->exec(ui->cmdProbe->mapToGlobal(pos));
    // });

    connect(ui->cmdScanTable, &QToolButton::clicked, this, &PartMainControl::onCmdScanTableClicked);
}

void PartMainControl::updateProbeIcon()
{
    if (m_probeMode == ProbeMode::Single) {
        ui->cmdProbe->setIcon(QIcon(":/images/probe_z.svg"));
    } else {
        ui->cmdProbe->setIcon(QIcon(":/images/probe_z_dual.svg"));
    }
    if (ThemeManager::instance().dark()) {
        Utils::invertButtonIconColors(ui->cmdProbe);
    }
}

PartMainControl::~PartMainControl()
{
    delete ui;
}

void PartMainControl::enable()
{

}

void PartMainControl::disable()
{

}

void PartMainControl::updateControlsState(bool portOpened, bool process)
{
    // ui->cmdCheck->setEnabled(portOpened && !process);
    // ui->cmdHome->setEnabled(!process);
    // ui->cmdCheck->setEnabled(!process);
    // ui->cmdUnlock->setEnabled(!process);
    // //ui->cmdSpindle->setEnabled(!process);
    // ui->cmdSleep->setEnabled(!process);
}

void PartMainControl::updateControlsState(AbstractStateBehavior *sb)
{
    // TODO: replace with sb->canExecute(Action::CheckMode) once Action::CheckMode is added
    ui->cmdCheck->setEnabled(sb->isOneOf(AbstractStateBehavior::Type::Idle, AbstractStateBehavior::Type::CheckMode));
    ui->cmdCheck->setChecked(sb->is(AbstractStateBehavior::Type::CheckMode));
    {
        QSignalBlocker blocker(ui->cmdHold);
        ui->cmdHold->setChecked(sb->is(AbstractStateBehavior::Type::Hold));
    }
    ui->cmdProbe->setEnabled(sb->canExecute(Action::Type::Probe));
    ui->cmdZeroZ->setEnabled(sb->canExecute(Action::Type::ZeroZ));
    ui->cmdZeroXY->setEnabled(sb->canExecute(Action::Type::ZeroXY));
    if (sb->isOneOf(AbstractStateBehavior::Type::ScanTableError)) {
        ui->cmdScanTable->setIcon(QIcon(":/images/control/scan_table_resume.svg"));
        ui->cmdScanTable->setEnabled(true);
        ui->cmdScanTable->setProperty("action", "scan");
    } else {
        ui->cmdScanTable->setIcon(QIcon(":/images/control/scan_table.svg"));
        ui->cmdScanTable->setEnabled(sb->canExecute(Action::Type::ScanTable));
        ui->cmdScanTable->setProperty("action", "resume");
    }
    if (ThemeManager::instance().dark()) {
        Utils::invertButtonIconColors(ui->cmdScanTable);
    }
}

bool PartMainControl::hold()
{
    return ui->cmdHold->isChecked();
}

void PartMainControl::setFlood(bool state)
{
    ui->cmdFlood->setChecked(state);
}

void PartMainControl::onCmdHomeClicked()
{
    emit this->home();
    emit this->command(GRBLCommand::Home);
}

void PartMainControl::onCmdCheckClicked(bool checked)
{
    if (checked) {
        emit this->check();
        emit this->command(GRBLCommand::Check);
    } else {
        emit this->abortCheck();
        emit this->command(GRBLCommand::AbortCheck);
    }
}

void PartMainControl::onCmdResetClicked()
{
    emit this->reset();
    emit this->command(GRBLCommand::Reset);
}

void PartMainControl::onCmdUnlockClicked()
{
    emit this->unlock();
    emit this->command(GRBLCommand::Unlock);
}

void PartMainControl::onCmdHoldClicked(bool checked)
{
    qDebug() << "[FrmMain] Hold" << checked;
}

void PartMainControl::onCmdSleepClicked()
{
}

void PartMainControl::onCmdDoorClicked()
{
}

void PartMainControl::onCmdFloodClicked(bool checked)
{
}

void PartMainControl::onCmdProbeClicked()
{
    emit this->probe(m_probeMode);
    emit this->command(GRBLCommand::Probe);
}

void PartMainControl::onCmdZeroZClicked()
{
    emit this->zeroZ();
    emit this->command(GRBLCommand::ZeroZ);
}

void PartMainControl::onCmdZeroXYClicked()
{
    emit this->zeroXY();
    emit this->command(GRBLCommand::ZeroXY);
}

void PartMainControl::onCmdScanTableClicked()
{
    emit this->scanTable();
}

void PartMainControl::textsVisible(bool visible)
{
    const auto style = visible ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly;
    for (auto *button : findChildren<QToolButton *>()) {
        button->setToolButtonStyle(style);
    }
}
