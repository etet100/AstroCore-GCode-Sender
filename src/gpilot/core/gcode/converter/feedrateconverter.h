// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef FEEDRATECONVERTER_H
#define FEEDRATECONVERTER_H

#include "streamconverter.h"
#include <optional>

// Example: simple stateless 1->1 converter.
class FeedRateConverter : public StreamConverter
{
    public:
        static QString parameterSchema();

        explicit FeedRateConverter(double multiplier = 1.0);

        QList<GCodeItem> push(const GCodeItem &input) override;
        QList<GCodeItem> flush() override { return {}; }
        void reset() override {}

        void setMultiplier(double multiplier) { m_multiplier = multiplier; }
        double getMultiplier() const { return m_multiplier; }

    private:
        double m_multiplier;
};

// Example: simple stateless 1->1 converter modifying coordinates.
class CoordinateOffsetConverter : public StreamConverter
{
    public:
        static QString parameterSchema();

        explicit CoordinateOffsetConverter(double offsetX = 0.0,
                                           double offsetY = 0.0,
                                           double offsetZ = 0.0);

        QList<GCodeItem> push(const GCodeItem &input) override;
        QList<GCodeItem> flush() override { return {}; }
        void reset() override {}

        void setOffset(double x, double y, double z);

    private:
        double m_offsetX;
        double m_offsetY;
        double m_offsetZ;

        QString modifyCoordinate(const QString &arg, char axis, double offset);
};

// Example: 1-line lookahead via a one-slot internal buffer.
// Holds back the current item until the next push() arrives, so it can
// inspect the following command before deciding whether to annotate.
class SafeSpindleStopConverter : public StreamConverter
{
    public:
        static QString parameterSchema();

        SafeSpindleStopConverter() = default;

        QList<GCodeItem> push(const GCodeItem &input) override;
        QList<GCodeItem> flush() override;
        void reset() override { m_pending.reset(); }

    private:
        std::optional<GCodeItem> m_pending;
};

// Example: N-line lookahead via a ring of pending items.
// Note: the original example also did a full-program scan (counting all
// movement lines) — that is incompatible with streaming and was removed.
// If a stage genuinely needs whole-program statistics, compute them once
// up-front and pass the result in via the constructor.
class MovementOptimizerConverter : public StreamConverter
{
    public:
        static QString parameterSchema();

        MovementOptimizerConverter() = default;

        QList<GCodeItem> push(const GCodeItem &input) override;
        QList<GCodeItem> flush() override;
        void reset() override { m_buffer.clear(); }

    private:
        static constexpr int LOOKAHEAD = 5;
        QList<GCodeItem> m_buffer;

        GCodeItem annotate(const GCodeItem &item) const;
};

#endif // FEEDRATECONVERTER_H
