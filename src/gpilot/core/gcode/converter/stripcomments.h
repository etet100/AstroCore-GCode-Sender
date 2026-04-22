// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef STRIPCOMMENTS_H
#define STRIPCOMMENTS_H

#include "abstractconverter.h"

// Removes all G-code comments from every line.
// Supports parenthetical style (comment) and semicolon style ; comment.
// Lines that contained only a comment become empty lines.
class StripComments : public AbstractConverter
{
public:
    StripComments();
    bool convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser) override;
};

#endif // STRIPCOMMENTS_H
