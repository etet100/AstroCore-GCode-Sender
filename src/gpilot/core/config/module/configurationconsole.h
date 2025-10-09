// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATION_CONSOLE_H
#define CONFIGURATION_CONSOLE_H

#include <QObject>
#include "configurationmodule.h"

class ConfigurationConsole : public ConfigurationModule
{
    friend class frmSettings;

    Q_OBJECT
    Q_PROPERTY(bool showProgramCommands MEMBER m_showProgramCommands NOTIFY changed)
    Q_PROPERTY(bool showUiCommands MEMBER m_showUiCommands NOTIFY changed)
    Q_PROPERTY(bool showSystemCommands MEMBER m_showSystemCommands NOTIFY changed)
    Q_PROPERTY(bool commandAutoCompletion MEMBER m_commandAutoCompletion NOTIFY changed)
    Q_PROPERTY(bool darkBackgroundMode MEMBER m_darkBackgroundMode NOTIFY changed)
    Q_PROPERTY(QStringList commandHistory MEMBER m_commandHistory NOTIFY changed)

    public:
        explicit ConfigurationConsole(QObject *parent = nullptr);
        ConfigurationConsole& operator=(const ConfigurationConsole&) { return *this; }
        QString getSectionName() override { return "console"; }

        bool showProgramCommands() const { return m_showProgramCommands; }
        bool showUiCommands() const { return m_showUiCommands; }
        bool showSystemCommands() const { return m_showSystemCommands; }
        bool commandAutoCompletion() const { return m_commandAutoCompletion; }
        bool darkBackgroundMode() const { return m_darkBackgroundMode; }
        QStringList commandHistory() const { return m_commandHistory; }
        void addCommandToHistory(QString);
        void setCommandHistory(QStringList);
    private:
        bool m_showProgramCommands;
        bool m_showUiCommands;
        bool m_showSystemCommands;
        bool m_commandAutoCompletion;
        bool m_darkBackgroundMode;
        QStringList m_commandHistory;
};

#endif // CONFIGURATION_CONSOLE_H
