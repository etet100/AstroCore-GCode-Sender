// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "partsettingssender.h"
#include "ui_partsettingssender.h"

partSettingsSender::partSettingsSender(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partSettingsSender)
{
    ui->setupUi(this);
}

partSettingsSender::~partSettingsSender()
{
    delete ui;
}

void partSettingsSender::setUsePauseCommands(bool use)
{
    ui->chkUsePauseCommands->setChecked(use);
}

void partSettingsSender::setBeforePauseCommands(const QString &commands)
{
    ui->txtBeforePauseCommands->setPlainText(commands);
}

void partSettingsSender::setAfterPauseCommands(const QString &commands)
{
    ui->txtAfterPauseCommands->setPlainText(commands);
}

void partSettingsSender::setUseStartCommands(bool use)
{
    ui->chkUseStartCommands->setChecked(use);
}

void partSettingsSender::setUseEndCommands(bool use)
{
    ui->chkUseEndCommands->setChecked(use);
}

void partSettingsSender::setStartCommands(const QString &commands)
{
    ui->txtStartCommands->setPlainText(commands);
}

void partSettingsSender::setEndCommands(const QString &commands)
{
    ui->txtEndCommands->setPlainText(commands);
}

void partSettingsSender::setUseToolChangeCommands(bool use)
{
    ui->chkUseToolChangeCommands->setChecked(use);
}

void partSettingsSender::setToolChangeCommands(const QString &commands)
{
    ui->txtToolChangeCommands->setPlainText(commands);
}

void partSettingsSender::setConfirmToolChangeCommandsExecution(bool confirm)
{
    ui->chkConfirmToolChangeCommandsExecution->setChecked(confirm);
}

void partSettingsSender::setPauseOnToolChange(bool pause)
{
    ui->chkPauseOnToolChange->setChecked(pause);
}

void partSettingsSender::setIgnoreResponseErrors(bool ignore)
{
    ui->chkIgnoreResponseErrors->setChecked(ignore);
}

void partSettingsSender::setSetParseStateBeforeSendFromLine(bool set)
{
    ui->chkSetParseStateBeforeSendFromLine->setChecked(set);
}

bool partSettingsSender::usePauseCommands() const
{
    return ui->chkUsePauseCommands->isChecked();
}

QString partSettingsSender::beforePauseCommands() const
{
    return ui->txtBeforePauseCommands->toPlainText();
}

QString partSettingsSender::afterPauseCommands() const
{
    return ui->txtAfterPauseCommands->toPlainText();
}

bool partSettingsSender::useStartCommands() const
{
    return ui->chkUseStartCommands->isChecked();
}

bool partSettingsSender::useEndCommands() const
{
    return ui->chkUseEndCommands->isChecked();
}

QString partSettingsSender::startCommands() const
{
    return ui->txtStartCommands->toPlainText();
}

QString partSettingsSender::endCommands() const
{
    return ui->txtEndCommands->toPlainText();
}

bool partSettingsSender::useToolChangeCommands() const
{
    return ui->chkUseToolChangeCommands->isChecked();
}

QString partSettingsSender::toolChangeCommands() const
{
    return ui->txtToolChangeCommands->toPlainText();
}

bool partSettingsSender::confirmToolChangeCommandsExecution() const
{
    return ui->chkConfirmToolChangeCommandsExecution->isChecked();
}

bool partSettingsSender::pauseOnToolChange() const
{
    return ui->chkPauseOnToolChange->isChecked();
}

bool partSettingsSender::ignoreResponseErrors() const
{
    return ui->chkIgnoreResponseErrors->isChecked();
}

bool partSettingsSender::setParseStateBeforeSendFromLine() const
{
    return ui->chkSetParseStateBeforeSendFromLine->isChecked();
}
