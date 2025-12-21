
#include "partmainstatelcd.h"
#include "ui_partmainstatelcd.h"
#include <QFontDatabase>

PartMainStateLcd::PartMainStateLcd(QWidget *parent)
    : PartMainStateBase(parent)
    , ui(new Ui::partMainStateLcd)
{
    ui->setupUi(this);
    initializeColorsAndCaptions();
    setWorkCoordinates(QVector3D(0, 0, 0));
    setMachineCoordinates(QVector3D(0, 0, 0));

    int fontId = QFontDatabase::addApplicationFont(":/fonts/Patopian1986.ttf");
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    }
}

PartMainStateLcd::~PartMainStateLcd()
{
    delete ui;
}

void PartMainStateLcd::setStatusText(QString status, QString bgColor, QString fgColor)
{
    ui->txtStatus->setText(status);
    ui->txtStatus->setStyleSheet(QString("background-color: %1; color: %2;").arg(bgColor, fgColor));
}

void PartMainStateLcd::setConName(QString name)
{
    ui->txtConName->setText(name);
}

void PartMainStateLcd::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    // Calculating appropriate font size based on the width of the LCD displays

    int lcdWidth = ui->txtWX->width();

    const QString sampleText = "-000.000";
    const int charCount = sampleText.length();

    int fontSize = qMax(12, qMin(31, static_cast<int>(lcdWidth / charCount * 2.1)));

    //

    QString styleSheet = QString("font: %1pt \"Patopian 1986\";").arg(fontSize);

    ui->txtWX->setStyleSheet(styleSheet);
    ui->txtWY->setStyleSheet(styleSheet);
    ui->txtWZ->setStyleSheet(styleSheet);
    ui->txtMX->setStyleSheet(styleSheet);
    ui->txtMY->setStyleSheet(styleSheet);
    ui->txtMZ->setStyleSheet(styleSheet);
}

void PartMainStateLcd::setState(MachineState state)
{
    Q_UNUSED(state);
}

QString PartMainStateLcd::formatPos(float val)
{
    return QString("%1").arg(val, 0, 'f', 3).rightJustified(6, ' ');
}

void PartMainStateLcd::setWorkCoordinates(QVector3D pos)
{
    ui->txtWX->setText(formatPos(pos.x()));
    ui->txtWY->setText(formatPos(pos.y()));
    ui->txtWZ->setText(formatPos(pos.z()));
}

void PartMainStateLcd::setMachineCoordinates(QVector3D pos)
{
    ui->txtMX->setText(formatPos(pos.x()));
    ui->txtMY->setText(formatPos(pos.y()));
    ui->txtMZ->setText(formatPos(pos.z()));
}

void PartMainStateLcd::setUnits(Units units)
{
    // int prec = units == Units::Millimeters ? 3 : 4;
    // Q_UNUSED(prec);
}

