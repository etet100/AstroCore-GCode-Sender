// This file is a part of "GPilot" application.
// Copyright 2024-2026 BTS

#ifndef PROGRAMTIMEESTIMATOR_H
#define PROGRAMTIMEESTIMATOR_H

#include <QTime>
#include <QDateTime>
#include <QList>
#include "core/gcode/parser/linesegment.h"

class GCode;

class ProgramTimeEstimator
{
public:
    ProgramTimeEstimator();

    QTime calculateEstimatedTime(const QList<LineSegment>& lines,
                                  int feedOverride = 100,
                                  int rapidOverride = 100);

    void startExecution();
    void stopExecution();
    void pauseExecution();
    void resumeExecution();
    void reset();

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
    bool isTracking() const { return m_isTracking; }
    bool isPaused() const { return m_isPaused; }

    // Estimation accuracy info
    QString accuracyInfo() const;

private:
    // Time tracking
    qint64 m_startTimeSeconds;
    qint64 m_pauseTimeSeconds;
    qint64 m_totalPausedSeconds;
    QTime m_estimatedTotalTime;

    // Segment tracking for detailed correction
    QList<double> m_segmentEstimatedTimes; // in seconds
    int m_lastCompletedSegmentIndex;

    double m_progressPercentage;
    double m_correctionFactor;
    double m_completedEstimatedTime; // sum of estimated time for completed segments

    bool m_isTracking;
    bool m_isPaused;

    double calculateSegmentTime(LineSegment& segment, int feedOverride, int rapidOverride);
    void updateCorrectionFactor();
    qint64 getCurrentTimeSeconds() const;
};

#endif // PROGRAMTIMEESTIMATOR_H
