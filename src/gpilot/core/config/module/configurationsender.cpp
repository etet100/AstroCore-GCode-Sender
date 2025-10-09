// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationsender.h"

const QMap<QString, QVariant> DEFAULTS = {
    {"useProgramStartCommands", false},
    {"programStartCommands", ""},
    {"useProgramEndCommands", false},
    {"programEndCommands", ""},
    {"usePauseCommands", false},
    {"beforePauseCommands", ""},
    {"afterPauseCommands", ""},
    {"useToolChangeCommands", false},
    {"toolChangeCommands", ""},
    {"confirmToolChangeCommandsExecution", false},
    {"toolChangePause", false},
    {"ignoreErrorResponses", false},
    {"setParserStateBeforeSendingFromSelectedLine", false},
};

ConfigurationSender::ConfigurationSender(QObject *parent) : ConfigurationModule(parent, DEFAULTS)
{
}
