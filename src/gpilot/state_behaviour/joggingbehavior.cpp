// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "joggingbehavior.h"
#include "core/communicator/communicator.h"
#include "idlebehavior.h"
// #include "pausebehavior.h"
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

bool JoggingBehavior::onAboutToChange(StateBehavior *newState, bool forced)
{
    return forced || (newState->inherits("ResetBehavior") && newState->name() == "Reset");
}

StateBehavior::Result JoggingBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[JoggingBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

    startJogging();

    return Result::Ok;
}

StateBehavior::Result JoggingBehavior::onExit(StateBehavior *next)
{
    stopJogging();

    return StateBehavior::onExit(next);
}

void JoggingBehavior::onDeviceStateChanged(DeviceState state)
{
    qDebug() << "[JoggingBehavior] Device State Changed:" << static_cast<int>(state);

    if (state == DeviceState::Jog) {
        qDebug() << "[JoggingBehavior] Device is jogging";
        m_isJoggingState = true;
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
        // emit transition(this, new PauseBehavior(PauseBehavior::PauseSource::Jogging));
    }
}

void JoggingBehavior::onDeviceState(DeviceState state)
{
    if (m_stopping && state == DeviceState::Idle) {
        qDebug() << "[JoggingBehavior] Device is not jogging anymore";
        emit transition(this, new IdleBehavior());
    }
}

bool JoggingBehavior::onCommandResponse(QString command, QString response, QStringList fullResponse)
{
    qDebug() << "[JoggingBehavior] Command Response:" << command << "->" << response;

    if (!command.startsWith("$J=")) {
        // error
    }

    m_acked++;
    if (response == "ok") {
        if (m_distance == JoggingContinuous && !m_stopping) {
            // Fill buffer with more jogging commands
            while (m_sent - m_acked < 5) {
                continueJogging();
            }
        }

        // if (m_stopping && !m_isJoggingState) {
        //     // It means that we having received jogging status
        //     m_communicator->requestStatusUpdate();
        //     waitForStateResponse([this](DeviceState state) {
        //         if (state == DeviceState::Idle) {
        //             emit transition(this, new IdleBehavior());
        //         }
        //     });
        // }
    } else if (response.startsWith("error")) {
        qDebug() << "[JoggingBehavior] Jogging command error:" << response;
        if (response == "error:15") {
            if (!m_stopping) {
                log("Next move exceeds machine limits.", {"Jogging"});
            }
        } else {
            log("Error: " + response, {"Jogging"});
        }

        // stopJogging();
        // m_communicator->clearCommandsAndQueue();
        // m_communicator->requestStatusUpdate();
        // waitForStateResponse([this](DeviceState state) {
        //     if (state == DeviceState::Idle) {
        //         emit transition(this, new IdleBehavior());
        //     }
        // });

        m_communicator->clearQueue(); // Delete unsent jog commands

        if (m_firstCommand) {
            qDebug() << "[JoggingBehavior] First jogging command failed, should be in Idle state";
            emit transition(this, new IdleBehavior());
        }

        m_stopping = true;
    }

    m_firstCommand = false;

    return true;
}

void JoggingBehavior::continueJogging()
{
    assert(m_jogCommand.length());

    if (!m_isJogging) {
        return;
    }

    qDebug() << "[JoggingBehavior] Continuing jogging: " << m_sent << m_jogCommand << m_sent << m_acked;
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

    double distance = m_distance;

    if (m_distance == JoggingContinuous) {
        qDebug() << "[JoggingBehavior] Continuous mode";

        // Sent multiple small moves to simulate continuous jogging
        // Each move will be 1% of the feed rate distance
        distance = m_feedRate / 500.0;
        if (distance < 1.0) {
            distance = 1.0;
        }
    }

    m_jogCommand = "$J=";
    switch (m_currentDirection) {
        case JoggindDir::XPlus:
            m_jogCommand += "G91 G21 X" + QString::number(distance > 0 ? distance : 100);
            break;
        case JoggindDir::XMinus:
            m_jogCommand += "G91 G21 X-" + QString::number(distance > 0 ? distance : 100);
            break;
        case JoggindDir::YPlus:
            m_jogCommand += "G91 G21 Y" + QString::number(distance > 0 ? distance : 100);
            break;
        case JoggindDir::YMinus:
            m_jogCommand += "G91 G21 Y-" + QString::number(distance > 0 ? distance : 100);
            break;
        case JoggindDir::ZPlus:
            m_jogCommand += "G91 G21 Z" + QString::number(distance > 0 ? distance : 100);
            break;
        case JoggindDir::ZMinus:
            m_jogCommand += "G91 G21 Z-" + QString::number(distance > 0 ? distance : 100);
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
    if (!m_isJogging) {
        return;
    }

    qDebug() << "[JoggingBehavior] Stopping jogging";

    m_joggingTimer.stop();
    if (!m_communicator || !m_isJogging) {
        return;
    }

    m_communicator->clearQueue(); // Delete unsent jog commands
    m_communicator->sendRealtimeCommand(GRBL_LIVE_JOG_CANCEL);
    m_isJogging = false;
    m_stopping = true;
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
