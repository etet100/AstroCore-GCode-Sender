// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "action.h"

Action::Action(Type type) : m_type(type)
{
}

const QMap<int, QString> Action::NAMES = {
    {None, "None"},
    {Reset, "Reset"},
    {Run, "Start"},
    {Stop, "Stop"},
    {Pause, "Pause"},
    {Resume, "Resume"},
    {FeedHold, "Feed Hold"},
    {CycleStart, "Cycle Start"},
    {Jog, "Jog"},
    {Home, "Home"},
    {Probe, "Probe"},
    {Unlock, "Unlock"},
    {QueryMachineConfiguration, "QueryMachineConfiguration"},
    {SaveMachineConfigurationParam, "SaveMachineConfigurationParam"},
    {ToolChange, "Tool Change"},
};
