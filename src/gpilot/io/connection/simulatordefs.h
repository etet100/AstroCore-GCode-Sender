// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#ifndef SIMULATORDEFS_H
#define SIMULATORDEFS_H

#include <QString>

namespace Simulator {

enum Type { GRBL, FluidNC, UCNC };

enum StopFlag {
    Running       = 0,
    StopRequested = 2,
    Stopped       = 3
};

inline Type typeFromString(const QString& str)
{
    if (str == "grbl")    return GRBL;
    if (str == "fluidnc") return FluidNC;
    if (str == "ucnc")    return UCNC;
    return GRBL;
}

inline QString typeToString(Type type)
{
    static const char* names[] = {"grbl", "fluidnc", "ucnc"};
    return names[type];
}

}

#endif // SIMULATORDEFS_H
