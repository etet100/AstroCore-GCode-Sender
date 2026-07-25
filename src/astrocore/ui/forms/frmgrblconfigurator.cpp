#include "core/globals.h"
#include "ui/forms/frmgrblconfigurator.h"
#include "ui_frmgrblconfigurator.h"
#include "ui/config/configurationui.h"
#include "utils/utils.h"
#include <QMessageBox>
#include <CPropertyEditor.h>
#include <CPropertyHeader.h>
#include <CStringProperty.h>
#include <CSwitchProperty.h>
#include <CIntegerProperty.h>
#include <QCoroSignal>
#include <CColorProperty.h>
#include <CPropertyHeader.h>
#include <CSwitchProperty.h>
#include <CSwitchProperty.h>
#include <CDoubleProperty.h>
#include <CListProperty.h>
#include <CFontProperty.h>

enum class ConfigGroupEnum {
    General,
    StepInvert,
    DirInvert,
    Limits,
    Homing,
    HomingInvert,
    Spindle,
    Steps,
    MaxRate,
    Acceleration,
    Reporting,
    MaxTravel,
    Quality,
};

struct ConfigGroup {
    ConfigGroupEnum group;
    QColor color;
    QByteArray name;
    std::vector<ConfigEntry> entries;
    CPropertyHeader* header = nullptr;
};

#define REPORT_POS_TYPE_SETTING_ID 150
#define REPORT_POS_TYPE_SETTING_BIT 1
#define REPORT_BUFFER_SETTING_ID 151
#define REPORT_BUFFER_SETTING_BIT 2

// $0;Step pulse, microseconds; Shortest pulses for stepper drivers to recognize.
// $1;Step idle delay, milliseconds; Delay before disabling steppers after motion.
// $2;Step port invert, mask; Inverts the step pulse signa.
// $3;Direction port invert, mask; Inverts the direction signal for each axis.
// $4;Step enable invert, boolean; Inverts the stepper enable pin.
// $5;Limit pins invert, boolean; Inverts the limit pins.
// $6;Probe pin invert, boolean; Inverts the probe pin.
// $10;Status report, mask; Determines real-time data reported back.
// $11;Junction deviation, mm; Determines cornering speed.
// $12;Arc tolerance, mm; Accuracy of arc tracing.
// $13;Report inches, boolean; Toggles between mm and inches for reporting.
// $20;Soft limits, boolean; Prevents machine from traveling beyond limits.
// $21;Hard limits, boolean; Uses physical switches to prevent over-travel6.
// $22;Homing cycle, boolean; Locates a consistent position on startup.
// $23;Homing dir invert, mask; Inverts homing direction for certain axes.
// $24;Homing feed, mm/min; Feed rate for precise homing location.
// $25;Homing seek, mm/min; Search rate for homing cycle.
// $26;Homing debounce, milliseconds; Debounce delay for homing switches.
// $27;Homing pull-off, mm; Distance to pull off from limit switches after homing.
// $30;Max spindle speed, RPM; Sets max spindle speed for 5V PWM output.
// $31;Min spindle speed, RPM; Sets min spindle speed for 0.02V PWM output.
// $32;Laser mode, boolean; Enables continuous motion for laser cutting.
// $100, $101, $102;[X,Y,Z] steps/mm; Steps per mm for each axis.
// $110, $111, $112;[X,Y,Z] Max rate, mm/min; Max rate each axis can move.
// $120, $121, $122;[X,Y,Z] Acceleration, mm/sec^2; Acceleration parameters for each axis.
// $130, $131, $132;[X,Y,Z] Max travel, mm; Max travel for each axis

