// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "joggingbehavior.h"
#include "core/communicator/communicator.h"
#include "idlebehavior.h"
// #include "pausebehavior.h"
#include "alarmbehavior.h"

JoggingBehavior::JoggingBehavior(QVector3D vector, double distance, bool continuous, int feedRate, int feedRateZ, QObject *parent)
    : StateBehavior{parent}
    , m_joggingVector(vector)
    , m_feedRate(feedRate)
    , m_feedRateZ(feedRateZ)
    , m_continuous(continuous)
    , m_distance(distance)
{
}

JoggingBehavior::JoggingBehavior(int feedRate, int feedRateZ, QObject *parent)
    : StateBehavior{parent}
    , m_joggingVector(QVector3D(0, 0, 0))
    , m_feedRate(feedRate)
    , m_feedRateZ(feedRateZ)
{
}

bool JoggingBehavior::onAboutToChange(StateBehavior *newState, bool forced)
{
    return forced || (newState->inherits("ResetBehavior") && newState->description() == "Reset");
}

StateBehavior::Result JoggingBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    qDebug() << "[JoggingBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

    startJogging();
    communicator->startQueryingMachineState();

    return Result::Ok;
}

StateBehavior::Result JoggingBehavior::onExit(StateBehavior *next)
{
    m_communicator->stopQueryingMachineState();
    stopJogging();

    return StateBehavior::onExit(next);
}

void JoggingBehavior::onMachineStateChanged(MachineState state)
{
    qDebug() << "[JoggingBehavior] Device State Changed:" << static_cast<int>(state);

    if (state == MachineState::Jog) {
        qDebug() << "[JoggingBehavior] Device is jogging";
        m_isJoggingState = true;
    } else
    if (state == MachineState::Idle) {
        qDebug() << "[JoggingBehavior] Device is not jogging anymore";
        stopJogging();
        emit transition(this, new IdleBehavior());
    } else if (state == MachineState::Alarm) {
        // Stop jogging and handle alarm
        stopJogging();
        emit transition(this, new AlarmBehavior());
    } else if (state == MachineState::Hold0 || state == MachineState::Hold1) {
        // Machine hold - go to pause state
        // emit transition(this, new PauseBehavior(PauseBehavior::PauseSource::Jogging));
    }
}

void JoggingBehavior::onMachineState(MachineState state)
{
    if (m_stopping && state == MachineState::Idle) {
        qDebug() << "[JoggingBehavior] Device is not jogging anymore";
        emit transition(this, new IdleBehavior());
    }
}

StateBehavior::Result JoggingBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    qDebug() << "[JoggingBehavior] Command Response:" << command << "->" << response;

    if (!command.startsWith("$J=")) {
        // error
    }

    m_acked++;
    if (response == "ok") {
        if (m_continuous && !m_stopping) {
            // In timer-based continuous mode, we don't refill buffer here based on ack.
            //The timer handles the periodic sending.
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

    return Result::Ok;
}

void JoggingBehavior::continueJogging()
{
    assert(m_jogCommand.length());

    if (!m_isJogging) {
        return;
    }

    m_communicator->sendCommand(CommandSource::GeneralUI, m_jogCommand, TABLE_INDEX_UI);
    m_sent++;
}

void JoggingBehavior::fillBuffer()
{
    double realDistance = (m_communicator->machinePos() - m_startMachinePos).length();
    double remaining = m_sent * m_segmentDist - realDistance;

    qDebug() << "[JoggingBehavior] fillBuffer: real=" << realDistance
             << "sent=" << m_sent << "remaining=" << remaining
             << "target=" << m_targetLookahead;

    while (!m_stopping && remaining < m_targetLookahead
           && !m_communicator->willOverflowBuffer(m_jogCommand)) {
        continueJogging();
        remaining += m_segmentDist;
    }
}

void JoggingBehavior::buildJogCommand(double distance)
{
    m_jogCommand = "$J=G91 G21";
    if (m_joggingVector.z() != 0) {
        m_jogCommand += " Z" + QString::number(distance * m_joggingVector.z());
        m_jogCommand += " F" + QString::number(m_feedRateZ);
    } else {
        if (m_joggingVector.x() != 0) {
            m_jogCommand += " X" + QString::number(distance * m_joggingVector.x());
        }
        if (m_joggingVector.y() != 0) {
            m_jogCommand += " Y" + QString::number(distance * m_joggingVector.y());
        }
        m_jogCommand += " F" + QString::number(m_feedRate);
    }
}

void JoggingBehavior::startJogging()
{
    if (!m_communicator) {
        return;
    }

    if (m_joggingVector.length() == 0) {
        stopJogging();
        return;
    }

    if (m_continuous) {
        qDebug() << "[JoggingBehavior] Continuous mode";

        // Segment covers exactly one timer interval of travel at the configured feed rate.
        // This gives the shortest possible segments while maintaining continuous motion.
        double effectiveFeedRate = (m_joggingVector.z() != 0) ? m_feedRateZ : m_feedRate;
        m_segmentDist = std::max(effectiveFeedRate / 60000.0 * TIMER_INTERVAL_MS, 0.05);
        if (m_joggingVector.x() != 0 && m_joggingVector.y() != 0) {
            m_segmentDist /= std::sqrt(2.0);
        }

        // Keep 2.5 segments ahead: enough to cover one full poll cycle with margin,
        // so the planner never empties between timer ticks.
        m_targetLookahead = 2.5 * m_segmentDist;

        buildJogCommand(m_segmentDist);

        m_startMachinePos = m_communicator->machinePos();
        m_isJogging = true;
        m_sent = 0;

        // Pre-fill to target lookahead before the first timer tick.
        fillBuffer();

        disconnect(&m_joggingTimer, &QTimer::timeout, nullptr, nullptr);
        connect(&m_joggingTimer, &QTimer::timeout, this, [this]() {
            m_communicator->queryMachineState();

            if (m_stopping || !m_isJogging) {
                m_joggingTimer.stop();
                return;
            }

            fillBuffer();
        });

        m_communicator->stopQueryingMachineState();
        m_joggingTimer.start(TIMER_INTERVAL_MS);
        return;
    }

    // Non-continuous: single fixed-distance command.
    double distance = m_distance;
    if (m_joggingVector.x() != 0 && m_joggingVector.y() != 0) {
        distance /= std::sqrt(2.0);
    }

    buildJogCommand(distance);
    m_startMachinePos = m_communicator->machinePos();
    m_isJogging = true;
    m_sent = 0;
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

    qDebug() << "[JoggingBehavior] Send JOG CANCEL and clear queue";
    m_communicator->clearQueue();
    m_communicator->sendRealtimeCommand(GRBL_LIVE_JOG_CANCEL);
    m_isJogging = false;
    m_stopping = true;
    m_communicator->startQueryingMachineState();
}

void JoggingBehavior::setJoggingFeedRate(double feedRate)
{
    m_feedRate = feedRate;

    // If we are currently jogging, we may want to update the speed
    // However, in most controllers, to change the jogging speed,
    // we need to stop the current jogging and start a new one with the new speed
    if (m_isJogging) {
        stopJogging();
        startJogging();
    }
}

