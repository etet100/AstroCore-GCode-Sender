// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "partsettingssender.h"
#include "ui_partsettingssender.h"

PartSettingsSender::PartSettingsSender(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partSettingsSender)
{
    ui->setupUi(this);
}

PartSettingsSender::~PartSettingsSender()
{
    delete ui;
}

void PartSettingsSender::setUsePauseCommands(bool use)
{
    ui->chkUsePauseCommands->setChecked(use);
}

void PartSettingsSender::setBeforePauseCommands(const QString &commands)
{
    ui->txtBeforePauseCommands->setPlainText(commands);
}

void PartSettingsSender::setAfterPauseCommands(const QString &commands)
{
    ui->txtAfterPauseCommands->setPlainText(commands);
}

void PartSettingsSender::setUseStartCommands(bool use)
{
    ui->chkUseStartCommands->setChecked(use);
}

void PartSettingsSender::setUseEndCommands(bool use)
{
    ui->chkUseEndCommands->setChecked(use);
}

void PartSettingsSender::setStartCommands(const QString &commands)
{
    ui->txtStartCommands->setPlainText(commands);
}

void PartSettingsSender::setEndCommands(const QString &commands)
{
    ui->txtEndCommands->setPlainText(commands);
}

void PartSettingsSender::setUseToolChangeCommands(bool use)
{
    ui->chkUseToolChangeCommands->setChecked(use);
}

void PartSettingsSender::setToolChangeCommands(const QString &commands)
{
    ui->txtToolChangeCommands->setPlainText(commands);
}

void PartSettingsSender::setConfirmToolChangeCommandsExecution(bool confirm)
{
    ui->chkConfirmToolChangeCommandsExecution->setChecked(confirm);
}

void PartSettingsSender::setPauseOnToolChange(bool pause)
{
    ui->chkPauseOnToolChange->setChecked(pause);
}

void PartSettingsSender::setIgnoreResponseErrors(bool ignore)
{
    ui->chkIgnoreResponseErrors->setChecked(ignore);
}

void PartSettingsSender::setSetParseStateBeforeSendFromLine(bool set)
{
    ui->chkSetParseStateBeforeSendFromLine->setChecked(set);
}

bool PartSettingsSender::usePauseCommands() const
{
    return ui->chkUsePauseCommands->isChecked();
}

QString PartSettingsSender::beforePauseCommands() const
{
    return ui->txtBeforePauseCommands->toPlainText();
}

QString PartSettingsSender::afterPauseCommands() const
{
    return ui->txtAfterPauseCommands->toPlainText();
}

bool PartSettingsSender::useStartCommands() const
{
    return ui->chkUseStartCommands->isChecked();
}

bool PartSettingsSender::useEndCommands() const
{
    return ui->chkUseEndCommands->isChecked();
}

QString PartSettingsSender::startCommands() const
{
    return ui->txtStartCommands->toPlainText();
}

QString PartSettingsSender::endCommands() const
{
    return ui->txtEndCommands->toPlainText();
}

bool PartSettingsSender::useToolChangeCommands() const
{
    return ui->chkUseToolChangeCommands->isChecked();
}

QString PartSettingsSender::toolChangeCommands() const
{
    return ui->txtToolChangeCommands->toPlainText();
}

bool PartSettingsSender::confirmToolChangeCommandsExecution() const
{
    return ui->chkConfirmToolChangeCommandsExecution->isChecked();
}

bool PartSettingsSender::pauseOnToolChange() const
{
    return ui->chkPauseOnToolChange->isChecked();
}

bool PartSettingsSender::ignoreResponseErrors() const
{
    return ui->chkIgnoreResponseErrors->isChecked();
}

bool PartSettingsSender::setParseStateBeforeSendFromLine() const
{
    return ui->chkSetParseStateBeforeSendFromLine->isChecked();
}
