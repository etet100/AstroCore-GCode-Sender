#include "partmaincontrol.h"
#include "ui_partmaincontrol.h"
#include <QDebug>

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
    });
    connect(m_actDualProbe, &QAction::triggered, this, [this]() {
        m_probeMode = ProbeMode::Dual;
    });

    // Show menu on right-click; left-click still fires clicked() -> probe
    ui->cmdProbe->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->cmdProbe, &QWidget::customContextMenuRequested, this, [this, menu](const QPoint &pos) {
        menu->exec(ui->cmdProbe->mapToGlobal(pos));
    });
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

void PartMainControl::updateControlsState(SenderState senderState, MachineState machineState)
{
    ui->cmdCheck->setEnabled(machineState != MachineState::Run && (senderState == SenderState::Stopped));
    ui->cmdCheck->setChecked(machineState == MachineState::Check);
    ui->cmdHold->setChecked(machineState == MachineState::Hold0 || machineState == MachineState::Hold1 || machineState == MachineState::Queue);
    ui->cmdProbe->setEnabled(machineState == MachineState::Idle && senderState == SenderState::Stopped);
    ui->cmdZeroZ->setEnabled(machineState == MachineState::Idle && senderState == SenderState::Stopped);
    ui->cmdZeroXY->setEnabled(machineState == MachineState::Idle && senderState == SenderState::Stopped);
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
