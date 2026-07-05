// This file is a part of "AstroCore" application.
// Copyright 2024-2026 BTS

#include "timer.h"

Timer::Timer()
    : m_startTimeSeconds(0)
    , m_pauseTimeSeconds(0)
    , m_totalPausedSeconds(0)
    , m_isTracking(false)
    , m_isPaused(false)
{
}

void Timer::startExecution()
{
    if (!m_isTracking) {
        m_startTimeSeconds = getCurrentTimeSeconds();
        m_isTracking = true;
        m_isPaused = false;
        m_totalPausedSeconds = 0;
    } else if (m_isPaused) {
        // Resume from pause
        qint64 currentTime = getCurrentTimeSeconds();
        m_totalPausedSeconds += (currentTime - m_pauseTimeSeconds);
        m_isPaused = false;
    }
}

void Timer::stopExecution()
{
    if (m_isTracking) {
        m_isTracking = false;
        m_isPaused = false;
    }
}

void Timer::pauseExecution()
{
    if (m_isTracking && !m_isPaused) {
        m_pauseTimeSeconds = getCurrentTimeSeconds();
        m_isPaused = true;
    }
}

void Timer::resumeExecution()
{
    if (m_isTracking && m_isPaused) {
        qint64 currentTime = getCurrentTimeSeconds();
        m_totalPausedSeconds += (currentTime - m_pauseTimeSeconds);
        m_isPaused = false;
    }
}

void Timer::reset()
{
    m_startTimeSeconds = 0;
    m_pauseTimeSeconds = 0;
    m_totalPausedSeconds = 0;
    m_isTracking = false;
    m_isPaused = false;
}

QTime Timer::elapsedTime() const
{
    if (!m_isTracking) {
        return QTime(0, 0, 0);
    }

    qint64 currentTime = getCurrentTimeSeconds();
    qint64 elapsed = currentTime - m_startTimeSeconds - m_totalPausedSeconds;

    if (m_isPaused) {
        elapsed -= (currentTime - m_pauseTimeSeconds);
    }

    return QTime(0, 0, 0).addSecs(elapsed);
}

qint64 Timer::getCurrentTimeSeconds() const
{
    return QDateTime::currentSecsSinceEpoch();
}