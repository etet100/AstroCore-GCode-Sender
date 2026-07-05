// This file is a part of "AstroCore" application.
// Copyright 2024-2026 BTS

#ifndef PROGRAMTIMEESTIMATOR_H
#define PROGRAMTIMEESTIMATOR_H

#include <QTime>
#include <QDateTime>
#include <QList>
#include "core/gcode/parser/linesegment.h"
#include "timer.h"

class GCode;

class ProgramTimeEstimator
{
public:
    ProgramTimeEstimator(Timer& timer);

    QTime calculateEstimatedTime(const QList<LineSegment>& lines,
                                  int feedOverride = 100,
                                  int rapidOverride = 100);

    void updateProgress(GCode& program);

    QTime elapsedTime() const;
    QTime estimatedTotalTime() const;
    QTime remainingTime() const;
    QTime estimatedRemainingTimeWithCorrection() const;

    // Correction factor (1.0 = perfect estimate, >1.0 = slower than estimated, <1.0 = faster)
    double correctionFactor() const { return m_correctionFactor; }

    // Progress percentage (0-100)
    double progressPercentage() const { return m_progressPercentage; }
    // Check if estimation is active
    bool isTracking() const { return m_timer.isTracking(); }
    bool isPaused() const { return m_timer.isPaused(); }

    // Estimation accuracy info
    QString accuracyInfo() const;

    // Reset estimation state (but not timer)
    void resetEstimation();

private:
    Timer& m_timer;
    QTime m_estimatedTotalTime;

    // Segment tracking for detailed correction
    QList<double> m_segmentEstimatedTimes; // in seconds
    int m_lastCompletedSegmentIndex;

    double m_progressPercentage;
    double m_correctionFactor;
    double m_completedEstimatedTime; // sum of estimated time for completed segments

    double calculateSegmentTime(LineSegment& segment, int feedOverride, int rapidOverride);
    void updateCorrectionFactor();
};

#endif // PROGRAMTIMEESTIMATOR_H
