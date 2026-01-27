// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationpendant.h"

const QMap<QString,QVariant> DEFAULTS = {
    {"wifiSsid", "wifi"},
    {"wifiPassword", "password"},
    {"hostIp", "192.168.1.100"}
};

ConfigurationPendant::ConfigurationPendant(QObject *parent) : ConfigurationModule(parent, DEFAULTS)
{
}
