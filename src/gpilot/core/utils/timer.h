// This file is a part of "GPilot" application.
// Copyright 2024-2026 BTS

#ifndef TIMER_H
#define TIMER_H

#include <QTime>
#include <QDateTime>

class Timer
{
public:
    Timer();

    void startExecution();
    void stopExecution();
    void pauseExecution();
    void resumeExecution();
    void reset();

    QTime elapsedTime() const;
    bool isTracking() const { return m_isTracking; }
    bool isPaused() const { return m_isPaused; }

    qint64 startTimeSeconds() const { return m_startTimeSeconds; }
    qint64 pauseTimeSeconds() const { return m_pauseTimeSeconds; }
    qint64 totalPausedSeconds() const { return m_totalPausedSeconds; }

private:
    qint64 m_startTimeSeconds;
    qint64 m_pauseTimeSeconds;
    qint64 m_totalPausedSeconds;

    bool m_isTracking;
    bool m_isPaused;

    qint64 getCurrentTimeSeconds() const;
};

#endif // TIMER_H