// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "streamconverter.h"

GCode* StreamConverter::convertAll(GCode &source)
{
    reset();

    GCode *result = new GCode();
    result->reserve(source.count());

    for (auto &item : source) {
        for (const auto &out : push(item)) {
            *result << out;
        }
    }
    for (const auto &out : flush()) {
        *result << out;
    }

    return result;
}
