// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef SHAKINGGCODE_H
#define SHAKINGGCODE_H

#include "streamconverter.h"
#include "core/gcode/parser/gcodeparser.h"
#include <QVector3D>
#include <QRandomGenerator>
#include <optional>

/**
 * Test/example converter that intentionally distorts movement paths.
 *
 * Features:
 *   - Segments movement lines every N mm
 *   - Adds random XYZ offset to each segment endpoint (±maxOffset)
 *   - Optionally varies feed rate (±feedRateVariation)
 *
 * Lookahead: holds back the current movement until the next push() arrives,
 * so it can avoid distorting the endpoint that is the start of an arc
 * (offsetting an arc start would change its geometry).
 */
class ShakingGCode : public StreamConverter
{
    public:
        static QString parameterSchema();

        explicit ShakingGCode(double segmentLength = 5.0,
                              double maxOffset = 1.0,
                              bool shakeZ = false);
        ~ShakingGCode() override;

        QList<GCodeItem> push(const GCodeItem &input) override;
        QList<GCodeItem> flush() override;
        void reset() override;

        void setSegmentLength(double length) { m_segmentLength = length; }
        double segmentLength() const { return m_segmentLength; }

        void setMaxOffset(double offset) { m_maxOffset = offset; }
        double maxOffset() const { return m_maxOffset; }

        void setShakeZ(bool shakeZ) { m_shakeZ = shakeZ; }
        bool shakeZ() const { return m_shakeZ; }

        void setFeedRateVariation(double variation) { m_feedRateVariation = variation; }
        double feedRateVariation() const { return m_feedRateVariation; }

        void setSeed(quint32 seed);

    private:
        GcodeParser*       m_parser;
        QRandomGenerator*  m_random;
        bool               m_ownsRandom;
        std::optional<GCodeItem> m_pending;

        double m_segmentLength;      // Segment every N mm
        double m_maxOffset;          // Max random offset in mm (±)
        double m_feedRateVariation;  // Feed rate variation (0.0-1.0, default 0.2 = ±20%)
        bool   m_shakeZ;

        QList<GCodeItem> processOne(const GCodeItem &item, bool nextIsArc);
        QList<QVector3D> segmentLine(const QVector3D &start, const QVector3D &end, bool nextIsArc);

        void applyRandomOffset(QVector3D &point);
        double getRandomFeedRate(double originalFeedRate);
        QString generateGCodeLine(const QVector3D &start, const QVector3D &end,
                                  const GCodeItem &originalItem, bool isFirstSegment);
};

#endif // SHAKINGGCODE_H
