// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationpendant.h"
#include "core/config/configuration.h"

const QMap<QString,QVariant> DEFAULTS = {
    {"wifiSsid", "wifi"},
    {"wifiPassword", "password"},
    {"hostIp", "192.168.1.100"},
    {"port", 8000},
    {"enabled", false}
};

ConfigurationPendant::ConfigurationPendant(QObject *parent) : AbstractConfigurationModule(parent, DEFAULTS)
{
}

ConfigurationPendant& ConfigurationPendant::instance()
{
    static ConfigurationPendant inst;
    return inst;
}

void ConfigurationPendant::registerWith(Configuration& cfg)
{
    cfg.registerModule(&instance());
}
