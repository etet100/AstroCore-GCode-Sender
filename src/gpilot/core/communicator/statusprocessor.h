// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATUSPROCESSOR_H
#define STATUSPROCESSOR_H

#include <QObject>
#include <QString>
#include <QMap>
#include "machinestatus.h"
#include "core/globals.h"

class StatusProcessor : public QObject
{
    Q_OBJECT

public:
    explicit StatusProcessor(QObject *parent = nullptr);

    MachineStatus parse(const QString &statusLine);

private:
    void parseMachineState(const QString &stateStr, MachineStatus &status);
    void parseMachinePosition(const QString &line, MachineStatus &status);
    void parseWorkPosition(const QString &line, MachineStatus &status);
    void parseWorkOffset(const QString &line, MachineStatus &status);
    void parseOverrides(const QString &line, MachineStatus &status);
    void parseFeedSpindleSpeed(const QString &line, MachineStatus &status);
    void parseBuffersStatus(const QString &line, MachineStatus &status);
    void parsePinsState(const QString &line, MachineStatus &status);
    void parseAccessoryState(const QString &line, MachineStatus &status);

    QMap<QString, MachineState> m_machineStateDictionary;
};

#endif // STATUSPROCESSOR_H