// const std::vector<QColor> COLORS = {
//     QColor(255, 204, 204), // Różowy pastelowy
//     QColor(204, 255, 204), // Zielony pastelowy
//     QColor(204, 204, 255), // Niebieski pastelowy
//     QColor(255, 255, 204), // Żółty pastelowy
//     QColor(255, 204, 255), // Fioletowy pastelowy
//     QColor(204, 255, 255), // Turkusowy pastelowy
//     QColor(255, 230, 204), // Pomarańczowy pastelowy
//     QColor(240, 240, 240), // Szary pastelowy
//     QColor(255, 221, 204), // Brzoskwiniowy pastelowy
//     QColor(204, 255, 230), // Jasnozielony pastelowy
//     QColor(230, 204, 255), // Jasnofioletowy pastelowy
//     QColor(255, 204, 230), // Liliowy pastelowy
//     QColor(204, 255, 255), // Błękitny pastelowy
//     QColor(255, 204, 230), // Różowy pastelowy
//     QColor(255, 255, 255), // Biały
// };

// const std::vector<QColor> COLORS = {
//     QColor(255, 187, 187), // Ciemnoróżowy
//     QColor(187, 255, 187), // Ciemnozielony
//     QColor(187, 187, 255), // Ciemnoniebieski
//     QColor(255, 255, 187), // Jasnożółty
//     QColor(255, 187, 255), // Ciemnofioletowy
//     QColor(187, 255, 255), // Jasnoturkusowy
//     QColor(255, 212, 187), // Brązowy
//     QColor(224, 224, 224), // Jasnoszary
//     QColor(255, 204, 187), // Brzoskwiniowy
//     QColor(187, 255, 212), // Jasnyzielony
//     QColor(212, 187, 255), // Jasnyfioletowy
//     QColor(255, 187, 212), // Bladyliliowy
//     QColor(187, 255, 255), // Jasnybłękitny
//     QColor(255, 187, 212), // Jasnoróżowy
//     QColor(245, 245, 245), // Jasnobiały
// };

const std::vector<QColor> COLORS = {
    QColor(204, 102, 102), // Ciemnoczerwony
    QColor(102, 204, 102), // Ciemnozielony
    QColor(102, 102, 204), // Ciemnoniebieski
    QColor(204, 204, 102), // Jasnożółty
    QColor(204, 102, 204), // Ciemnofioletowy
    QColor(102, 204, 204), // Jasnoturkusowy
    QColor(204, 153, 102), // Brązowy
    QColor(192, 152, 192), // Jasnoszary (zm.)
    QColor(102, 204, 153), // Jasnyzielony
    QColor(153, 102, 204), // Jasnyfioletowy
    QColor(204, 153, 102), // Brzoskwiniowy
    QColor(204, 102, 153), // Bladyliliowy
    QColor(102, 204, 204), // Jasnybłękitny
    QColor(204, 102, 153), // Jasnoczerwony
    QColor(230, 230, 230), // Bardzo jasnoszary
};

