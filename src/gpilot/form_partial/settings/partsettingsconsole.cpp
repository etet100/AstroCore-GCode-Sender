// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "partsettingsconsole.h"
#include "ui_partsettingsconsole.h"

partSettingsConsole::partSettingsConsole(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partSettingsConsole)
{
    ui->setupUi(this);
}

partSettingsConsole::~partSettingsConsole()
{
    delete ui;
}

void partSettingsConsole::setShowProgramCommands(bool value)
{
    ui->chkConsoleShowProgramCommands->setChecked(value);
}

bool partSettingsConsole::showProgramCommands() const
{
    return ui->chkConsoleShowProgramCommands->isChecked();
}

void partSettingsConsole::setShowUICommands(bool value)
{
    ui->chkConsoleShowUICommands->setChecked(value);
}

bool partSettingsConsole::showUICommands() const
{
    return ui->chkConsoleShowUICommands->isChecked();
}

void partSettingsConsole::setShowSystemCommands(bool value)
{
    ui->chkConsoleShowSystemCommands->setChecked(value);
}

bool partSettingsConsole::showSystemCommands() const
{
    return ui->chkConsoleShowSystemCommands->isChecked();
}

void partSettingsConsole::setCommandAutoCompletion(bool value)
{
    ui->chkConsoleAutocompletion->setChecked(value);
}

bool partSettingsConsole::commandAutoCompletion() const
{
    return ui->chkConsoleAutocompletion->isChecked();
}

void partSettingsConsole::setDarkBackgroundMode(bool value)
{
    ui->chkConsoleDarkMode->setChecked(value);
}

bool partSettingsConsole::darkBackgroundMode() const
{
    return ui->chkConsoleDarkMode->isChecked();
}
