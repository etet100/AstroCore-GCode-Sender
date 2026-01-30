// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef EXAMPLECONVERTER_H
#define EXAMPLECONVERTER_H

#include "converter.h"

class FeedRateConverter : public Converter
{
    public:
        explicit FeedRateConverter(double multiplier = 1.0);

        bool convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser) override;
        void reset() override;

        void setMultiplier(double multiplier) { m_multiplier = multiplier; }
        double getMultiplier() const { return m_multiplier; }

    private:
        double m_multiplier;
};

class CoordinateOffsetConverter : public Converter
{
    public:
        explicit CoordinateOffsetConverter(double offsetX = 0.0,
                                          double offsetY = 0.0,
                                          double offsetZ = 0.0);

        bool convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser) override;
        void reset() override;
        bool needsParser() const override { return true; }

        void setOffset(double x, double y, double z);

    private:
        double m_offsetX;
        double m_offsetY;
        double m_offsetZ;

        QString modifyCoordinate(const QString &arg, char axis, double offset);
};

// Example: lookahead usage
class SafeSpindleStopConverter : public Converter
{
    public:
        SafeSpindleStopConverter();

        bool convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser) override;
        int needsLookahead() const override { return 1; }
};

// Example: full G-Code access
class MovementOptimizerConverter : public Converter
{
    public:
        MovementOptimizerConverter();

        bool convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser) override;
        bool needsFullGCode() const override { return true; }
        int needsLookahead() const override { return 5; }
};

#endif // EXAMPLECONVERTER_H