std::vector<ConfigGroup> ConfigMap = {
    {
        ConfigGroupEnum::General,
        COLORS[0],
        "General settings",
        {
            {0, "Step pulse, microseconds", "Shortest pulses for stepper drivers to recognize.", Integer},
            {1, "Step idle delay, milliseconds", "Delay before disabling steppers after motion.", Integer},
            {32, "Laser mode", "Enables continuous motion for laser cutting.", Boolean},
            //uCNC uses mask??
            {4, "Step enable invert", "Inverts the stepper enable pin.", Boolean},
            //uCNC uses mask??
            {5, "Limit pins invert", "Inverts the limit pins.", Boolean},
            {6, "Probe pin invert", "Inverts the probe pin.", Boolean},
        }
    },
    {
        ConfigGroupEnum::Reporting,
        COLORS[1],
        "Reporting machine status",
        {
             {REPORT_POS_TYPE_SETTING_ID , "Report position type", "Enabled `MPos`, Disabled `WPos`", Boolean},
             {REPORT_BUFFER_SETTING_ID, "Report buffer data", "Enabled `Buf` field appears with planner and serial RX available buffer.", Boolean},
             {13, "Report inches", "Toggles between mm and inches for reporting.", Boolean},
        }
    },
    {
        ConfigGroupEnum::Quality,
        COLORS[2],
        "Quality",
        {
            {11, "Junction deviation, mm", "Determines cornering speed.", Double},
            {12, "Arc tolerance, mm", "Accuracy of arc tracing.", Double},
        }
    },
    {
        ConfigGroupEnum::Limits,
        COLORS[3],
        "Limits",
        {
             {20, "Soft limits", "Prevents machine from traveling beyond limits.", Boolean},
             {21, "Hard limits", "Uses physical switches to prevent over-travel.", Boolean},
        },
    },
    {
        ConfigGroupEnum::StepInvert,
        COLORS[4],
        "Step signal polarity",
        {
            {2, "%1 step signal invert", "Inverts the step pulse signal.", Axes},
        }
    },
    {
        ConfigGroupEnum::DirInvert,
        COLORS[5],
        "Dir signal polarity",
        {
            {3, "%1 direction signal invert", "Inverts the direction signal for each axis.", Axes},
        }
    },
    {
        ConfigGroupEnum::Homing,
        COLORS[6],
        "Homing settings",
        {
            {22, "Homing cycle", "Locates a consistent position on startup.", Boolean},
            {24, "Homing feed, mm/min", "Feed rate for precise homing location.", Integer},
            {25, "Homing seek, mm/min", "Search rate for homing cycle.", Integer},
            {26, "Homing debounce, milliseconds", "Debounce delay for homing switches.", Integer},
            {27, "Homing pull-off, mm", "Distance to pull off from limit switches after homing.", Integer},
        }
    },
    {
        ConfigGroupEnum::HomingInvert,
        COLORS[7],
        "Homing signal polarity",
        {
            {23, "%1 homing dir invert", "Inverts homing direction for certain axes.", Axes},
        }
    },
    {
        ConfigGroupEnum::Spindle,
        COLORS[8],
        "Spindle",
        {
            {30, "Max spindle speed, RPM", "Sets max spindle speed for 5V PWM output.", Integer},
            {31, "Min spindle speed, RPM", "Sets min spindle speed for 0.02V PWM output.", Integer},
        }
    },
    {
        ConfigGroupEnum::Steps,
        COLORS[9],
        "Steps per mm",
        {
            {100, "X steps/mm", "Steps per mm for X axis.", Integer},
            {101, "Y steps/mm", "Steps per mm for Y axis.", Integer},
            {102, "Z steps/mm", "Steps per mm for Z axis.", Integer},
        }
    },
    {
        ConfigGroupEnum::MaxRate,
        COLORS[10],
        "Max axis rate",
        {
            {110, "X max rate, mm/min", "Max rate X axis can move.", Integer},
            {111, "Y max rate, mm/min", "Max rate Y axis can move.", Integer},
            {112, "Z max rate, mm/min", "Max rate Z axis can move.", Integer},
        }
    },
    {
        ConfigGroupEnum::Acceleration,
        COLORS[11],
        "Max axis acceleration",
        {
            {120, "X acceleration, mm/sec^2", "Acceleration parameters for X axis.", Integer},
            {121, "Y acceleration, mm/sec^2", "Acceleration parameters for Y axis.", Integer},
            {122, "Z acceleration, mm/sec^2", "Acceleration parameters for Z axis.", Integer},
        }
    },
    {
        ConfigGroupEnum::MaxTravel,
        COLORS[12],
        "Max axis travel",
        {
            {130, "X max travel, mm", "Max travel for X axis.", Integer},
            {131, "Y max travel, mm", "Max travel for Y axis.", Integer},
            {132, "Z max travel, mm", "Max travel for Z axis.", Integer},
        }
    }
};

