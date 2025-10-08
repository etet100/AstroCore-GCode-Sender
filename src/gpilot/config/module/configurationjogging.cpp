// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationjogging.h"

const QVariantMap DEFAULTS = {
    {"step", 0.1},
    {"stepChoices", QStringList{"0.01", "0.1", "1.0", "10.0", "50.0", "100.0"}},
    {"feed", 100},
    {"feedChoices", QStringList{"10", "50", "100", "500", "1000", "2000"}},
};

ConfigurationJogging::ConfigurationJogging(QObject *parent)
    : ConfigurationModule{parent, DEFAULTS}
{
}
