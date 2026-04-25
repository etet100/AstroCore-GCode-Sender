// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef STREAMCONVERTER_H
#define STREAMCONVERTER_H

#include "core/gcode/gcode.h"
#include <QList>
#include <QString>

/**
 * Stream-mode G-Code converter.
 *
 * A converter is a stream operator: each input line yields zero, one or many
 * output lines. Lookahead (when a converter needs to peek at upcoming inputs)
 * is realised by buffering inputs internally — never by reading from the
 * source program, which the converter never sees as a whole.
 *
 * Lifecycle:
 *   reset()            clear internal state, ready for a new stream
 *   push(in)*          feed N input lines, collecting their outputs
 *   flush()            signal end of input; converter emits any deferred items
 *
 * Cardinality contract:
 *   push(in) returns 0..N items; flush() returns 0..N items.
 *   After flush() it is invalid to call push() until reset().
 *
 * Composition: StreamPipeline chains converters; output of one stage is fed
 * as input to the next. The source program stays read-only — only the
 * streamed output is materialised.
 *
 * Every converter is expected to provide a static method:
 *     static QString parameterSchema();
 * returning a JSON string that describes the converter for the UI.
 *
 * Schema format:
 *   {
 *     "title":       "<human-readable converter name>",
 *     "description": "<short explanation of what the converter does>",
 *     "image":       "<Qt resource path to an illustration>",
 *     "fields":      [ { "name": "...", "type": "bool|int|float|choice", ... } ]
 *   }
 */
class StreamConverter
{
    public:
        virtual ~StreamConverter() = default;

        virtual QList<GCodeItem> push(const GCodeItem &input) = 0;
        virtual QList<GCodeItem> flush() = 0;
        virtual void reset() = 0;

        // Drives the converter over an entire source program and returns a
        // freshly-allocated GCode with the result. The source is not modified.
        // Calls reset() before processing — any pending stream state is dropped.
        GCode* convertAll(GCode &source);
};

#endif // STREAMCONVERTER_H
