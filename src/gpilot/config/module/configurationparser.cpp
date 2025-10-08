// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationparser.h"

const QMap<QString,QVariant> DEFAULTS = {
    {"arcApproximationMode", ConfigurationParser::ByAngle},
    {"arcApproximationLength", 0.1},
    {"arcApproximationAngle", 5.0}
};

ConfigurationParser::ConfigurationParser(QObject *parent) : ConfigurationModule(parent, DEFAULTS)
{
}
