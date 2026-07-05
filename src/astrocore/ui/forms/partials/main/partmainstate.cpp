
#include "partmainstate.h"
#include "ui_partmainstate.h"

PartMainState::PartMainState(QWidget *parent)
    : AbstractPartMainState(parent)
    , ui(new Ui::partMainState)
{
    ui->setupUi(this);
    initializeColorsAndCaptions();
}

PartMainState::~PartMainState()
{
    delete ui;
}

void PartMainState::setStatusText(QString status, QColor bgColor, QColor fgColor)
{
    ui->txtStatus->setText(status);
    ui->txtStatus->setStyleSheet(QString("background-color: %1; color: %2;").arg(bgColor.name(), fgColor.name()));
}

void PartMainState::setState(MachineState state)
{
}

void PartMainState::setWorkCoordinates(QVector3D pos)
{
    ui->txtWX->setValue(pos.x());
    ui->txtWY->setValue(pos.y());
    ui->txtWZ->setValue(pos.z());
}

void PartMainState::setMachineCoordinates(QVector3D pos)
{
    ui->txtMX->setValue(pos.x());
    ui->txtMY->setValue(pos.y());
    ui->txtMZ->setValue(pos.z());
}

void PartMainState::setUnits(Units units)
{
    int prec = units == Units::Millimeters ? 3 : 4;
    ui->txtMX->setDecimals(prec);
    ui->txtMY->setDecimals(prec);
    ui->txtMZ->setDecimals(prec);
    ui->txtWX->setDecimals(prec);
    ui->txtWY->setDecimals(prec);
    ui->txtWZ->setDecimals(prec);
}
