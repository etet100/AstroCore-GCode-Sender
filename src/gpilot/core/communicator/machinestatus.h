// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef MACHINESTATUS_H
#define MACHINESTATUS_H

#include <QVector3D>
#include <QString>
#include <QDebug>
#include "core/globals.h"

struct PinState
{
    bool limitX     = false;
    bool limitY     = false;
    bool limitZ     = false;
    bool limitA     = false;
    bool limitB     = false;
    bool probe      = false;
    bool door       = false;
    bool feedHold   = false;
    bool reset      = false;
    bool cycleStart = false;

    QString raw;

    static PinState parse(const QString& s)
    {
        PinState p;
        p.raw = s;
        for (QChar c : s) {
            switch (c.toLatin1()) {
                case 'X': p.limitX     = true; break;
                case 'Y': p.limitY     = true; break;
                case 'Z': p.limitZ     = true; break;
                case 'A': p.limitA     = true; break;
                case 'B': p.limitB     = true; break;
                case 'P': p.probe      = true; break;
                case 'D': p.door       = true; break;
                case 'H': p.feedHold   = true; break;
                case 'R': p.reset      = true; break;
                case 'S': p.cycleStart = true; break;
            }
        }

        return p;
    }

    const QString& toString() const
    {
        return raw;
    }
};

struct MachineStatusReport
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
    PinState pinStates;
    bool hasPinStates = false;

    // Accessory states (spindle, flood, mist)
    bool spindleEnabled = false;
    bool spindleCW = false;
    bool floodEnabled = false;
    bool mistEnabled = false;
    bool hasAccessoryState = false;

    // Raw line for debugging
    QString rawLine;

    QString toMarkdown() const
    {
        static const QMap<MachineState, QString> stateNames = {
            { MachineState::Unknown, "Unknown" },
            { MachineState::Idle,    "Idle"    },
            { MachineState::Alarm,   "Alarm"   },
            { MachineState::Run,     "Run"     },
            { MachineState::Home,    "Home"    },
            { MachineState::Hold0,   "Hold"    },
            { MachineState::Hold1,   "Hold (door)"  },
            { MachineState::Queue,   "Queue"   },
            { MachineState::Check,   "Check"   },
            { MachineState::Door0,   "Door"    },
            { MachineState::Door1,   "Door (open)"  },
            { MachineState::Door2,   "Door (hold)"  },
            { MachineState::Door3,   "Door (resume)" },
            { MachineState::Jog,     "Jog"     },
            { MachineState::Sleep,   "Sleep"   },
        };

        QStringList lines;

        // Line 1: state
        lines << QString("**%1**").arg(stateNames.value(state, "Unknown"));

        // Line 2: positions
        QStringList pos;
        if (hasWorkPos) {
            pos << QString("*WPos:* %1, %2, %3")
                .arg(workPos.x(), 0, 'f', 3)
                .arg(workPos.y(), 0, 'f', 3)
                .arg(workPos.z(), 0, 'f', 3);
        }
        if (hasMachinePos) {
            pos << QString("*MPos:* %1, %2, %3")
                .arg(machinePos.x(), 0, 'f', 3)
                .arg(machinePos.y(), 0, 'f', 3)
                .arg(machinePos.z(), 0, 'f', 3);
        }
        if (!pos.isEmpty()) { lines << pos.join("  "); }
            else { lines << "*No position data*"; }

        // Line 3: feed/spindle + accessories
        QStringList motion;
        if (hasFeedSpindleSpeed) {
            motion << QString("*F:* %1  *S:* %2").arg(feedRate).arg(spindleSpeed);
        }
        if (hasAccessoryState) {
            if (spindleEnabled) { motion << (spindleCW ? "Spindle CW" : "Spindle CCW"); }
            if (floodEnabled)   { motion << "Flood"; }
            if (mistEnabled)    { motion << "Mist"; }
        }
        if (hasBufferStatus) {
            motion << QString("*Bf:* %1/%2").arg(bufferAvailable).arg(bufferSize);
        }
        if (!motion.isEmpty()) { lines << motion.join("  "); }
            else { lines << "*No feed/spindle data*"; }

        // Line 4: overrides + pins
        QStringList extras;
        if (hasPinStates && !pinStates.raw.isEmpty()) {
            extras << QString("*Pins:* `%1`").arg(pinStates.toString());
        } else {
            extras << QString("*Pins:* none");
        }
        if (hasOverrides) {
            extras << QString("*Ov:* %1% / %2% / %3%").arg(feedOverride).arg(rapidOverride).arg(spindleOverride);
        }
        if (!extras.isEmpty()) { lines << extras.join("  "); }
                else { lines << "*No overrides/pin data*"; }

        return lines.join("<br/>");
    }
};

inline QDebug operator<<(QDebug debug, const MachineStatusReport &status)
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
        debug << ", Pn=" << status.pinStates.toString();
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
