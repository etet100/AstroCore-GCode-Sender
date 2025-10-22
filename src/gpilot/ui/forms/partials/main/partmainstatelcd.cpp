
#include "partmainstatelcd.h"
#include "ui_partmainstatelcd.h"
#include <QFontDatabase>

partMainStateLcd::partMainStateLcd(QWidget *parent)
    : PartMainStateBase(parent)
    , ui(new Ui::partMainStateLcd)
{
    ui->setupUi(this);
    initializeColorsAndCaptions();
    setWorkCoordinates(QVector3D(0, 0, 0));
    setMachineCoordinates(QVector3D(0, 0, 0));
    QFontDatabase::addApplicationFont(":/fonts/Patopian1986.ttf");
}

partMainStateLcd::~partMainStateLcd()
{
    delete ui;
}

void partMainStateLcd::setStatusText(QString status, QString bgColor, QString fgColor)
{
    ui->txtStatus->setText(status);
    ui->txtStatus->setStyleSheet(QString("background-color: %1; color: %2;").arg(bgColor, fgColor));
}

void partMainStateLcd::setConName(QString name)
{
    ui->txtConName->setText(name);
}

void partMainStateLcd::setState(MachineState state)
{
    Q_UNUSED(state);
}

QString partMainStateLcd::formatPos(float val)
{
    return QString("%1").arg(val, 0, 'f', 3).rightJustified(7, ' ');
}

void partMainStateLcd::setWorkCoordinates(QVector3D pos)
{
    ui->txtWX->setText(formatPos(pos.x()));
    ui->txtWY->setText(formatPos(pos.y()));
    ui->txtWZ->setText(formatPos(pos.z()));
}

void partMainStateLcd::setMachineCoordinates(QVector3D pos)
{
    ui->txtMX->setText(formatPos(pos.x()));
    ui->txtMY->setText(formatPos(pos.y()));
    ui->txtMZ->setText(formatPos(pos.z()));
}

void partMainStateLcd::setUnits(Units units)
{
    int prec = units == Units::Millimeters ? 3 : 4;
    // Wersja LCD nie ustawia precyzji przez setDecimals
    Q_UNUSED(prec);
}

