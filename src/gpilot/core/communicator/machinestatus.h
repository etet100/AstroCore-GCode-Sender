// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef MACHINESTATUS_H
#define MACHINESTATUS_H

#include <QVector3D>
#include <QString>
#include <QDebug>
#include "core/globals.h"

struct MachineStatus
{
    // Main state
    MachineState state = MachineState::Unknown;

    // Position data
    QVector3D machinePos;
    QVector3D workPos;
    QVector3D workOffset;
    bool hasMachinePos = false;
    bool hasWorkPos = false;
    bool hasWorkOffset = false;

    // Feed and spindle
    int feedRate = 0;
    int spindleSpeed = 0;
    bool hasFeedSpindleSpeed = false;

    // Overrides
    int feedOverride = 100;
    int rapidOverride = 100;
    int spindleOverride = 100;
    bool hasOverrides = false;

    // Buffer status
    int bufferAvailable = 0;
    int bufferSize = 0;
    bool hasBufferStatus = false;

    // Pin states
    QString pinStates;
    bool hasPinStates = false;

    // Accessory states (spindle, flood, mist)
    bool spindleEnabled = false;
    bool spindleCW = false;
    bool floodEnabled = false;
    bool mistEnabled = false;
    bool hasAccessoryState = false;

    // Raw line for debugging
    QString rawLine;
};

inline QDebug operator<<(QDebug debug, const MachineStatus &status)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "MachineStatus(";
    debug << "state=" << static_cast<int>(status.state);

    if (status.hasMachinePos) {
        debug << ", MPos=" << status.machinePos;
    }
    if (status.hasWorkPos) {
        debug << ", WPos=" << status.workPos;
    }
    if (status.hasWorkOffset) {
        debug << ", WCO=" << status.workOffset;
    }
    if (status.hasFeedSpindleSpeed) {
        debug << ", F=" << status.feedRate << ", S=" << status.spindleSpeed;
    }
    if (status.hasOverrides) {
        debug << ", Ov=" << status.feedOverride << "/" << status.rapidOverride << "/" << status.spindleOverride;
    }
    if (status.hasBufferStatus) {
        debug << ", Bf=" << status.bufferAvailable << "/" << status.bufferSize;
    }
    if (status.hasPinStates) {
        debug << ", Pn=" << status.pinStates;
    }
    if (status.hasAccessoryState) {
        debug << ", A:";
        if (status.spindleEnabled) debug << (status.spindleCW ? "S" : "C");
        if (status.floodEnabled) debug << "F";
        if (status.mistEnabled) debug << "M";
    }

    debug << ")";

    return debug;
}

#endif // MACHINESTATUS_H
