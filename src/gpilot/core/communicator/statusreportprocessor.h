// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATUSREPORTPROCESSOR_H
#define STATUSREPORTPROCESSOR_H

#include <QObject>
#include <QString>
#include <QMap>
#include "machinestatus.h"
#include "core/globals.h"

class StatusReportProcessor : public QObject
{
    Q_OBJECT

public:
    explicit StatusReportProcessor(QObject *parent = nullptr);

    MachineStatusReport parse(const QString &statusLine);

private:
    void parseMachineState(const QString &stateStr, MachineStatusReport &status);
    void parseMachinePosition(const QString &line, MachineStatusReport &status);
    void parseWorkPosition(const QString &line, MachineStatusReport &status);
    void parseWorkOffset(const QString &line, MachineStatusReport &status);
    void parseOverrides(const QString &line, MachineStatusReport &status);
    void parseFeedSpindleSpeed(const QString &line, MachineStatusReport &status);
    void parseBuffersStatus(const QString &line, MachineStatusReport &status);
    void parsePinsState(const QString &line, MachineStatusReport &status);
    void parseAccessoryState(const QString &line, MachineStatusReport &status);

    QMap<QString, MachineState> m_machineStateDictionary;
};

#endif // STATUSREPORTPROCESSOR_H
