// This file is a part of "GPilot" application.
// Copyright 2024-2026 BTS

#include "programtimeestimator.h"
#include "core/gcode/gcode.h"
#include <QDebug>
#include <cmath>

ProgramTimeEstimator::ProgramTimeEstimator(Timer& timer)
    : m_timer(timer)
    , m_estimatedTotalTime(0, 0, 0)
    , m_lastCompletedSegmentIndex(-1)
    , m_progressPercentage(0.0)
    , m_correctionFactor(1.0)
    , m_completedEstimatedTime(0.0)
{
}

QTime ProgramTimeEstimator::calculateEstimatedTime(const QList<LineSegment>& lines,
                                                     int feedOverride,
                                                     int rapidOverride)
{
    m_segmentEstimatedTimes.clear();
    m_segmentEstimatedTimes.reserve(lines.count());

    double totalTimeSeconds = 0.0;

    // Need to cast away const to call non-const methods
    for (int i = 0; i < lines.count(); i++) {
        LineSegment& segment = const_cast<LineSegment&>(lines[i]);
        double segmentTime = calculateSegmentTime(segment, feedOverride, rapidOverride);
        m_segmentEstimatedTimes.append(segmentTime);
        totalTimeSeconds += segmentTime;
    }

    QTime estimatedTime(0, 0, 0);
    estimatedTime = estimatedTime.addSecs(static_cast<int>(totalTimeSeconds));

    m_estimatedTotalTime = estimatedTime;

    return estimatedTime;
}

double ProgramTimeEstimator::calculateSegmentTime(LineSegment& segment,
                                                    int feedOverride,
                                                    int rapidOverride)
{
    double length = (segment.getEnd() - segment.getStart()).length();

    if (qIsNaN(length) || qIsNaN(segment.getSpeed()) || segment.getSpeed() == 0) {
        return 0.0;
    }

    double effectiveSpeed = segment.getSpeed();

    // Apply override based on segment type
    if (!segment.isFastTraverse() && feedOverride != 100) {
        effectiveSpeed *= (feedOverride / 100.0);
    } else if (segment.isFastTraverse() && rapidOverride != 100) {
        effectiveSpeed *= (rapidOverride / 100.0);
    }

    // Time = distance / speed, result in minutes, convert to seconds
    return (length / effectiveSpeed) * 60.0;
}

void ProgramTimeEstimator::updateProgress(GCode& program)
{
    int processedIndex = program.processedCommandIndex();
    int totalCount = program.count();

    if (totalCount > 0) {
        m_progressPercentage = (static_cast<double>(processedIndex) / totalCount) * 100.0;
    }

    // Update completed estimated time
    if (processedIndex > m_lastCompletedSegmentIndex) {
        for (int i = m_lastCompletedSegmentIndex + 1; i <= processedIndex; ++i) {
            if (i >= 0 && i < m_segmentEstimatedTimes.count()) {
                m_completedEstimatedTime += m_segmentEstimatedTimes[i];
            }
        }
        m_lastCompletedSegmentIndex = processedIndex;
    }

    updateCorrectionFactor();
}

void ProgramTimeEstimator::updateCorrectionFactor()
{
    if (!m_timer.isTracking() || m_completedEstimatedTime < 1.0) {
        return;
    }

    QTime elapsed = m_timer.elapsedTime();
    qint64 actualElapsed = elapsed.hour() * 3600 + elapsed.minute() * 60 + elapsed.second();

    if (actualElapsed > 0) {
        // Correction factor = actual time / estimated time
        // If > 1.0, execution is slower than estimated
        // If < 1.0, execution is faster than estimated
        m_correctionFactor = static_cast<double>(actualElapsed) / m_completedEstimatedTime;

        // Smooth the correction factor to avoid jumps (exponential moving average)
        // This gives more weight to recent measurements while keeping history
        static double smoothingFactor = 0.3; // 30% new data, 70% old data
        static double lastCorrectionFactor = 1.0;

        if (m_progressPercentage > 5.0) { // Start smoothing after 5% progress
            m_correctionFactor = smoothingFactor * m_correctionFactor +
                                (1.0 - smoothingFactor) * lastCorrectionFactor;
        }

        lastCorrectionFactor = m_correctionFactor;

        // Clamp correction factor to reasonable bounds (0.5 to 2.0)
        m_correctionFactor = qBound(0.5, m_correctionFactor, 2.0);
    }
}

void ProgramTimeEstimator::resetEstimation()
{
    m_lastCompletedSegmentIndex = -1;
    m_progressPercentage = 0.0;
    m_correctionFactor = 1.0;
    m_completedEstimatedTime = 0.0;
    m_segmentEstimatedTimes.clear();
}

QTime ProgramTimeEstimator::elapsedTime() const
{
    return m_timer.elapsedTime();
}

QTime ProgramTimeEstimator::estimatedTotalTime() const
{
    return m_estimatedTotalTime;
}

QTime ProgramTimeEstimator::remainingTime() const
{
    int totalSeconds = m_estimatedTotalTime.hour() * 3600 +
                       m_estimatedTotalTime.minute() * 60 +
                       m_estimatedTotalTime.second();

    int elapsedSeconds = elapsedTime().hour() * 3600 +
                         elapsedTime().minute() * 60 +
                         elapsedTime().second();

    int remainingSeconds = qMax(0, totalSeconds - elapsedSeconds);

    QTime time(0, 0, 0);
    return time.addSecs(remainingSeconds);
}

QTime ProgramTimeEstimator::estimatedRemainingTimeWithCorrection() const
{
    if (!m_timer.isTracking() || m_segmentEstimatedTimes.isEmpty()) {
        return remainingTime();
    }

    // Calculate remaining estimated time (sum of not yet completed segments)
    double remainingEstimatedTime = 0.0;
    for (int i = m_lastCompletedSegmentIndex + 1; i < m_segmentEstimatedTimes.count(); ++i) {
        remainingEstimatedTime += m_segmentEstimatedTimes[i];
    }

    // Apply correction factor
    double correctedRemainingTime = remainingEstimatedTime * m_correctionFactor;

    QTime time(0, 0, 0);
    return time.addSecs(static_cast<int>(correctedRemainingTime));
}

QString ProgramTimeEstimator::accuracyInfo() const
{
    if (!m_timer.isTracking() || m_correctionFactor == 1.0) {
        return QString("Accuracy: estimating...");
    }

    double accuracyPercent = (1.0 / m_correctionFactor) * 100.0;
    QString trend;

    if (m_correctionFactor > 1.05) {
        trend = "slower than estimated";
    } else if (m_correctionFactor < 0.95) {
        trend = "faster than estimated";
    } else {
        trend = "on track";
    }

    return QString("Accuracy: %1% (%2, factor: %3)")
        .arg(accuracyPercent, 0, 'f', 1)
        .arg(trend)
        .arg(m_correctionFactor, 0, 'f', 2);
}