FrmGrblConfigurator::FrmGrblConfigurator(QWidget *parent, ConfigurationUI &uiConfiguration, Communicator *communicator)
    : QDialog(parent)
    , ui(new Ui::frmGrblConfigurator)
    , m_uiConfiguration(uiConfiguration)
    , m_communicator(communicator)
{
    ui->setupUi(this);
    ui->editor->init();

    for (std::vector<ConfigGroup>::iterator it = ConfigMap.begin(); it != ConfigMap.end(); it++) {
        ConfigGroup &group = *it;

        CPropertyHeader* header = new CPropertyHeader(group.name, group.name);
        ui->editor->add(header);
        header->setBackground(group.color);

        QFont font = header->font(0);
        font.setBold(true);
        header->setFont(0, font);

        for (std::vector<ConfigEntry>::iterator it2 = group.entries.begin(); it2 != group.entries.end(); it2++) {
            ConfigEntry &entry = *it2;
            switch (entry.type) {
                case Axes:
                    entry.properties = addAxesProperty(header, entry);
                    break;
                case Boolean:
                    entry.property = addBooleanProperty(header, entry);
                    break;
                case Integer:
                    entry.property = addIntegerProperty(header, entry);
                    break;
                case Double:
                    entry.property = addDoubleProperty(header, entry);
                    break;
            }
        }

        group.header = header;
    }

    ui->editor->adjustToContents();

    connect(ui->editor, &QTreeWidget::itemChanged, this, &FrmGrblConfigurator::itemChanged);

    m_activeTask = update();
}

FrmGrblConfigurator::~FrmGrblConfigurator()
{
    delete ui;
}

void FrmGrblConfigurator::showEvent(QShowEvent *se)
{
    QDialog::showEvent(se);
    if (m_firstShow) {
        Utils::positionDialog(this, m_uiConfiguration.grblConfigratorFormGeometry(), m_uiConfiguration.grblConfigratorFormMaximized());
        m_firstShow = false;
    }
}

void FrmGrblConfigurator::resizeEvent(QResizeEvent *re)
{
    QDialog::resizeEvent(re);
    if (!m_firstShow) {
        m_uiConfiguration.setGrblConfigratorFormGeometry(this);
    }
}

void FrmGrblConfigurator::changeEvent(QEvent *ce)
{
    QDialog::changeEvent(ce);
    if (ce->type() == QEvent::WindowStateChange) {
        m_uiConfiguration.setGrblConfigratorFormGeometry(this);
    }
}

void FrmGrblConfigurator::moveEvent(QMoveEvent *me)
{
    QDialog::moveEvent(me);
    if (!m_firstShow) {
        m_uiConfiguration.setGrblConfigratorFormGeometry(this);
    }
}

void FrmGrblConfigurator::onUpdateClicked()
{
    m_activeTask = update();
}

void FrmGrblConfigurator::itemChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    CBaseProperty *property = dynamic_cast<CBaseProperty*>(item);
    if (property == nullptr) {
        return;
    }

    int entryIndex = property->data(0, Qt::UserRole).toInt();
    Axis axis = Axis::None;
    if (entryIndex > 1000) {
        axis = Axis(entryIndex % 10000);
        entryIndex = entryIndex / 10000;
    }

    if (entryIndex == REPORT_POS_TYPE_SETTING_ID) {
        setSettingsBit(10, REPORT_POS_TYPE_SETTING_BIT, property->getVariantValue().toBool());
    } else
    if (entryIndex == REPORT_BUFFER_SETTING_ID) {
        setSettingsBit(10, REPORT_BUFFER_SETTING_BIT, property->getVariantValue().toBool());
    } else
    if (axis != Axis::None) {
        setSettingsBit(entryIndex, 1 << (int) axis, property->getVariantValue().toBool());
    } else {
        m_currentSettings[entryIndex] = property->getVariantValue().toDouble();
    }
}

