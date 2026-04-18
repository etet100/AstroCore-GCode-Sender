// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationjogging.h"

const QVariantMap DEFAULTS = {
    {"step", 0.1},
    {"continuous", false},
    {"stepChoices", QStringList{"0.01", "0.1", "1.0", "10.0", "50.0", "100.0"}},
    {"feed", 100},
    {"feedz", 100},
    {"feedChoices", QStringList{"10", "50", "100", "500", "1000", "2000"}},
    {"keyboardControl", false},
    {"sepFeedZ", false}
};

ConfigurationJogging::ConfigurationJogging(QObject *parent)
    : AbstractConfigurationModule{parent, DEFAULTS}
{
}
