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

void partMainStateLcd::setState(DeviceState state)
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
    m_statusCaptions[DeviceState::Unknown] = tr("Unknown");
    m_statusCaptions[DeviceState::Idle] = tr("Idle");
    m_statusCaptions[DeviceState::Alarm] = tr("Alarm");
    m_statusCaptions[DeviceState::Run] = tr("Run");
    m_statusCaptions[DeviceState::Home] = tr("Home");
    m_statusCaptions[DeviceState::Hold0] = tr("Hold") + " (0)";
    m_statusCaptions[DeviceState::Hold1] = tr("Hold") + " (1)";
    m_statusCaptions[DeviceState::Queue] = tr("Queue");
    m_statusCaptions[DeviceState::Check] = tr("Check");
    m_statusCaptions[DeviceState::Door0] = tr("Door") + " (0)";
    m_statusCaptions[DeviceState::Door1] = tr("Door") + " (1)";
    m_statusCaptions[DeviceState::Door2] = tr("Door") + " (2)";
    m_statusCaptions[DeviceState::Door3] = tr("Door") + " (3)";
    m_statusCaptions[DeviceState::Jog] = tr("Jog");
    m_statusCaptions[DeviceState::Sleep] = tr("Sleep");

    m_statusBackColors[DeviceState::Unknown] = "red";
    m_statusBackColors[DeviceState::Idle] = "palette(button)";
    m_statusBackColors[DeviceState::Alarm] = "red";
    m_statusBackColors[DeviceState::Run] = "lime";
    m_statusBackColors[DeviceState::Home] = "lime";
    m_statusBackColors[DeviceState::Hold0] = "yellow";
    m_statusBackColors[DeviceState::Hold1] = "yellow";
    m_statusBackColors[DeviceState::Queue] = "yellow";
    m_statusBackColors[DeviceState::Check] = "palette(button)";
    m_statusBackColors[DeviceState::Door0] = "red";
    m_statusBackColors[DeviceState::Door1] = "red";
    m_statusBackColors[DeviceState::Door2] = "red";
    m_statusBackColors[DeviceState::Door3] = "red";
    m_statusBackColors[DeviceState::Jog] = "lime";
    m_statusBackColors[DeviceState::Sleep] = "blue";

    m_statusForeColors[DeviceState::Unknown] = "white";
    m_statusForeColors[DeviceState::Idle] = "palette(text)";
    m_statusForeColors[DeviceState::Alarm] = "white";
    m_statusForeColors[DeviceState::Run] = "black";
    m_statusForeColors[DeviceState::Home] = "black";
    m_statusForeColors[DeviceState::Hold0] = "black";
    m_statusForeColors[DeviceState::Hold1] = "black";
    m_statusForeColors[DeviceState::Queue] = "black";
    m_statusForeColors[DeviceState::Check] = "palette(text)";
    m_statusForeColors[DeviceState::Door0] = "white";
    m_statusForeColors[DeviceState::Door1] = "white";
    m_statusForeColors[DeviceState::Door2] = "white";
    m_statusForeColors[DeviceState::Door3] = "white";
    m_statusForeColors[DeviceState::Jog] = "black";
    m_statusForeColors[DeviceState::Sleep] = "white";
}
