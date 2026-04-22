// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef ABSTRACTCONVERTER_H
#define ABSTRACTCONVERTER_H

#include "core/gcode/gcode.h"

class GcodeParser;

/**
 * Converters modify G-Code line by line. Return true from convertLine()
 * if the line was changed and needs reparsing before next converter.
 *
 * Every converter is expected to provide a static method:
 *     static QString parameterSchema();
 * returning a JSON string that describes the converter for the UI.
 *
 * Schema format:
 *   {
 *     "title":       "<human-readable converter name>",
 *     "description": "<short explanation of what the converter does>",
 *     "image":       "<Qt resource path to an illustration, e.g. :/images/...>",
 *     "fields":      [ { "name": "...", "type": "bool|int|float|choice", ... } ]
 *   }
 *
 * Converters with no configurable parameters still provide title /
 * description / image; "fields" is an empty array. FrmConverterSettings
 * then shows a 'No configurable parameters' placeholder.
 *
 * The values returned by the dialog are passed to the concrete
 * converter's constructor.
 */
class AbstractConverter
{
    public:
        AbstractConverter();
        virtual ~AbstractConverter();

        virtual bool convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser) = 0;
        virtual void reset();

        virtual bool needsParser() const { return false; }
        virtual int needsLookahead() const { return 0; }
        virtual bool needsFullGCode() const { return false; }
};

#endif // ABSTRACTCONVERTER_H
