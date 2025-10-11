// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "arcstolines.h"
#include "core/gcode/gcode.h"

ArcsToLines::ArcsToLines(GCode &data) : Converter(data)
{
}

GCode &ArcsToLines::convert()
{
    for (auto &cmd : m_data) {
        if (cmd.isArc()) {
            // QList<GCodeCommand> lineCommands = cmd.toLines();
            // if (!lineCommands.isEmpty()) {
            //     cmd = lineCommands.takeFirst();
            //     for (const auto &lineCmd : lineCommands) {
            //         // m_data.insertCommandAfter(cmd, lineCmd);
            //         cmd = lineCmd;
            //     }
            // }
        }
    }


}
