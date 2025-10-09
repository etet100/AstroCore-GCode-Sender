// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATION_SENDER_H
#define CONFIGURATION_SENDER_H

#include <QObject>
#include "configurationmodule.h"

class ConfigurationSender : public ConfigurationModule
{
    friend class frmSettings;

    Q_OBJECT
    Q_PROPERTY(bool useProgramStartCommands MEMBER m_useProgramStartCommands NOTIFY changed)
    Q_PROPERTY(QString programStartCommands MEMBER m_programStartCommands NOTIFY changed)
    Q_PROPERTY(bool useProgramEndCommands MEMBER m_useProgramEndommands NOTIFY changed)
    Q_PROPERTY(QString programEndCommands MEMBER m_programEndCommands NOTIFY changed)
    Q_PROPERTY(bool usePauseCommands MEMBER m_usePauseCommands NOTIFY changed)
    Q_PROPERTY(QString beforePauseCommands MEMBER m_beforePauseCommands NOTIFY changed)
    Q_PROPERTY(QString afterPauseCommands MEMBER m_afterPauseCommands NOTIFY changed)
    Q_PROPERTY(bool useToolChangeCommands MEMBER m_useToolChangeCommands NOTIFY changed)
    Q_PROPERTY(QString toolChangeCommands MEMBER m_toolChangeCommands NOTIFY changed)
    Q_PROPERTY(bool confirmToolChangeCommandsExecution MEMBER m_confirmToolChangeCommandsExecution NOTIFY changed)
    Q_PROPERTY(bool toolChangePause MEMBER m_toolChangePause NOTIFY changed)
    Q_PROPERTY(bool ignoreErrorResponses MEMBER m_ignoreErrorResponses NOTIFY changed)
    Q_PROPERTY(bool setParserStateBeforeSendingFromSelectedLine MEMBER m_setParserStateBeforeSendingFromSelectedLine NOTIFY changed)

    public:
        explicit ConfigurationSender(QObject *parent = nullptr);
        ConfigurationSender& operator=(const ConfigurationSender&) { return *this; }
        QString getSectionName() override { return "sender"; }

        bool useProgramStartCommands() const { return m_useProgramStartCommands; }
        QString programStartCommands() const { return m_programStartCommands; }
        bool useProgramEndCommands() const { return m_useProgramEndommands; }
        QString programEndCommands() const { return m_programEndCommands; }
        bool usePauseCommands() const { return m_usePauseCommands; }
        QString beforePauseCommands() const { return m_beforePauseCommands; }
        QString afterPauseCommands() const { return m_afterPauseCommands; }
        bool useToolChangeCommands() const { return m_useToolChangeCommands; }
        QString toolChangeCommands() const { return m_toolChangeCommands; }
        bool confirmToolChangeCommandsExecution() const { return m_confirmToolChangeCommandsExecution; }
        bool pauseSenderOnToolChange() const { return m_toolChangePause; }
        bool ignoreErrorResponses() const { return m_ignoreErrorResponses; }
        bool setParserStateBeforeSendingFromSelectedLine() const { return m_setParserStateBeforeSendingFromSelectedLine; }

    private:
        bool m_useProgramStartCommands;
        QString m_programStartCommands;
        bool m_useProgramEndommands;
        QString m_programEndCommands;
        bool m_usePauseCommands;
        QString m_beforePauseCommands;
        QString m_afterPauseCommands;
        bool m_useToolChangeCommands;
        QString m_toolChangeCommands;
        bool m_confirmToolChangeCommandsExecution;
        bool m_toolChangePause;
        bool m_ignoreErrorResponses;
        bool m_setParserStateBeforeSendingFromSelectedLine;
};

#endif // CONFIGURATION_SENDER_H
