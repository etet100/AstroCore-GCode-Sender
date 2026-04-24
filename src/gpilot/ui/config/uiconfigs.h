// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#ifndef UICONFIGS_H
#define UICONFIGS_H

#include "configurationui.h"
#include "configurationconsole.h"
#include "configurationvisualizer.h"
#include "core/config/module/configurationmacros.h"

class Configuration;

// Owner of UI-only configuration modules. Meyers singleton so UI code can
// access each module from anywhere without pulling in Core. Call
// registerAll(configuration) once during startup, before Configuration::init()
// for eager loading (or after init() for lazy).
class UiConfigs
{
    public:
        static UiConfigs& instance();

        void registerAll(Configuration& cfg);

        ConfigurationUI& ui() { return m_ui; }
        ConfigurationConsole& console() { return m_console; }
        ConfigurationVisualizer& visualizer() { return m_visualizer; }
        ConfigurationMacros& macros() { return m_macros; }

    private:
        UiConfigs() = default;
        UiConfigs(const UiConfigs&) = delete;
        UiConfigs& operator=(const UiConfigs&) = delete;

        ConfigurationUI m_ui;
        ConfigurationConsole m_console;
        ConfigurationVisualizer m_visualizer;
        ConfigurationMacros m_macros;
};

#endif // UICONFIGS_H
