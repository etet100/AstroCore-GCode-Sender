#include "partmainstate.h"
#include "ui_partmainstate.h"

partMainState::partMainState(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainState)
    // , m_configuration(configuration)
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
    ui->txtStatus->setStyleSheet(QString("background-color: %1; color: %2;")
                                     .arg(bgColor, fgColor));

}

void partMainState::setState(MachineState state)
{
    // setStatusText(m_statusCaptions[state], m_statusBackColors[state], m_statusForeColors[state]);
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
    // int bound = m_settings->units() == 0 ? 9999 : 999;

    ui->txtMX->setDecimals(prec);
    // @TODO what are these for?
    // ui->txtMPosX->setMinimum(-bound);
    // ui->txtMPosX->setMaximum(bound);
    ui->txtMY->setDecimals(prec);
    // @TODO what are these for?
    // ui->txtMPosY->setMinimum(-bound);
    // ui->txtMPosY->setMaximum(bound);
    ui->txtMZ->setDecimals(prec);
    // ui->txtMPosZ->setMinimum(-bound);
    // ui->txtMPosZ->setMaximum(bound);

    ui->txtWX->setDecimals(prec);
    // ui->txtWPosX->setMinimum(-bound);
    // ui->txtWPosX->setMaximum(bound);
    ui->txtWY->setDecimals(prec);
    // ui->txtWPosY->setMinimum(-bound);
    // ui->txtWPosY->setMaximum(bound);
    ui->txtWZ->setDecimals(prec);
    // ui->txtWPosZ->setMinimum(-bound);
    // ui->txtWPosZ->setMaximum(bound);
}

void partMainState::initializeColorsAndCaptions()
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