QCoro::Task<void> FrmGrblConfigurator::update()
{
    setInfo("Updating...", Qt::blue);
    ui->btnRefresh->setEnabled(false);

    QTimer delay;
    delay.setSingleShot(true);
    delay.start(250);
    co_await qCoro(&delay, &QTimer::timeout);

    m_communicator->stateBehavior()->action(Action::QueryMachineConfiguration);

    auto result = co_await qCoro(
        m_communicator,
        &Communicator::machineConfigurationReceived,
        std::chrono::milliseconds{1000}
    );

    if (!result) {
        setInfo("Error: No response from machine", Qt::red);
        ui->btnRefresh->setEnabled(true);

        co_return;
    }

    QMap<int, double> rawConfiguration = result->raw();
    m_currentSettings = rawConfiguration;

    setInfo("Updated");
    qDebug() << "[GRBLConfigurator] Settings received" << rawConfiguration;

    // ugly hack! split status report setting into two separate settings
    int statusReportSetting = rawConfiguration[10];
    rawConfiguration[REPORT_BUFFER_SETTING_ID] = statusReportSetting & REPORT_BUFFER_SETTING_BIT;
    rawConfiguration[REPORT_POS_TYPE_SETTING_ID] = statusReportSetting & REPORT_POS_TYPE_SETTING_BIT;

    for (std::vector<ConfigGroup>::iterator it = ConfigMap.begin(); it != ConfigMap.end(); it++) {
        ConfigGroup &group = *it;
        for (std::vector<ConfigEntry>::iterator it2 = group.entries.begin(); it2 != group.entries.end(); it2++) {
            ConfigEntry entry = *it2;

            double setting = rawConfiguration[entry.index];

            QMap<Axis, CBaseProperty*> properties;
            switch (entry.type) {
                case Axes:
                    properties = entry.properties;
                    (dynamic_cast<CSwitchProperty*>(properties[Axis::X]))->setValue(((int) setting) & 1);
                    (dynamic_cast<CSwitchProperty*>(properties[Axis::Y]))->setValue(((int) setting) & 2);
                    (dynamic_cast<CSwitchProperty*>(properties[Axis::Z]))->setValue(((int) setting) & 4);
                    break;
                case Boolean:
                    CSwitchProperty *boolProperty;
                    boolProperty = dynamic_cast<CSwitchProperty*>(entry.property);
                    boolProperty->setValue((bool) setting);
                    break;
                case Integer:
                    CIntegerProperty *intProperty;
                    intProperty = dynamic_cast<CIntegerProperty*>(entry.property);
                    intProperty->setValue((int) setting);
                    break;
                case Double:
                    CDoubleProperty *doubleProperty;
                    doubleProperty = dynamic_cast<CDoubleProperty*>(entry.property);
                    doubleProperty->setValue(setting);
                    break;
            }
        }
    }

    ui->btnRefresh->setEnabled(true);
}

void FrmGrblConfigurator::setInfo(QString text, QColor color)
{
    ui->lblInfo->setText(text);
    if (color == Qt::transparent) {
        ui->lblInfo->setStyleSheet("");
    } else {
        ui->lblInfo->setStyleSheet(QString("color: %1").arg(color.name()));
    }
}

QMap<Axis, CBaseProperty*> FrmGrblConfigurator::addAxesProperty(CPropertyHeader *header, ConfigEntry entry)
{
    return {
        {Axis::X, addAxisProperty(header, entry, Axis::X)},
        {Axis::Y, addAxisProperty(header, entry, Axis::Y)},
        {Axis::Z, addAxisProperty(header, entry, Axis::Z)},
    };
}

CBaseProperty* FrmGrblConfigurator::addAxisProperty(CPropertyHeader *header, ConfigEntry entry, Axis axis)
{
    // @TODO ...
    QMap<Axis, QString> axisNames = {
        {Axis::X, "X"},
        {Axis::Y, "Y"},
        {Axis::Z, "Z"},
    };

    CSwitchProperty* property = new CSwitchProperty(
        header,
        QByteArray::fromStdString(QString("#%1_%2").arg(entry.index).arg((int) axis).toStdString()),
        entry.description.arg(axisNames[axis]),
        0,
        0
    );
    property->setData(0, Qt::UserRole, entry.index * 10000 + (int) axis);

    ui->editor->add(property);

    return property;
}

