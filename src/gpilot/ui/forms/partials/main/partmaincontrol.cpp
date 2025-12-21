#include "partmaincontrol.h"
#include "ui_partmaincontrol.h"
#include <QDebug>

PartMainControl::PartMainControl(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainControl)
{
    ui->setupUi(this);
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
    qDebug() << "Hold" << checked;
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
    emit this->probe();
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
