// This file is a part of "G-Pilot" application.
// Copyright 2024 BTS

#include "statecolors.h"

StateColorGroup colorGroupForState(StateBehavior::Type type)
{
    switch (type) {
        case StateBehavior::Type::Disconnection:
            return StateColorGroup::Offline;

        case StateBehavior::Type::Connecting:
        case StateBehavior::Type::Reconnecting:
        case StateBehavior::Type::Handshake:
        case StateBehavior::Type::Initialization:
        case StateBehavior::Type::Reset:
            return StateColorGroup::Connecting;

        case StateBehavior::Type::Idle:
            return StateColorGroup::Ready;

        case StateBehavior::Type::Running:
        case StateBehavior::Type::CheckMode:
        case StateBehavior::Type::Jogging:
        case StateBehavior::Type::GoTo:
            return StateColorGroup::Active;

        case StateBehavior::Type::Homing:
        case StateBehavior::Type::Probing:
        case StateBehavior::Type::ScanTable:
            return StateColorGroup::Special;

        case StateBehavior::Type::Hold:
        case StateBehavior::Type::Pause:
        case StateBehavior::Type::ToolChange:
        case StateBehavior::Type::ExternalProcess:
            return StateColorGroup::Attention;

        case StateBehavior::Type::Alarm:
        case StateBehavior::Type::Error:
            return StateColorGroup::Critical;
    }

    return StateColorGroup::Offline;
}

QColor colorForGroup(StateColorGroup group, bool dark)
{
    // Light theme: saturated Material Design colors, visible on white/gray background.
    // Dark theme: lighter variants, visible on dark background.
    switch (group) {
        case StateColorGroup::Offline:   return dark ? QColor(0xBD, 0xBD, 0xBD) // gray 400
                                                     : QColor(0x75, 0x75, 0x75); // gray 600
        case StateColorGroup::Connecting: return dark ? QColor(0x64, 0xB5, 0xF6) // blue 300
                                                      : QColor(0x19, 0x76, 0xD2); // blue 700
        case StateColorGroup::Ready:     return dark ? QColor(0x81, 0xC7, 0x84) // green 300
                                                     : QColor(0x38, 0x8E, 0x3C); // green 800
        case StateColorGroup::Active:    return dark ? QColor(0x4D, 0xD0, 0xE1) // cyan 300
                                                     : QColor(0x00, 0x83, 0x8F); // cyan 800
        case StateColorGroup::Special:   return dark ? QColor(0xCE, 0x93, 0xD8) // purple 200
                                                     : QColor(0x7B, 0x1F, 0xA2); // purple 800
        case StateColorGroup::Attention: return dark ? QColor(0xFF, 0xB7, 0x4D) // orange 300
                                                     : QColor(0xE6, 0x51, 0x00); // orange 900
        case StateColorGroup::Critical:  return dark ? QColor(0xEF, 0x53, 0x50) // red 400
                                                     : QColor(0xC6, 0x28, 0x28); // red 900
    }

    return QColor();
}
