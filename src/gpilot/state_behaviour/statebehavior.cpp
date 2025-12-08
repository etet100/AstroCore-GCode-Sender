// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "statebehavior.h"
#include "core/communicator/communicator.h"
#include <QRegularExpression>

StateBehavior::StateBehavior(QObject *parent) : QObject(nullptr)
{
}

void StateBehavior::reset()
{
    emit transition(this, new ResetBehavior(this));
}

StateBehavior::Result StateBehavior::onRawResponse(QString response) {
    Q_UNUSED(response);
    // if (dataIsReset(response)) {
    //     qDebug() << "[StateBehavior] Unexpected reset?";

    //     // Dangerous situation, reset detected unexpectedly
    //     // What to do? For now, just transition to ResetBehavior
    //     emit transition(this, new ResetBehavior());

    //     return true;
    // }

    return Result::Unhandled;
}

StateBehavior::Result StateBehavior::onExit(StateBehavior *next)
{
    Q_UNUSED(next);
    stopTimer();
    emit asyncCompleted();

    return Result::Ok;
}

void StateBehavior::stopTimer()
{
    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }
}

StateBehavior::Result StateBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    if (previous) {
        m_previous = previous;
    }

    m_communicator = communicator;

    return Result::Ok;
}

void StateBehavior::waitForStateResponse(StateResponseCallback callback)
{
    m_stateResponseCallbacks.append(callback);
}

void StateBehavior::log(QString message, QStringList context)
{
    if (!context.isEmpty()) {
        message = QString("[%1] %2").arg(context.join("]["), message);
    }

    emit logSignal(message);
}

void StateBehavior::log(QString message, std::initializer_list<QString> context)
{
    QStringList contextList;

    for (const auto& ctx : context) {
        contextList << ctx;
    }

    log(message, contextList);
}

bool StateBehavior::dataIsReset(QString data)
{
    // "GRBL" in either case, optionally followed by a number of non-whitespace characters,
    // followed by a version number in the format x.y.
    // This matches e.g.
    // Grbl 1.1h ['$' for help]
    // GrblHAL 1.1f ['$' or '' for help]
    // Grbl 1.8 [uCNC v1.8.8 '$' for help]
    // Gcarvin ?? https://github.com/inventables/gCarvin
    static QRegularExpression re("^(GrblHAL|GRBL|GCARVIN)\\s\\d\\.\\d.", QRegularExpression::CaseInsensitiveOption);

    return data.contains(re);
}
