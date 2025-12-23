// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "joggingbehavior.h"
#include "core/communicator/communicator.h"
#include "idlebehavior.h"
// #include "pausebehavior.h"
#include "alarmbehavior.h"

JoggingBehavior::JoggingBehavior(JoggindDir direction, double distance, int feedRate, int feedRateZ, QObject *parent)
    : StateBehavior{parent}
    , m_currentDirection(direction)
    , m_feedRate(feedRate)
    , m_feedRateZ(feedRateZ)
    , m_distance(distance)
{
}

JoggingBehavior::JoggingBehavior(QVector3D vector, int feedRate, int feedRateZ, QObject *parent)
    : StateBehavior{parent}
    , m_currentDirection(JoggindDir::None)
    , m_feedRate(feedRate)
    , m_feedRateZ(feedRateZ)
    , m_vector(vector)
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


// void frmMain::jogStep(QVector3D vector)
// {
//     assert(m_communicator->isMachineConfigurationReady());

//     if (ui->jog->isContinuous()) {
//         return;
//     }

//     bool unitsInches = m_communicator->machineConfiguration().unitsInches();
//     vector *= ui->jog->stepSize();

//     m_communicator->sendCommand(
//         CommandSource::System,
//         QString("$J=%5G91X%1Y%2Z%3F%4")
//             .arg(vector.x(), 0, 'f', unitsInches ? 4 : 3)
//             .arg(vector.y(), 0, 'f', unitsInches ? 4 : 3)
//             .arg(vector.z(), 0, 'f', unitsInches ? 4 : 3)
//             .arg(m_configuration.joggingModule().jogFeed())
//             .arg(unitsInches ? "G20" : "G21"),
//         -3
//     );
// }

// void frmMain::jogStart(QVector3D vector)
// {
//     bool unitsInches = m_communicator->machineConfiguration().unitsInches();

//     // Bounds
//     QVector3D b = m_communicator->machineConfiguration().machineBounds();
//     // Current machine coords
//     // @TODO use m_communicator storedVars
//     QVector3D m(
//         m_communicator->toMetric(m_communicator->m_storedVars.Mx()),
//         m_communicator->toMetric(m_communicator->m_storedVars.My()),
//         m_communicator->toMetric(m_communicator->m_storedVars.Mz())
//         );
//     // Distance to bounds
//     QVector3D t;
//     // Minimum distance to bounds
//     double d = 0;
//     if (m_communicator->machineConfiguration().softLimitsEnabled()) {
//         t = QVector3D(vector.x() * b.x() < 0 ? 0 - m.x() : b.x() - m.x(),
//                       vector.y() * b.y() < 0 ? 0 - m.y() : b.y() - m.y(),
//                       vector.z() * b.z() < 0 ? 0 - m.z() : b.z() - m.z());
//         for (int i = 0; i < 3; i++) if ((vector[i] && (qAbs(t[i]) < d)) || (vector[i] && !d)) d = qAbs(t[i]);
//         // Coords not aligned, add some bounds offset
//         d -= unitsInches ? m_communicator->toMetric(0.0005) : 0.005;
//     } else {
//         for (int i = 0; i < 3; i++) if (vector[i] && (qAbs(b[i]) > d)) d = qAbs(b[i]);
//     }

//     // Jog vector
//     QVector3D vec = vector * m_communicator->toInches(d);

//     if (vec.length()) {
//         m_communicator->sendCommand(CommandSource::System, QString("$J=%5G91X%1Y%2Z%3F%4")
//                                         .arg(vec.x(), 0, 'f', unitsInches ? 4 : 3)
//                                         .arg(vec.y(), 0, 'f', unitsInches ? 4 : 3)
//                                         .arg(vec.z(), 0, 'f', unitsInches ? 4 : 3)
//                                         .arg(m_configuration.joggingModule().feed())
//                                         .arg(unitsInches ? "G20" : "G21")
//                                         , -2);
//     }
// }

// void frmMain::jogContinuous()
// {
//     static bool block = false;
//     static QVector3D lastVector(0, 0, 0);

//     if ((ui->jog->isContinuous()) && !block) {
//         if (ui->jog->jogVector() != lastVector) {
//             // Store jog vector before block
//             QVector3D vector = ui->jog->jogVector();

//             // Stop jogging
//             if (lastVector.length()) {
//                 lastVector = vector;
//                 block = true;

//                 m_communicator->sendRealtimeCommand(GRBL_LIVE_JOG_CANCEL);

//                 if (!vector.length()) {
//                     return;
//                 }

//                 QObject *obj = new QObject(this);
//                 connect(m_communicator, &Communicator::deviceStateChanged, obj, [this, obj, vector] (DeviceState state) {
//                     qDebug() << "deviceStateChanged" << (int) state;
//                     if (state != DeviceState::Jog) {
//                         jogStart(vector);
//                         obj->deleteLater();
//                     }
//                 });

//                 block = false;
//             } else {
//                 lastVector = vector;
//                 jogStart(vector);
//             }
//         }
//     }
// }