void FrmGrblConfigurator::setSettingsBit(int index, int bit, bool bitValue)
{
    int value = m_currentSettings[index];
    if (bitValue) {
        value |= bit;
    } else {
        value &= ~bit;
    }
    m_currentSettings[index] = value;
}

void FrmGrblConfigurator::accept()
{
    if (QMessageBox::question(this, "Saving settings", "Are you sure you want to update machine settings?") == QMessageBox::No) {
        return;
    }

    m_activeTask = save();
}

QCoro::Task<void> FrmGrblConfigurator::save()
{
    setInfo("Reading current settings...", Qt::blue);
    ui->btnRefresh->setEnabled(false);

    QTimer delay;
    delay.setSingleShot(true);
    delay.start(250);
    co_await qCoro(&delay, &QTimer::timeout);

    m_communicator->stateBehavior()->action(Action::QueryMachineConfiguration);

    auto result = co_await qCoro(
        m_communicator,
        &Communicator::machineConfigurationReceived,
        std::chrono::milliseconds{1000}
    );

    ui->btnRefresh->setEnabled(true);

    if (!result) {
        setInfo("Error: No response from machine", Qt::red);

        co_return;
    }

    QMap<int, double> rawConfiguration = result->raw();
    findParametersToBeSaved(rawConfiguration);
}

void FrmGrblConfigurator::findParametersToBeSaved(QMap<int, double> settings)
{
    QMap<int, double> toBeSaved = {};
    for (QMap<int, double>::iterator it = m_currentSettings.begin(); it != m_currentSettings.end(); it++) {
        int index = it.key();
        double value = it.value();

        if (settings[index] != value) {
            toBeSaved[index] = value;
        }
    }

    if (toBeSaved.size() == 0) {
        QDialog::accept();

        return;
    }

    for (QMap<int, double>::iterator it = toBeSaved.begin(); it != toBeSaved.end(); it++) {
        m_communicator->stateBehavior()->action(SaveMachineConfigurationParamAction(it.key(), it.value()));
        // QString command = QString("$%1=%2").arg(it.key()).arg(it.value());
        // qDebug() << (int) m_communicator->sendCommand(CommandSource::System, command);
        // qDebug() << "[GRBLConfigurator] Sending command" << command;
    }

    QDialog::accept();
}

CBaseProperty* FrmGrblConfigurator::addBooleanProperty(CPropertyHeader *header, ConfigEntry entry)
{
    CSwitchProperty* property = new CSwitchProperty(
        header,
        QByteArray::fromStdString(QString("#%1").arg(entry.index).toStdString()),
        entry.description,
        0,
        0
    );
    property->setData(0, Qt::UserRole, entry.index);

    ui->editor->add(property);

    return property;
}

CBaseProperty* FrmGrblConfigurator::addIntegerProperty(CPropertyHeader *header, ConfigEntry entry)
{
    CIntegerProperty* property = new CIntegerProperty(
        header,
        QByteArray::fromStdString(QString("#%1").arg(entry.index).toStdString()),
        entry.description,
        0,
        0
    );
    property->setData(0, Qt::UserRole, entry.index);

    ui->editor->add(property);

    return property;
}

CBaseProperty *FrmGrblConfigurator::addDoubleProperty(CPropertyHeader *header, ConfigEntry entry)
{
    CDoubleProperty* property = new CDoubleProperty(
        header,
        QByteArray::fromStdString(QString("#%1").arg(entry.index).toStdString()),
        entry.description,
        0,
        0
    );

    property->setData(0, Qt::UserRole, entry.index);

    ui->editor->add(property);

    return property;
}
