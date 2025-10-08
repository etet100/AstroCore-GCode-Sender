// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationconnection.h"

const QMap<QString,QVariant> DEFAULTS = {
    {"queryStateInterval", 100},
    {"connectionMode", ConfigurationConnection::ConnectionMode::VIRTUAL_UCNC},
    {"serialPort", ""},
    {"serialBaud", 115200},
    {"rawTcpHost", "localhost"},
    {"rawTcpPort", 8080},
};

ConfigurationConnection::ConfigurationConnection(QObject *parent) : ConfigurationModule(parent, DEFAULTS)
{
}
