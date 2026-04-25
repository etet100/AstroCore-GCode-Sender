// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef STRIPCOMMENTS_H
#define STRIPCOMMENTS_H

#include "streamconverter.h"

// Removes all G-code comments. Supports parenthetical (comment) and
// semicolon ; comment styles. Lines that contained only a comment are
// dropped entirely from the stream.
class StripComments : public StreamConverter
{
    public:
        static QString parameterSchema();

        StripComments() = default;

        QList<GCodeItem> push(const GCodeItem &input) override;
        QList<GCodeItem> flush() override { return {}; }
        void reset() override {}
};

#endif // STRIPCOMMENTS_H
