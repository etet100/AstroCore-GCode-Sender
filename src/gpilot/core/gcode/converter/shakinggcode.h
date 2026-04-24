// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS
// Test/example converter for G-code modification

#ifndef SHAKINGGCODE_H
#define SHAKINGGCODE_H

#include "abstractbatchconverter.h"
#include "core/gcode/parser/gcodeparser.h"
#include <QVector3D>
#include <QObject>
#include <QRandomGenerator>

/**
 * ShakingGCode is a test converter that demonstrates G-code modification.
 *
 * Features:
 * - Segments movement lines every 5mm
 * - Adds random XYZ offset to each segment endpoint (±1mm)
 * - Randomly modifies feed rate (±20%)
 *
 * Purpose: Testing, demonstration, intentional path distortion for testing
 *
 * Usage:
 *   ShakingGCode shaker(5.0, 1.0);
 *   shaker.setGCode(originalGCode);
 *   GCode* result = shaker.convertAll();
 */
class ShakingGCode : public QObject, public AbstractBatchConverter
{
    Q_OBJECT

    public:
        static QString parameterSchema();

        explicit ShakingGCode(double segmentLength = 5.0,
                             double maxOffset = 1.0,
                             bool shakeZ = false,
                             QObject *parent = nullptr);
        ~ShakingGCode() override;

        // AbstractBatchConverter implementation
        void setGCode(GCode *gcode) override;
        int convertNext(int count) override;
        void reset() override;
        bool hasMore() const override;
        int currentPosition() const override { return m_currentIndex; }
        int totalLines() const override;
        GCode* convertAll() override;

        void setSegmentLength(double length) { m_segmentLength = length; }
        double segmentLength() const { return m_segmentLength; }

        void setMaxOffset(double offset) { m_maxOffset = offset; }
        double maxOffset() const { return m_maxOffset; }

        void setShakeZ(bool shakeZ) { m_shakeZ = shakeZ; }
        bool shakeZ() const { return m_shakeZ; }

        void setFeedRateVariation(double variation) { m_feedRateVariation = variation; }
        double feedRateVariation() const { return m_feedRateVariation; }

        void setSeed(quint32 seed);

    signals:
        void progressChanged(int current, int total);

    private:
        GcodeParser* m_parser;
        GCode* m_gcode;
        QRandomGenerator* m_random;

        double m_segmentLength;      // Segment every N mm
        double m_maxOffset;          // Max random offset in mm (±)
        double m_feedRateVariation;  // Feed rate variation (0.0-1.0, default 0.2 = ±20%)
        bool m_shakeZ;
        int m_currentIndex;

        // Helper methods
        QList<GCodeItem> processLine(const GCodeItem &item, bool nextIsArc = false);
        QList<QVector3D> segmentLine(const QVector3D &start, const QVector3D &end, bool nextIsArc = false);

        void applyRandomOffset(QVector3D &point);
        double getRandomFeedRate(double originalFeedRate);
        QString generateGCodeLine(const QVector3D &start, const QVector3D &end,
                                  const GCodeItem &originalItem, bool isFirstSegment);
};

#endif // SHAKINGGCODE_H
