// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "partmainvirtualsettings.h"
#include "ui_partmainvirtualsettings.h"

PartMainVirtualSettings::PartMainVirtualSettings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PartMainVirtualSettings)
{
    ui->setupUi(this);
}

PartMainVirtualSettings::~PartMainVirtualSettings()
{
    delete ui;
}

void PartMainVirtualSettings::deviceConfigurationReceived(PhysicalMachineConfiguration &machineConfiguration)
{
    m_homingDirs = machineConfiguration.homingDirs();

    const QChar homingDirString[2] = {'-', '+'};

    ui->labelX->setText(QString("X: (%1)").arg(homingDirString[(int) m_homingDirs.x()]));
    ui->labelY->setText(QString("Y: (%1)").arg(homingDirString[(int) m_homingDirs.y()]));
    ui->labelZ->setText(QString("Z: (%1)").arg(homingDirString[(int) m_homingDirs.z()]));

    setEnabled(true);
}

float PartMainVirtualSettings::calcFinalAxisPos(HomingDir homingDir, float currentPos, int val)
{
    float finalPos = currentPos + (homingDir == HomingDir::Negative ? -val : val);

    return finalPos;
}

void PartMainVirtualSettings::updateDevice()
{
    QString updateCommand = QString("@@@,%s,%s,%s,%s,%s")
        .arg(ui->editX->text())
        .arg(ui->editY->text())
        .arg(ui->editZ->text())
        .arg(ui->editProbe->text())
        .arg(ui->btnEStop->isChecked() ? "1" : "0");
}

void PartMainVirtualSettings::set(int val)
{
    float currentPos = 0.1;

    ui->editX->setText(QString("%1").arg(calcFinalAxisPos(m_homingDirs.x(), currentPos, val)));
    ui->editY->setText(QString("%1").arg(calcFinalAxisPos(m_homingDirs.y(), currentPos, val)));
    ui->editZ->setText(QString("%1").arg(calcFinalAxisPos(m_homingDirs.z(), currentPos, val)));

    updateDevice();
}

void PartMainVirtualSettings::onSetClicked1()
{
    set(10);
}

void PartMainVirtualSettings::onSetClicked2()
{
    set(50);
}

void PartMainVirtualSettings::onSetClicked3()
{
    set(100);
}

void PartMainVirtualSettings::onEditingFinished()
{
    updateDevice();
}

void PartMainVirtualSettings::onEStopToggled(bool value)
{
    updateDevice();
}
