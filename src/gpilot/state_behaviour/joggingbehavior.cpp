// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "joggingbehavior.h"
#include "core/communicator/communicator.h"
#include "idlebehavior.h"
#include "pausebehavior.h"
#include "alarmbehavior.h"

JoggingBehavior::JoggingBehavior(JoggindDir direction, double distance, int feedRate, QObject *parent)
    : StateBehavior{parent}
    , m_currentDirection(direction)
    , m_feedRate(feedRate)
    , m_distance(distance)
{
}

JoggingBehavior::JoggingBehavior(QVector3D vector, int feedRate, QObject *parent)
    : StateBehavior{parent}
    , m_currentDirection(JoggindDir::None)
    , m_feedRate(feedRate)
    , m_vector(vector)
{
}

void JoggingBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[JoggingBehavior] Entering Jogging State";
    StateBehavior::onEntry(communicator, previous);

    startJogging();
}

void JoggingBehavior::onExit(StateBehavior *next)
{
    stopJogging();
}

void JoggingBehavior::onDeviceStateChanged(DeviceState state)
{
    qDebug() << "[JoggingBehavior] Device State Changed:" << static_cast<int>(state);

    if (state == DeviceState::Jog) {
        qDebug() << "[JoggingBehavior] Device is jogging";
    } else
    if (state == DeviceState::Idle) {
        qDebug() << "[JoggingBehavior] Device is not jogging anymore";
        stopJogging();
        emit transition(this, new IdleBehavior());
    } else if (state == DeviceState::Alarm) {
        // Stop jogging and handle alarm
        stopJogging();
        emit transition(this, new AlarmBehavior());
    } else if (state == DeviceState::Hold0 || state == DeviceState::Hold1) {
        // Machine hold - go to pause state
        emit transition(this, new PauseBehavior(PauseBehavior::PauseSource::Jogging));
    }
}

void JoggingBehavior::onCommandResponse(QString command, QString response, QStringList fullResponse)
{
    if (!command.startsWith("$J=")) {
        // error
    }

    if (response == "ok") {
        m_acked++;
        if (m_distance == JoggingContinuous) {
            // Fill buffer with more jogging commands
            while (m_sent - m_acked < 5) {
                continueJogging();
            }
        }
    } else if (response.startsWith("error")) {
        qDebug() << "[JoggingBehavior] Jogging command error:" << response;
        stopJogging();
    }
}

void JoggingBehavior::continueJogging()
{
    assert(m_jogCommand.length());

    if (!m_isJogging) {
        return;
    }

    qDebug() << "[JoggingBehavior] Continuing jogging: " << m_sent << m_jogCommand;
    m_communicator->sendCommand(CommandSource::GeneralUI, m_jogCommand, TABLE_INDEX_UI);
    m_sent++;
}

void JoggingBehavior::startJogging()
{
    if (!m_communicator) {
        return;
    }

    if (m_currentDirection == JoggindDir::None) {
        stopJogging();

        return;
        // Use m_vector to determine jogging direction and distance
        // ...
    }

    if (m_distance == JoggingContinuous) {
        // Sent multiple small moves to simulate continuous jogging
        m_distance = 1.0;
    }

    m_jogCommand = "$J=";
    switch (m_currentDirection) {
        case JoggindDir::XPlus:
            m_jogCommand += "G91 G21 X" + QString::number(m_distance > 0 ? m_distance : 100);
            break;
        case JoggindDir::XMinus:
            m_jogCommand += "G91 G21 X-" + QString::number(m_distance > 0 ? m_distance : 100);
            break;
        case JoggindDir::YPlus:
            m_jogCommand += "G91 G21 Y" + QString::number(m_distance > 0 ? m_distance : 100);
            break;
        case JoggindDir::YMinus:
            m_jogCommand += "G91 G21 Y-" + QString::number(m_distance > 0 ? m_distance : 100);
            break;
        case JoggindDir::ZPlus:
            m_jogCommand += "G91 G21 Z" + QString::number(m_distance > 0 ? m_distance : 100);
            break;
        case JoggindDir::ZMinus:
            m_jogCommand += "G91 G21 Z-" + QString::number(m_distance > 0 ? m_distance : 100);
            break;
        default:
            return; // Nieznany kierunek
    }

    m_jogCommand += " F" + QString::number(m_feedRate);

    m_isJogging = true;
    continueJogging();
}

void JoggingBehavior::stopJogging()
{
    m_joggingTimer.stop();
    if (!m_communicator || !m_isJogging) {
        return;
    }

    // Wysłanie komendy zatrzymania joggingu
    m_communicator->sendRealtimeCommand(GRBL_LIVE_JOG_CANCEL);
    m_isJogging = false;
    // m_currentDirection = JoggindDir::None;
}

void JoggingBehavior::setJoggingFeedRate(double feedRate)
{
    m_feedRate = feedRate;

    // Jeśli jesteśmy w trakcie joggingu, możemy chcieć zaktualizować prędkość
    // Jednak w większości kontrolerów, aby zmienić prędkość joggingu,
    // trzeba zatrzymać aktualny jogging i rozpocząć nowy z nową prędkością
    if (m_isJogging) {
        JoggindDir currentDir = m_currentDirection;
        stopJogging();
        startJogging();
    }
}
