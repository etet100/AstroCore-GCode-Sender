// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef MODIFYFEEDRATE_H
#define MODIFYFEEDRATE_H

#include "abstractconverter.h"

// Scales every F (feed rate) value in the G-code by a percentage.
// 100% = no change, 150% = 1.5x faster, 50% = half speed.
// Only the command part of each line is modified — F values inside
// comments are left untouched.
class ModifyFeedRate : public AbstractConverter
{
public:
    static QString parameterSchema();

    explicit ModifyFeedRate(double percent = 100.0);

    bool convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser) override;

    void setPercent(double percent) { m_percent = percent; }
    double percent() const { return m_percent; }

private:
    double m_percent;
};

#endif // MODIFYFEEDRATE_H
