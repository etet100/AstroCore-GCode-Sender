// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "partsettingsconsole.h"
#include "ui_partsettingsconsole.h"

PartSettingsConsole::PartSettingsConsole(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partSettingsConsole)
{
    ui->setupUi(this);
}

PartSettingsConsole::~PartSettingsConsole()
{
    delete ui;
}

void PartSettingsConsole::setShowProgramCommands(bool value)
{
    ui->chkConsoleShowProgramCommands->setChecked(value);
}

bool PartSettingsConsole::showProgramCommands() const
{
    return ui->chkConsoleShowProgramCommands->isChecked();
}

void PartSettingsConsole::setShowUICommands(bool value)
{
    ui->chkConsoleShowUICommands->setChecked(value);
}

bool PartSettingsConsole::showUICommands() const
{
    return ui->chkConsoleShowUICommands->isChecked();
}

void PartSettingsConsole::setShowSystemCommands(bool value)
{
    ui->chkConsoleShowSystemCommands->setChecked(value);
}

bool PartSettingsConsole::showSystemCommands() const
{
    return ui->chkConsoleShowSystemCommands->isChecked();
}

void PartSettingsConsole::setCommandAutoCompletion(bool value)
{
    ui->chkConsoleAutocompletion->setChecked(value);
}

bool PartSettingsConsole::commandAutoCompletion() const
{
    return ui->chkConsoleAutocompletion->isChecked();
}

void PartSettingsConsole::setDarkBackgroundMode(bool value)
{
    ui->chkConsoleDarkMode->setChecked(value);
}

bool PartSettingsConsole::darkBackgroundMode() const
{
    return ui->chkConsoleDarkMode->isChecked();
}
