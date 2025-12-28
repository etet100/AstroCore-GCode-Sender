#include "partmainoverride.h"
#include "ui_partmainoverride.h"

PartMainOverride::PartMainOverride(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainOverride)
{
    ui->setupUi(this);

    connect(ui->actOverrideSpindleMinus, &QAction::triggered, this, &PartMainOverride::onActSpindleSpeedMinusTriggered);
    connect(ui->actOverrideSpindlePlus, &QAction::triggered, this, &PartMainOverride::onActOverrideSpindlePlusTriggered);
    connect(ui->actOverrideFeedMinus, &QAction::triggered, this, &PartMainOverride::onActOverrideFeedMinusTriggered);
    connect(ui->actOverrideFeedPlus, &QAction::triggered, this, &PartMainOverride::onActOverrideFeedPlusTriggered);
    connect(ui->actOverrideRapidMinus, &QAction::triggered, this, &PartMainOverride::onActOverrideRapidMinusTriggered);
    connect(ui->actOverrideRapidPlus, &QAction::triggered, this, &PartMainOverride::onActOverrideRapidPlusTriggered);

    // Setting up slider boxes
    ui->slbFeed->setRatio(1);
    ui->slbFeed->setMinimum(10);
    ui->slbFeed->setMaximum(200);
    ui->slbFeed->setCurrentValue(100);
    ui->slbFeed->setTitle(tr("Feed rate:"));
    ui->slbFeed->setSuffix("%");
    connect(ui->slbFeed, &SliderBox::toggled, this, &PartMainOverride::onOverridingToggled);
    connect(ui->slbFeed, &SliderBox::valueChanged, this, &PartMainOverride::onValueChanged);

    ui->slbRapid->setRatio(50);
    ui->slbRapid->setMinimum(25);
    ui->slbRapid->setMaximum(100);
    ui->slbRapid->setCurrentValue(100);
    ui->slbRapid->setTitle(tr("Rapid speed:"));
    ui->slbRapid->setSuffix("%");
    connect(ui->slbRapid, &SliderBox::toggled, this, &PartMainOverride::onOverridingToggled);
    connect(ui->slbRapid, &SliderBox::valueChanged, this, &PartMainOverride::onValueChanged);

    ui->slbSpindle->setRatio(1);
    ui->slbSpindle->setMinimum(50);
    ui->slbSpindle->setMaximum(200);
    ui->slbSpindle->setCurrentValue(100);
    ui->slbSpindle->setTitle(tr("Spindle speed:"));
    ui->slbSpindle->setSuffix("%");
    connect(ui->slbSpindle, &SliderBox::toggled, this, &PartMainOverride::onOverridingToggled);
}

PartMainOverride::~PartMainOverride()
{
    delete ui;
}

void PartMainOverride::applyConfiguration(ConfigurationMachine &machineConfiguration)
{
    ui->slbFeed->setChecked(machineConfiguration.overrideFeed());
    ui->slbFeed->setValue(machineConfiguration.overrideFeedValue());

    ui->slbRapid->setChecked(machineConfiguration.overrideRapid());
    ui->slbRapid->setValue(machineConfiguration.overrideRapidValue());

    ui->slbSpindle->setChecked(machineConfiguration.overrideSpindleSpeed());
    ui->slbSpindle->setValue(machineConfiguration.overrideSpindleSpeedValue());
}

PartMainOverride::Overrides PartMainOverride::overrides()
{
    return {
        .feedOverridden = ui->slbFeed->isChecked(),
        .feed =  ui->slbFeed->isChecked() ? ui->slbFeed->value() : NO_OVERRIDE,
        .rapidOverridden = ui->slbRapid->isChecked(),
        .rapid = ui->slbRapid->isChecked() ? ui->slbRapid->value() : NO_OVERRIDE,
        .spindleOverridden = ui->slbSpindle->isChecked(),
        .spindle = ui->slbSpindle->isChecked() ? ui->slbSpindle->value() : NO_OVERRIDE
    };
}

void PartMainOverride::setRapid(int value)
{
    ui->slbRapid->setCurrentValue(value);
}

void PartMainOverride::onActSpindleSpeedMinusTriggered()
{
    ui->slbSpindle->setSliderPosition(ui->slbSpindle->sliderPosition() - 1);
}

void PartMainOverride::onActOverrideFeedPlusTriggered()
{
    ui->slbFeed->setSliderPosition(ui->slbFeed->sliderPosition() + 1);
}

void PartMainOverride::onActOverrideFeedMinusTriggered()
{
    ui->slbFeed->setSliderPosition(ui->slbFeed->sliderPosition() - 1);
}

void PartMainOverride::onActOverrideRapidPlusTriggered()
{
    ui->slbRapid->setSliderPosition(ui->slbRapid->sliderPosition() + 1);
}

void PartMainOverride::onActOverrideRapidMinusTriggered()
{
    ui->slbRapid->setSliderPosition(ui->slbRapid->sliderPosition() - 1);
}

void PartMainOverride::onActOverrideSpindlePlusTriggered()
{
    ui->slbSpindle->setSliderPosition(ui->slbSpindle->sliderPosition() + 1);
}

void PartMainOverride::onActOverrideSpindleMinusTriggered()
{
    ui->slbSpindle->setSliderPosition(ui->slbSpindle->sliderPosition() - 1);
}

void PartMainOverride::onOverridingToggled(bool state)
{
    Q_UNUSED(state);
    emit overrideChanged();
}

void PartMainOverride::onValueChanged()
{
}
