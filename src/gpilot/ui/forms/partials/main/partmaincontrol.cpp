#include "partmaincontrol.h"
#include "ui_partmaincontrol.h"
#include <QDebug>

partMainControl::partMainControl(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainControl)
{
    ui->setupUi(this);
}

partMainControl::~partMainControl()
{
    delete ui;
}

void partMainControl::enable()
{

}

void partMainControl::disable()
{

}

void partMainControl::updateControlsState(bool portOpened, bool process)
{
    // ui->cmdCheck->setEnabled(portOpened && !process);
    // ui->cmdHome->setEnabled(!process);
    // ui->cmdCheck->setEnabled(!process);
    // ui->cmdUnlock->setEnabled(!process);
    // //ui->cmdSpindle->setEnabled(!process);
    // ui->cmdSleep->setEnabled(!process);
}

void partMainControl::updateControlsState(SenderState senderState, DeviceState deviceState)
{
    ui->cmdCheck->setEnabled(deviceState != DeviceState::Run && (senderState == SenderState::Stopped));
    ui->cmdCheck->setChecked(deviceState == DeviceState::Check);
    ui->cmdHold->setChecked(deviceState == DeviceState::Hold0 || deviceState == DeviceState::Hold1 || deviceState == DeviceState::Queue);
    ui->cmdProbe->setEnabled(deviceState == DeviceState::Idle && senderState == SenderState::Stopped);
    ui->cmdZeroZ->setEnabled(deviceState == DeviceState::Idle && senderState == SenderState::Stopped);
    ui->cmdZeroXY->setEnabled(deviceState == DeviceState::Idle && senderState == SenderState::Stopped);
}

bool partMainControl::hold()
{
    return ui->cmdHold->isChecked();
}

void partMainControl::setFlood(bool state)
{
    ui->cmdFlood->setChecked(state);
}

void partMainControl::onCmdHomeClicked()
{
    emit this->home();
    emit this->command(GRBLCommand::Home);
}

void partMainControl::onCmdCheckClicked(bool checked)
{

}

void partMainControl::onCmdResetClicked()
{
    emit this->reset();
    emit this->command(GRBLCommand::Reset);
}

void partMainControl::onCmdUnlockClicked()
{
    emit this->unlock();
    emit this->command(GRBLCommand::Unlock);
}

void partMainControl::onCmdHoldClicked(bool checked)
{
    qDebug() << "Hold" << checked;
}

void partMainControl::onCmdSleepClicked()
{
}

void partMainControl::onCmdDoorClicked()
{
}

void partMainControl::onCmdFloodClicked(bool checked)
{
}

void partMainControl::onCmdProbeClicked()
{
    emit this->probe();
    emit this->command(GRBLCommand::Probe);
}

void partMainControl::onCmdZeroZClicked()
{
    emit this->zeroZ();
    emit this->command(GRBLCommand::ZeroZ);
}

void partMainControl::onCmdZeroXYClicked()
{
    emit this->zeroXY();
    emit this->command(GRBLCommand::ZeroXY);
}
