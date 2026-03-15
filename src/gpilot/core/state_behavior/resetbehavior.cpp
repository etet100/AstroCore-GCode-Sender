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

void ResetBehavior::onAlarm(int code)
{
    emit transition(this, new AlarmBehavior(code));
}

StateBehavior::Result ResetBehavior::onRawResponse(QString response)
{
    qDebug() << "[Behavior][Reset][Raw response]" << response;

    if (dataIsReset(response)) {
        if (m_stage == SentReset) {
            clearAllTimeouts();
            qDebug() << "[Behavior][Reset] Reset header received. Handing off to HandshakeBehavior.";
            m_stage = Completed;
            emit transition(this, new HandshakeBehavior());
        }

        return Result::Ok;
    }

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
