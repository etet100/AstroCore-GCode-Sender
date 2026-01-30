// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef CONVERTER_H
#define CONVERTER_H

#include "core/gcode/gcode.h"

class GcodeParser;

/**
 * Converters modify G-Code line by line. Return true from convertLine()
 * if the line was changed and needs reparsing before next converter.
 */
class Converter
{
    public:
        Converter();
        virtual ~Converter();

        virtual bool convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser) = 0;
        virtual void reset();

        virtual bool needsParser() const { return false; }
        virtual int needsLookahead() const { return 0; }
        virtual bool needsFullGCode() const { return false; }
};

#endif // CONVERTER_H
