// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include <QRegularExpression>
#include "core/communicator/communicator.h"
#include "core/state_behavior/behaviors.h"

ResetBehavior::ResetBehavior(QObject *parent)
    : StateBehavior{parent}
{}

void ResetBehavior::onMachineState(MachineState state)
{
    qDebug() << "[Behavior][Reset] Device State:" << static_cast<int>(state);
    if (m_stage == SentReset) {
        // qDebug() << "[Behavior][Reset] Reset sent, waiting for reset response. Ignoring machine state changes until reset is confirmed.";

        // return;
    }

    if (m_stage != Completed) {
        return;
    }

    qDebug() << "[Behavior][Reset] Device State:" << static_cast<int>(state);
    // // Handle device state changes
    if (state == MachineState::Idle) {
        emit transition(this, new IdleBehavior());

        return;
    } else if (state == MachineState::Alarm) {
        emit transition(this, new AlarmBehavior(m_communicator->lastAlarmCode()));

        return;
    } else {
        qDebug() << "[Behavior][Reset] Unhandled state after reset:" << int(state);
    }
}

void ResetBehavior::onAlarm(int code)
{
    emit transition(this, new AlarmBehavior(code));
}

StateBehavior::Result ResetBehavior::onRawResponse(QString response)
{
    qDebug() << "[Behavior][Reset][Raw response] " << response;

    if (dataIsReset(response)) {
        if (m_stage == SentReset) {
            clearAllTimeouts();
            qDebug() << "[Behavior][Reset] Reset detected in raw response. Sending $$.";

            m_communicator->sendCommand(CommandSource::System, "$$", TABLE_INDEX_UTIL1);

            m_stage = SentSettingsAndOffsets;
        } else {
            // Ignore silently
        }

        return Result::Ok;
    }

    return Result::Unhandled;
}

StateBehavior::Result ResetBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    Q_UNUSED(commandAttributes);

    qDebug() << "[Behavior][Reset] Command Response:" << command << response;

    if (command == "$$" && !cmdStatus.ok && cmdStatus.errorCode == 7) {
        qDebug() << "[Behavior][Reset] Eeprom error during $$, requeue and wait for ok.";

        return Result::ReturnCommandToQueue;
    }
    if (command == "$#" && !cmdStatus.ok && cmdStatus.errorCode == 7) {
        qDebug() << "[Behavior][Reset] Eeprom error during $#, requeue and wait for ok.";

        return Result::ReturnCommandToQueue;
    }

    // if (dataIsReset(response)) {
    //     qDebug() << "[Behavior][Reset] Reset detected in response. Sending $$ and $#.";

    //     m_communicator->sendCommand(CommandSource::System, "$$", TABLE_INDEX_UTIL1);
    //     m_communicator->sendCommand(CommandSource::System, "$#", TABLE_INDEX_UTIL1, true);

    //     return true;
    // }

    if (command == "$$") {
        if (!cmdStatus.ok) {
            qDebug() << "[Behavior][Reset] Error receiving device configuration.";

            return Result::Ok;
        }

        qDebug() << "[Behavior][Reset] Processing device configuration.";
        m_communicator->processDeviceConfiguration(fullResponse);

        m_stage = ReceivedSettings;
        m_communicator->sendCommand(CommandSource::System, "$#", TABLE_INDEX_UTIL1);

        return Result::Ok;
    }

    if (command == "$#") {
        if (!cmdStatus.ok) {
            qDebug() << "[Behavior][Reset] Error receiving offsets.";
            if (cmdStatus.errorCode != 7) {
                emit transition(this, new ErrorBehavior(cmdStatus.errorCode));

                return Result::Ok;
            }

            qDebug() << "[Behavior][Reset] We continue despite the error 7.";
        } else {
            qDebug() << "[Behavior][Reset] Processing offsets.";
            m_communicator->processOffsetsVars(fullResponse);
        }

        qDebug() << "[Behavior][Reset] Reset completed.";
        m_communicator->queryMachineState();

        // if (m_state == DeviceState::Alarm) {
        //     emit transition(this, new IdleBehavior());
        // } else {
        //     qDebug() << "[ConnectingBehavior] Unhandled state after reset:" << int(m_state);
        // }

        m_stage = Completed;

        return Result::Ok;
    }

    // if (command == "$G") {
    //     m_communicator->processGCodeParserState(commandAttributes, response.first());
    // }

    return Result::Unhandled;
}

StateBehavior::Result ResetBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    qDebug() << "[Behavior][Reset] Entry";
    StateBehavior::onEntry(communicator, previous);

    qDebug() << "[Behavior][Reset] Clearing command queues.";
    communicator->clearCommandsAndQueue();

    qDebug() << "[Behavior][Reset] Soft reset";
    communicator->connection()->sendByteArray(QByteArray(1, GRBL_LIVE_SOFT_RESET));
    setTimeout(100, [this]() {
        qWarning() << "[Behavior][Reset] Timeout: no response after reset.";
        if (m_stage == SentReset) {
            qWarning() << "[Behavior][Reset] Timeout: no reset sequence received within 100ms.";

            emit transition(this, new ErrorBehavior(0));

            return;
        }
    });
    m_stage = SentReset;

    communicator->startQueryingMachineState();

    return Result::Ok;
}

bool ResetBehavior::dataIsReset(QString data)
{
    // "GRBL" in either case, optionally followed by a number of non-whitespace characters,
    // followed by a version number in the format x.y.
    // This matches e.g.
    // Grbl 1.1h ['$' for help]
    // GrblHAL 1.1f ['$' or '' for help]
    // Grbl 1.8 [uCNC v1.8.8 '$' for help]
    // Gcarvin ?? https://github.com/inventables/gCarvin
    static QRegularExpression re("^(GrblHAL|GRBL|Grbl|GCARVIN|uCNC)\\s\\d\\.\\d.", QRegularExpression::CaseInsensitiveOption);
    if (!data.contains(re)) {
        return false;
    }

    if (data.contains("GrblHAL")) {
        logSignal("Detected GrblHAL device.");
    } else if (data.contains("GCARVIN")) {
        logSignal("Detected gCarvin device.");
    } else if (data.contains("uCNC")) {
        logSignal("Detected uCNC device.");
    } else if (data.contains("FluidNC")) {
        logSignal("Detected FluidNC device.");
    } else if (data.contains("Grbl")) {
        logSignal("Detected GRBL device.");
    } else {
        logSignal("Detected unknown device: " + data);
    }

    return true;
}
