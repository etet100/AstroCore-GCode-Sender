
#include "partmainstate.h"
#include "ui_partmainstate.h"

partMainState::partMainState(QWidget *parent)
    : PartMainStateBase(parent)
    , ui(new Ui::partMainState)
{
    ui->setupUi(this);
    initializeColorsAndCaptions();
}

partMainState::~partMainState()
{
    delete ui;
}

void partMainState::setStatusText(QString status, QString bgColor, QString fgColor)
{
    ui->txtStatus->setText(status);
    ui->txtStatus->setStyleSheet(QString("background-color: %1; color: %2;").arg(bgColor, fgColor));
}

void partMainState::setState(MachineState state)
{
}

void partMainState::setWorkCoordinates(QVector3D pos)
{
    ui->txtWX->setValue(pos.x());
    ui->txtWY->setValue(pos.y());
    ui->txtWZ->setValue(pos.z());
}

void partMainState::setMachineCoordinates(QVector3D pos)
{
    ui->txtMX->setValue(pos.x());
    ui->txtMY->setValue(pos.y());
    ui->txtMZ->setValue(pos.z());
}

void partMainState::setUnits(Units units)
{
    int prec = units == Units::Millimeters ? 3 : 4;
    ui->txtMX->setDecimals(prec);
    ui->txtMY->setDecimals(prec);
    ui->txtMZ->setDecimals(prec);
    ui->txtWX->setDecimals(prec);
    ui->txtWY->setDecimals(prec);
    ui->txtWZ->setDecimals(prec);
}
