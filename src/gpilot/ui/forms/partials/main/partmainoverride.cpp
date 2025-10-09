#include "partmainoverride.h"
#include "ui_partmainoverride.h"

partMainOverride::partMainOverride(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainOverride)
{
    ui->setupUi(this);

    connect(ui->actOverrideSpindleMinus, &QAction::triggered, this, &partMainOverride::onActSpindleSpeedMinusTriggered);
    connect(ui->actOverrideSpindlePlus, &QAction::triggered, this, &partMainOverride::onActOverrideSpindlePlusTriggered);
    connect(ui->actOverrideFeedMinus, &QAction::triggered, this, &partMainOverride::onActOverrideFeedMinusTriggered);
    connect(ui->actOverrideFeedPlus, &QAction::triggered, this, &partMainOverride::onActOverrideFeedPlusTriggered);
    connect(ui->actOverrideRapidMinus, &QAction::triggered, this, &partMainOverride::onActOverrideRapidMinusTriggered);
    connect(ui->actOverrideRapidPlus, &QAction::triggered, this, &partMainOverride::onActOverrideRapidPlusTriggered);

    // Setting up slider boxes
    ui->slbFeed->setRatio(1);
    ui->slbFeed->setMinimum(10);
    ui->slbFeed->setMaximum(200);
    ui->slbFeed->setCurrentValue(100);
    ui->slbFeed->setTitle(tr("Feed rate:"));
    ui->slbFeed->setSuffix("%");
    // connect(ui->slbFeed, SIGNAL(toggled(bool)), this, SLOT(onOverridingToggled(bool)));
    connect(ui->slbFeed, &SliderBox::toggled, this, &partMainOverride::onOverrideChanged);
    connect(ui->slbFeed, &SliderBox::valueChanged, this, &partMainOverride::onOverrideChanged);

    ui->slbRapid->setRatio(50);
    ui->slbRapid->setMinimum(25);
    ui->slbRapid->setMaximum(100);
    ui->slbRapid->setCurrentValue(100);
    ui->slbRapid->setTitle(tr("Rapid speed:"));
    ui->slbRapid->setSuffix("%");
    connect(ui->slbRapid, SIGNAL(toggled(bool)), this, SLOT(onOverridingToggled(bool)));
    connect(ui->slbRapid, &SliderBox::toggled, this, &partMainOverride::onOverrideChanged);
    connect(ui->slbRapid, &SliderBox::valueChanged, this, &partMainOverride::onOverrideChanged);

    ui->slbSpindle->setRatio(1);
    ui->slbSpindle->setMinimum(50);
    ui->slbSpindle->setMaximum(200);
    ui->slbSpindle->setCurrentValue(100);
    ui->slbSpindle->setTitle(tr("Spindle speed:"));
    ui->slbSpindle->setSuffix("%");
    connect(ui->slbSpindle, SIGNAL(toggled(bool)), this, SLOT(onOverridingToggled(bool)));
}

partMainOverride::~partMainOverride()
{
    delete ui;
}

void partMainOverride::applyConfiguration(ConfigurationMachine &machineConfiguration)
{
    ui->slbFeed->setChecked(machineConfiguration.overrideFeed());
    ui->slbFeed->setValue(machineConfiguration.overrideFeedValue());

    ui->slbRapid->setChecked(machineConfiguration.overrideRapid());
    ui->slbRapid->setValue(machineConfiguration.overrideRapidValue());

    ui->slbSpindle->setChecked(machineConfiguration.overrideSpindleSpeed());
    ui->slbSpindle->setValue(machineConfiguration.overrideSpindleSpeedValue());
}

partMainOverride::Overrides partMainOverride::overrides()
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

void partMainOverride::setRapid(int value)
{
    ui->slbRapid->setCurrentValue(value);
}

void partMainOverride::onActSpindleSpeedMinusTriggered()
{
    ui->slbSpindle->setSliderPosition(ui->slbSpindle->sliderPosition() - 1);
}

void partMainOverride::onActOverrideFeedPlusTriggered()
{
    ui->slbFeed->setSliderPosition(ui->slbFeed->sliderPosition() + 1);
}

void partMainOverride::onActOverrideFeedMinusTriggered()
{
    ui->slbFeed->setSliderPosition(ui->slbFeed->sliderPosition() - 1);
}

void partMainOverride::onActOverrideRapidPlusTriggered()
{
    ui->slbRapid->setSliderPosition(ui->slbRapid->sliderPosition() + 1);
}

void partMainOverride::onActOverrideRapidMinusTriggered()
{
    ui->slbRapid->setSliderPosition(ui->slbRapid->sliderPosition() - 1);
}

void partMainOverride::onActOverrideSpindlePlusTriggered()
{
    ui->slbSpindle->setSliderPosition(ui->slbSpindle->sliderPosition() + 1);
}

void partMainOverride::onActOverrideSpindleMinusTriggered()
{
    ui->slbSpindle->setSliderPosition(ui->slbSpindle->sliderPosition() - 1);
}

void partMainOverride::onOverrideChanged()
{
    emit overrideChanged();
}
