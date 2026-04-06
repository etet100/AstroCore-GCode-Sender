
#include "partmainstatelcd.h"
#include "ui_partmainstatelcd.h"
#include "utils/utils.h"
#include <QFontDatabase>
#include <QPushButton>
#include "ui/utils/thememanager.h"

PartMainStateLcd::PartMainStateLcd(QWidget *parent)
    : PartMainStateBase(parent)
    , ui(new Ui::partMainStateLcd)
{
    ui->setupUi(this);

    ui->txtMachineInfo->setVisible(false);
    connect(ui->chkMachineInfo, &XSwitchButtonWithLabel::stateChanged, this, [this](bool state) {
        ui->txtMachineInfo->setVisible(state);
    });

    initializeColorsAndCaptions();
    setWorkCoordinates(QVector3D(0, 0, 0));
    setMachineCoordinates(QVector3D(0, 0, 0));
    setConnectionState(false);

    int fontId = QFontDatabase::addApplicationFont(":/fonts/Patopian1986.ttf");
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    }

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        this->resizeEvent(nullptr);
    });

    connect(ui->btnConnection, &QPushButton::clicked, this, [this]() {
        if (ui->btnConnection->property("connected").toBool() == false) {
            emit connectClicked();
        } else {
            emit disconnectClicked();
        }
    });
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

void PartMainStateLcd::setConnectionName(QString name)
{
    ui->btnConnection->setToolTip(name);
}

void PartMainStateLcd::setConnectionState(bool connected)
{
    QColor color = connected ? QColor("lightgreen") : QColor("lightcoral");
    QString image;
    if (connected) {
        image = ":/images/conn/connected.svg";
    } else {
        image = ":/images/conn/disconnected.svg";
    }
    ui->btnConnection->setBackColor(color);
    ui->btnConnection->setProperty("connected", connected);
    ui->btnConnection->setIcon(QIcon(image));
    // ui->btnConnection->setStyleSheet(QString("color: white; border: 0px; background-color: %1;").arg(color));
}

void PartMainStateLcd::setMachineStateReport(QString report)
{
    ui->txtMachineInfo->setText(report);
    ui->txtMachineInfo->adjustSize();
    Utils::refreshStyle({ui->txtMachineInfo, this});
}

void PartMainStateLcd::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    // Calculating appropriate font size based on the width of the LCD displays

    int lcdWidth = ui->txtWX->width();

    const QString sampleText = "-000.000";
    const int charCount = sampleText.length();

    int fontSize = qMax(12, qMin(31, static_cast<int>(lcdWidth / charCount * 2.1)));

    QPalette pal = qApp->palette();
    QColor color("#96DFCF"); // text color
    QColor bgColor = pal.color(QPalette::Button);
    if (!ThemeManager::instance().dark()) {
        bgColor = bgColor.darker(110);
        color = color.darker(300);
    }

    QString styleSheet = QString("font: %1pt \"Patopian 1986\"; "
                                 "color: " + color.name() +"; background: " + bgColor.name() + "; "
                                 "border: 1px solid palette(window); border-radius: 4px;").arg(fontSize);
    qDebug() << styleSheet;

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

