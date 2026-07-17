#include "partsettingsupdate.h"
#include "ui_partsettingsupdate.h"

PartSettingsUpdate::PartSettingsUpdate(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::partSettingsUpdate)
{
    ui->setupUi(this);

    connect(ui->chkCheckForUpdates, &QCheckBox::toggled, ui->spnIntervalDays, &QWidget::setEnabled);
}

PartSettingsUpdate::~PartSettingsUpdate()
{
    delete ui;
}

void PartSettingsUpdate::setCheckForUpdates(bool value)
{
    ui->chkCheckForUpdates->setChecked(value);
    ui->spnIntervalDays->setEnabled(value);
}

bool PartSettingsUpdate::checkForUpdates() const
{
    return ui->chkCheckForUpdates->isChecked();
}

void PartSettingsUpdate::setCheckIntervalDays(int value)
{
    ui->spnIntervalDays->setValue(value);
}

int PartSettingsUpdate::checkIntervalDays() const
{
    return ui->spnIntervalDays->value();
}
