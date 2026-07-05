// This file is a part of "G-Pilot" application.
// Copyright 2024 BTS

#include "statecolors.h"

StateColorGroup colorGroupForState(AbstractStateBehavior::Type type)
{
    switch (type) {
        case AbstractStateBehavior::Type::Disconnecting:
            return StateColorGroup::Offline;

        case AbstractStateBehavior::Type::Connecting:
        case AbstractStateBehavior::Type::Reconnecting:
        case AbstractStateBehavior::Type::Handshake:
        case AbstractStateBehavior::Type::Initialization:
        case AbstractStateBehavior::Type::Reset:
            return StateColorGroup::Connecting;

        case AbstractStateBehavior::Type::Idle:
            return StateColorGroup::Ready;

        case AbstractStateBehavior::Type::Running:
        case AbstractStateBehavior::Type::CheckMode:
        case AbstractStateBehavior::Type::Jogging:
        case AbstractStateBehavior::Type::GoTo:
            return StateColorGroup::Active;

        case AbstractStateBehavior::Type::Homing:
        case AbstractStateBehavior::Type::Probing:
        case AbstractStateBehavior::Type::ScanTable:
            return StateColorGroup::Special;

        case AbstractStateBehavior::Type::Hold:
        case AbstractStateBehavior::Type::Pause:
        case AbstractStateBehavior::Type::ToolChange:
        case AbstractStateBehavior::Type::ExternalProcess:
        case AbstractStateBehavior::Type::UserPrompt:
            return StateColorGroup::Attention;

        case AbstractStateBehavior::Type::Alarm:
        case AbstractStateBehavior::Type::Error:
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
