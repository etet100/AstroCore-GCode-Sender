// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "abstractconverter.h"

AbstractConverter::AbstractConverter()
{
}

AbstractConverter::~AbstractConverter()
{
}

void AbstractConverter::reset()
{
    // Default implementation does nothing
    // Override in derived classes if state needs to be reset
}
