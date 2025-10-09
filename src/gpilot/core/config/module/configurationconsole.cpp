// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationconsole.h"

const QMap<QString, QVariant> DEFAULTS = {
    {"showProgramCommands", false},
    {"showUiCommands", true},
    {"showSystemCommands", false},
    {"commandAutoCompletion", true},
    {"darkBackgroundMode", false},
};

ConfigurationConsole::ConfigurationConsole(QObject *parent) : ConfigurationModule(parent, DEFAULTS)
{
}

void ConfigurationConsole::addCommandToHistory(QString command)
{
    m_commandHistory << command;
    if (m_commandHistory.size() > 25) {
        m_commandHistory.removeFirst();
    }
}

void ConfigurationConsole::setCommandHistory(QStringList commandHistory)
{
    m_commandHistory = commandHistory;
}
