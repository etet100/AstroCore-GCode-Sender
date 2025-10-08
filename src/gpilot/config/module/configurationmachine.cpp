// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationmachine.h"

const QMap<QString,QVariant> DEFAULTS = {
    {"spindleSpeedRange", QVariantMap({{"min", 0}, {"max", 100}})},
    {"spindleSpeed", 100},
    {"laserPowerRange", QVariantMap({{"min", 0}, {"max", 100}})},
    {"referencePositionDirX", ConfigurationMachine::ReferencePositionDir::Positive},
    {"referencePositionDirY", ConfigurationMachine::ReferencePositionDir::Positive},
    {"referencePositionDirZ", ConfigurationMachine::ReferencePositionDir::Positive},
    {"overrideMaxTravel", false},
    {"maxTravel", QVariantMap({{"x", 100.0f}, {"y", 100.0f}, {"z", 100.0f}})},
    {"overrideFeed", false},
    {"overrideFeedValue", 100},
    {"overrideRapid", false},
    {"overrideRapidValue", 100},
    {"overrideSpindleSpeed", false},
    {"overrideSpindleSpeedValue", 100},
};

ConfigurationMachine::ConfigurationMachine(QObject *parent) : ConfigurationModule(parent, DEFAULTS)
{
}
