// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "joggingbehavior.h"
#include "core/communicator/communicator.h"
#include "idlebehavior.h"
// #include "pausebehavior.h"
#include "alarmbehavior.h"

JoggingBehavior::JoggingBehavior(JoggindDir direction, double distance, bool continuous, int feedRate, int feedRateZ, QObject *parent)
    : StateBehavior{parent}
    , m_currentDirection(direction)
    , m_feedRate(feedRate)
    , m_feedRateZ(feedRateZ)
    , m_continuous(continuous)
    , m_distance(distance)
{
}

JoggingBehavior::JoggingBehavior(int feedRate, int feedRateZ, QObject *parent)
    : StateBehavior{parent}
    , m_currentDirection(JoggindDir::None)
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
    }

    double distance = m_distance;

    if (m_continuous) {
        qDebug() << "[JoggingBehavior] Continuous mode";

        // Time = 0.1s
        // 0.1/60 min, so distance = m_feedRate * (0.1/60) = m_feedRate / 600
        distance = std::max(m_feedRate / 600.0, 0.05);

        // Timer interval should be slightly less than the move duration to ensure continuity
        // Move duration: 100ms
        // Timer interval: 50ms
        // This means we send command every 50ms, adding 100ms of motion.
        // The buffer will grow by 50ms every 50ms.
        // To prevent buffer overflow and run-on, we stop filling if we are too far ahead.
        // But since we can't easily check buffer depth, we rely on the user releasing the key
        // and sending Jog Cancel.
        // Or better: match the timer to the duration closely.
        // Interval: 80ms. Move: 100ms. Growth: 20ms/tick. Fills 1s buffer in 4s.
        // Let's use 50ms timer and recalculate distance for ~70ms motion?
        // Let's try: Timer 50ms, Motion 0.1s (100ms).
        int timerInterval = 80;

        m_compensation.reset();

        disconnect(&m_joggingTimer, &QTimer::timeout, nullptr, nullptr);
        connect(&m_joggingTimer, &QTimer::timeout, this, [this, distance]() {
            m_communicator->queryMachineState();

            if (m_stopping || !m_isJogging) {
                m_joggingTimer.stop();
                return;
            }

            performDynamicCompensation(distance);

            if (!m_communicator->willOverflowBuffer(m_jogCommand)) {
                continueJogging();
            }
        });
        m_joggingTimer.start(timerInterval);
    }

    int feedRate = m_feedRate;

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
            feedRate = m_feedRateZ;
            break;
        case JoggindDir::ZMinus:
            m_jogCommand += "G91 G21 Z-" + QString::number(distance > 0 ? distance : 100);
            feedRate = m_feedRateZ;
            break;
        default:
            return; // Nieznany kierunek
    }
    m_jogCommand += " F" + QString::number(feedRate);

    m_startMachinePos = m_communicator->machinePos();

    m_isJogging = true;
    m_sent = 0;
    continueJogging();
    if (m_continuous) {
        m_communicator->stopQueryingMachineState();
    }
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

    // Jeśli jesteśmy w trakcie joggingu, możemy chcieć zaktualizować prędkość
    // Jednak w większości kontrolerów, aby zmienić prędkość joggingu,
    // trzeba zatrzymać aktualny jogging i rozpocząć nowy z nową prędkością
    if (m_isJogging) {
        JoggindDir currentDir = m_currentDirection;
        stopJogging();
        startJogging();
    }
}

void JoggingBehavior::performDynamicCompensation(double distance)
{
    QVector3D m_machinePosDiff = m_communicator->machinePos() - m_startMachinePos;
    double realDistance = m_machinePosDiff.length();
    // TODO: Units conversion
    // if (m_communicator->machineConfiguration().unitsInches()) {
    //     realDistance *= 25.4;
    // }

    double sentDistance = m_sent * distance;

    // Target strategy: Maintain buffer of ~2.5 segments.
    // We virtually increase the real distance by 2.5 segments.
    // We want SentDistance to be equal to this AugmentedRealDistance.
    double augmentedRealDistance = realDistance + (2.5 * distance);

    // Error = Sent - AugmentedTarget.
    // Positive means Sent > AugmentedTarget (Buffer too big) -> Slow down
    // Negative means Sent < AugmentedTarget (Buffer too small) -> Speed up
    double currentDiff = sentDistance - augmentedRealDistance;

    double smoothedDiff = m_compensation.addDiff(currentDiff);

    int currentInterval = m_joggingTimer.interval();
    int newInterval = currentInterval;

    // Simple incremental control loop
    double deadband = 0.1 * distance;

    if (smoothedDiff > deadband) {
        newInterval += 1;
    } else if (smoothedDiff < -deadband) {
        newInterval -= 1;
    }

    if (smoothedDiff > distance) newInterval += 2;
    else if (smoothedDiff < -distance) newInterval -= 2;
    newInterval = qBound(20, newInterval, 200);

    if (newInterval != currentInterval) {
        m_joggingTimer.setInterval(newInterval);
    }

    qDebug() << "[JoggingBehavior] Real:" << realDistance << "Target(Adj):" << augmentedRealDistance
             << "Sent:" << sentDistance << "Error:" << smoothedDiff << "Interval:" << newInterval;
}

void JoggingBehavior::SendingIntervalCompensation::reset()
{
    historyIndex = 0;
    historyCount = 0;
}

double JoggingBehavior::SendingIntervalCompensation::addDiff(double diff)
{
    diffHistory[historyIndex] = diff;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    if (historyCount < HISTORY_SIZE) {
        historyCount++;
    }

    return smoothedDiff();
}

double JoggingBehavior::SendingIntervalCompensation::smoothedDiff() const
{
    if (historyCount == 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < historyCount; ++i) {
        sum += diffHistory[i];
    }

    return sum / historyCount;
}
