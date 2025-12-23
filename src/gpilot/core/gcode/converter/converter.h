// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef CONVERTER_H
#define CONVERTER_H

#include "core/gcode/gcode.h"

class Converter
{
    public:
        Converter(GCode &data);
        virtual GCode &convert() = 0;

    protected:
        GCode &m_data;
};

#endif // CONVERTER_H
