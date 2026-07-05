// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef MODIFYFEEDRATE_H
#define MODIFYFEEDRATE_H

#include "streamconverter.h"

// Scales every F (feed rate) value in the G-code by a percentage.
// 100% = no change, 150% = 1.5x faster, 50% = half speed.
// Only the command part of each line is modified — F values inside
// comments are left untouched.
class ModifyFeedRate : public StreamConverter
{
    public:
        static QString parameterSchema();

        explicit ModifyFeedRate(double percent = 100.0);

        QList<GCodeItem> push(const GCodeItem &input) override;
        QList<GCodeItem> flush() override { return {}; }
        void reset() override {}

        void setPercent(double percent) { m_percent = percent; }
        double percent() const { return m_percent; }

    private:
        double m_percent;
};

#endif // MODIFYFEEDRATE_H
