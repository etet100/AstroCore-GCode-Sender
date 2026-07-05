// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATUSREPORTPROCESSOR_H
#define STATUSREPORTPROCESSOR_H

#include <QString>
#include <QMap>
#include <optional>
#include "core/machine/machinestatus.h"
#include "core/globals.h"

class StatusReportProcessor
{
    public:
        StatusReportProcessor() = delete;

        static std::optional<MachineStatusReport> parse(const QString &statusLine);

    private:
        static void parseMachineState(const QString &stateStr, MachineStatusReport &status);
        static void parseMachinePosition(const QString &line, MachineStatusReport &status);
        static void parseWorkPosition(const QString &line, MachineStatusReport &status);
        static void parseWorkOffset(const QString &line, MachineStatusReport &status);
        static void parseOverrides(const QString &line, MachineStatusReport &status);
        static void parseFeedSpindleSpeed(const QString &line, MachineStatusReport &status);
        static void parseBuffersStatus(const QString &line, MachineStatusReport &status);
        static void parsePinsState(const QString &line, MachineStatusReport &status);
        static void parseAccessoryState(const QString &line, MachineStatusReport &status);

        static const QMap<QString, MachineState> s_machineStateDictionary;
};

#endif // STATUSREPORTPROCESSOR_H
