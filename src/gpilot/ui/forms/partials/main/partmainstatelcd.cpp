#include "partmainstatelcd.h"
#include "ui_partmainstatelcd.h"
#include <QFontDatabase>

partMainStateLcd::partMainStateLcd(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainStateLcd)
    // , m_configuration(configuration)
{
    ui->setupUi(this);

    initializeColorsAndCaptions();

    setWorkCoordinates(QVector3D(0, 0, 0));
    setMachineCoordinates(QVector3D(0, 0, 0));

    QFontDatabase::addApplicationFont(":/fonts/Patopian1986.ttf");

    // qDebug() << ui->txtMX->fontMetrics().height();
    // qDebug() << ui->txtMX->fontMetrics().lineSpacing();
}

partMainStateLcd::~partMainStateLcd()
{
    delete ui;
}

void partMainStateLcd::setStatusText(QString status, QString bgColor, QString fgColor)
{
    ui->txtStatus->setText(status);
    ui->txtStatus->setStyleSheet(QString("background-color: %1; color: %2;")
                                     .arg(bgColor, fgColor));

}

void partMainStateLcd::setConName(QString name)
{
    ui->txtConName->setText(name);
}

void partMainStateLcd::setState(MachineState state)
{
    Q_UNUSED(state);
    // setStatusText(m_statusCaptions[state], m_statusBackColors[state], m_statusForeColors[state]);
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
    // int bound = m_settings->units() == 0 ? 9999 : 999;

    // ui->txtMX->setDecimals(prec);
    // // @TODO what are these for?
    // // ui->txtMX->setMinimum(-bound);
    // // ui->txtMX->setMaximum(bound);
    // ui->txtMY->setDecimals(prec);
    // // @TODO what are these for?
    // // ui->txtMY->setMinimum(-bound);
    // // ui->txtMY->setMaximum(bound);
    // ui->txtMZ->setDecimals(prec);
    // // ui->txtMZ->setMinimum(-bound);
    // // ui->txtMZ->setMaximum(bound);

    // ui->txtWX->setDecimals(prec);
    // // ui->txtWX->setMinimum(-bound);
    // // ui->txtWX->setMaximum(bound);
    // ui->txtWY->setDecimals(prec);
    // // ui->txtWY->setMinimum(-bound);
    // // ui->txtWY->setMaximum(bound);
    // ui->txtWZ->setDecimals(prec);
    // ui->txtWZ->setMinimum(-bound);
    // ui->txtWZ->setMaximum(bound);
}

void partMainStateLcd::initializeColorsAndCaptions()
{
    m_statusCaptions[MachineState::Unknown] = tr("Unknown");
    m_statusCaptions[MachineState::Idle] = tr("Idle");
    m_statusCaptions[MachineState::Alarm] = tr("Alarm");
    m_statusCaptions[MachineState::Run] = tr("Run");
    m_statusCaptions[MachineState::Home] = tr("Home");
    m_statusCaptions[MachineState::Hold0] = tr("Hold") + " (0)";
    m_statusCaptions[MachineState::Hold1] = tr("Hold") + " (1)";
    m_statusCaptions[MachineState::Queue] = tr("Queue");
    m_statusCaptions[MachineState::Check] = tr("Check");
    m_statusCaptions[MachineState::Door0] = tr("Door") + " (0)";
    m_statusCaptions[MachineState::Door1] = tr("Door") + " (1)";
    m_statusCaptions[MachineState::Door2] = tr("Door") + " (2)";
    m_statusCaptions[MachineState::Door3] = tr("Door") + " (3)";
    m_statusCaptions[MachineState::Jog] = tr("Jog");
    m_statusCaptions[MachineState::Sleep] = tr("Sleep");

    m_statusBackColors[MachineState::Unknown] = "red";
    m_statusBackColors[MachineState::Idle] = "palette(button)";
    m_statusBackColors[MachineState::Alarm] = "red";
    m_statusBackColors[MachineState::Run] = "lime";
    m_statusBackColors[MachineState::Home] = "lime";
    m_statusBackColors[MachineState::Hold0] = "yellow";
    m_statusBackColors[MachineState::Hold1] = "yellow";
    m_statusBackColors[MachineState::Queue] = "yellow";
    m_statusBackColors[MachineState::Check] = "palette(button)";
    m_statusBackColors[MachineState::Door0] = "red";
    m_statusBackColors[MachineState::Door1] = "red";
    m_statusBackColors[MachineState::Door2] = "red";
    m_statusBackColors[MachineState::Door3] = "red";
    m_statusBackColors[MachineState::Jog] = "lime";
    m_statusBackColors[MachineState::Sleep] = "blue";

    m_statusForeColors[MachineState::Unknown] = "white";
    m_statusForeColors[MachineState::Idle] = "palette(text)";
    m_statusForeColors[MachineState::Alarm] = "white";
    m_statusForeColors[MachineState::Run] = "black";
    m_statusForeColors[MachineState::Home] = "black";
    m_statusForeColors[MachineState::Hold0] = "black";
    m_statusForeColors[MachineState::Hold1] = "black";
    m_statusForeColors[MachineState::Queue] = "black";
    m_statusForeColors[MachineState::Check] = "palette(text)";
    m_statusForeColors[MachineState::Door0] = "white";
    m_statusForeColors[MachineState::Door1] = "white";
    m_statusForeColors[MachineState::Door2] = "white";
    m_statusForeColors[MachineState::Door3] = "white";
    m_statusForeColors[MachineState::Jog] = "black";
    m_statusForeColors[MachineState::Sleep] = "white";
}
