// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include <QRegularExpression>
#include "core/communicator/communicator.h"
#include "core/state_behavior/behaviors.h"

ResetBehavior::ResetBehavior(QObject *parent)
    : AbstractStateBehavior{parent}
{}

void ResetBehavior::onAlarm(int code)
{
    emit transition(this, new AlarmBehavior(code));
}

AbstractStateBehavior::Result ResetBehavior::onRawResponse(QString response)
{
    qDebug() << "[Behavior][Reset][Raw response]" << response;

    auto machineType = detectMachineType(response);
    if (machineType != MachineType::Unknown) {
        m_communicator->setMachineType(machineType);
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

AbstractStateBehavior::Result ResetBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][Reset] Entry";

    qDebug() << "[Behavior][Reset] Clearing command queues.";
    communicator->clearCommandsAndQueue();

    qDebug() << "[Behavior][Reset] Soft reset";
    communicator->sendRealtimeCommand(GRBL_LIVE_SOFT_RESET);
    setTimeout(100, [this]() {
        qWarning() << "[Behavior][Reset] Timeout: no response after reset.";
        if (m_stage == SentReset) {
            qWarning() << "[Behavior][Reset] Timeout: no reset sequence received within 100ms.";

            emit transition(this, new ErrorBehavior("No response from device"));

            return;
        }
    });
    m_stage = SentReset;

    communicator->startQueryingMachineState();

    return Result::Ok;
}

MachineType ResetBehavior::detectMachineType(const QString &data)
{
    // Matches startup messages from all supported GRBL variants, e.g.:
    // Grbl 1.1h ['$' for help]
    // GrblHAL 1.1f ['$' or '' for help]
    // Grbl 1.8 [uCNC v1.8.8 '$' for help]
    // Gcarvin ?? https://github.com/inventables/gCarvin
    static QRegularExpression re("^(GrblHAL|GRBL|Grbl|GCARVIN|uCNC)\\s\\d\\.\\d.", QRegularExpression::CaseInsensitiveOption);
    if (!data.contains(re)) {
        return MachineType::Unknown;
    }

    MachineType type;
    QString typeName;

    if (data.contains("GrblHAL", Qt::CaseInsensitive)) {
        type = MachineType::GrblHAL;  typeName = "GrblHAL";
    } else if (data.contains("GCARVIN", Qt::CaseInsensitive)) {
        type = MachineType::GCarvin;  typeName = "gCarvin";
    } else if (data.contains("uCNC", Qt::CaseInsensitive)) {
        type = MachineType::uCNC;     typeName = "uCNC";
    } else if (data.contains("FluidNC", Qt::CaseInsensitive)) {
        type = MachineType::FluidNC;  typeName = "FluidNC";
    } else {
        type = MachineType::Grbl;     typeName = "Grbl";
    }

    qDebug() << "[Behavior][Reset] Detected startup message from device:" << data;
    qDebug() << "[Behavior][Reset] Device type:" << typeName;
    logSignal(QString("Detected %1 device.").arg(typeName));

    return type;
}
