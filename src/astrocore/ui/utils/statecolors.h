// This file is a part of "G-Pilot" application.
// Copyright 2024 BTS

#ifndef STATECOLORS_H
#define STATECOLORS_H

#include <QColor>
#include "core/state_behavior/abstractstatebehavior.h"

// Maps AbstractStateBehavior::Type to a color group and then to a QColor.
// Keeps all visual state decisions in one place, away from core logic.

enum class StateColorGroup {
    Offline,    // gray   — disconnected
    Connecting, // blue   — establishing connection
    Ready,      // green  — idle, waiting for commands
    Active,     // teal   — normal operation in progress
    Special,    // purple — calibration / measurement
    Attention,  // orange — waiting for user or paused
    Critical,   // red    — alarm or error
};

StateColorGroup colorGroupForState(AbstractStateBehavior::Type type);
QColor colorForGroup(StateColorGroup group, bool dark = false);

inline QColor colorForState(AbstractStateBehavior::Type type, bool dark = false) {
    return colorForGroup(colorGroupForState(type), dark);
}

#endif // STATECOLORS_H
