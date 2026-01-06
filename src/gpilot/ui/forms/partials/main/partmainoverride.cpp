#include "partmainoverride.h"
#include "ui_partmainoverride.h"

PartMainOverride::PartMainOverride(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainOverride)
{
    ui->setupUi(this);

    connect(ui->actOverrideSpindleMinus, &QAction::triggered, this, &PartMainOverride::onSpindleMinusTriggered);
    connect(ui->actOverrideSpindlePlus, &QAction::triggered, this, &PartMainOverride::onSpindlePlusTriggered);
    connect(ui->actOverrideFeedMinus, &QAction::triggered, this, &PartMainOverride::onFeedMinusTriggered);
    connect(ui->actOverrideFeedPlus, &QAction::triggered, this, &PartMainOverride::onFeedPlusTriggered);
    connect(ui->actOverrideRapidMinus, &QAction::triggered, this, &PartMainOverride::onRapidMinusTriggered);
    connect(ui->actOverrideRapidPlus, &QAction::triggered, this, &PartMainOverride::onRapidPlusTriggered);

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

void PartMainOverride::setCurrentRapid(int value)
{
    ui->slbRapid->setCurrentValue(value);
}

void PartMainOverride::setCurrentFeed(int value)
{
    ui->slbFeed->setCurrentValue(value);
}

void PartMainOverride::setCurrentSpindle(int value)
{
    ui->slbSpindle->setCurrentValue(value);
}

int PartMainOverride::targetFeed()
{
    return ui->slbFeed->isChecked() ? ui->slbFeed->value() : NO_OVERRIDE;
}

int PartMainOverride::targetRapid()
{
    return ui->slbRapid->isChecked() ? ui->slbRapid->value() : NO_OVERRIDE;
}

int PartMainOverride::targetSpindle()
{
    return ui->slbSpindle->isChecked() ? ui->slbSpindle->value() : NO_OVERRIDE;
}

bool PartMainOverride::feedOverridden()
{
    return ui->slbFeed->isChecked();
}

bool PartMainOverride::rapidOverridden()
{
    return ui->slbRapid->isChecked();
}

bool PartMainOverride::spindleOverridden()
{
    return ui->slbSpindle->isChecked();
}

void PartMainOverride::onFeedPlusTriggered()
{
    ui->slbFeed->setSliderPosition(ui->slbFeed->sliderPosition() + 1);
}

void PartMainOverride::onFeedMinusTriggered()
{
    ui->slbFeed->setSliderPosition(ui->slbFeed->sliderPosition() - 1);
}

void PartMainOverride::onRapidPlusTriggered()
{
    ui->slbRapid->setSliderPosition(ui->slbRapid->sliderPosition() + 1);
}

void PartMainOverride::onRapidMinusTriggered()
{
    ui->slbRapid->setSliderPosition(ui->slbRapid->sliderPosition() - 1);
}

void PartMainOverride::onSpindlePlusTriggered()
{
    ui->slbSpindle->setSliderPosition(ui->slbSpindle->sliderPosition() + 1);
}

void PartMainOverride::onSpindleMinusTriggered()
{
    ui->slbSpindle->setSliderPosition(ui->slbSpindle->sliderPosition() - 1);
}

void PartMainOverride::onOverridingToggled(bool state)
{
    Q_UNUSED(state);

    emitOverrideChanged();
}

void PartMainOverride::onValueChanged()
{
    emitOverrideChanged();
}

void PartMainOverride::emitOverrideChanged()
{
    emit overrideChanged(
        ui->slbFeed->isChecked(),
        ui->slbFeed->isChecked() ? ui->slbFeed->value() : NO_OVERRIDE,
        ui->slbRapid->isChecked(),
        ui->slbRapid->isChecked() ? ui->slbRapid->value() : NO_OVERRIDE,
        ui->slbSpindle->isChecked(),
        ui->slbSpindle->isChecked() ? ui->slbSpindle->value() : NO_OVERRIDE
    );
}
