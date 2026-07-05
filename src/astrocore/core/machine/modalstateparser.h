// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef MODALSTATEPARSER_H
#define MODALSTATEPARSER_H

#include <QString>
#include <optional>

struct ModalState {
    // Current coordinate system (G54–G59)
    QString coordinateSystem;
    // Active work plane (G17, G18, G19)
    QString workPlane;
    // Units (G20 = inches, G21 = mm)
    QString units;
    // Motion mode (G90 = absolute, G91 = relative)
    QString motionMode;
    // Feed rate mode (G93, G94, G95)
    QString feedMode;
    // Spindle mode (M3 = CW, M4 = CCW, M5 = off)
    QString spindleMode;

    int feedRate = -1;
    int spindleSpeed = -1;

    QString raw;

    QString toString() const;
};

// Parses a modal state line like [G54 G17 G21 G90 G94 M5 M9 T0 F0 S0].
class ModalStateParser
{
    public:
        static std::optional<ModalState> parse(const QString &line);
};

#endif // MODALSTATEPARSER_H
