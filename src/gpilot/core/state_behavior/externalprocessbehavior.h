// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef EXTERNALPROCESSBEHAVIOR_H
#define EXTERNALPROCESSBEHAVIOR_H

#include "statebehavior.h"

// Represents a state where the CNC machine is executing a process
// that was not started by GPilot (e.g., connected while machine was already running).
// Monitors state changes and transitions to the appropriate behavior
// once the external process finishes.
class ExternalProcessBehavior : public StateBehavior
{
    Q_OBJECT

public:
    explicit ExternalProcessBehavior(QObject *parent = nullptr);
    QString description() override;

    Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
    void onMachineStateChanged(MachineState state) override;
    void onAlarm(int code) override;

protected:
    QString name() const override { return "ExternalProcess"; }
};

#endif // EXTERNALPROCESSBEHAVIOR_H
