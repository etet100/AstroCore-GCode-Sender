// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#include "uiconfigs.h"
#include "core/config/configuration.h"

UiConfigs& UiConfigs::instance()
{
    static UiConfigs instance;
    return instance;
}

void UiConfigs::registerAll(Configuration& cfg)
{
    cfg.registerModule(&m_ui);
    cfg.registerModule(&m_console);
    cfg.registerModule(&m_visualizer);
    cfg.registerModule(&m_macros);
}
