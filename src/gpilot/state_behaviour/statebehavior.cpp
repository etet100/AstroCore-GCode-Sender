// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "statebehavior.h"
#include "core/communicator/communicator.h"
#include <QRegularExpression>
#include "state_behaviour/resetbehavior.h"

const QMap<int, QString> StateBehavior::ERRORS = {
    { GRBL_ERROR_EXPECTED_COMMAND_LETTER,     "Missing letter" },
    { GRBL_ERROR_BAD_NUMBER_FORMAT,           "Bad number" },
    { GRBL_ERROR_INVALID_STATEMENT,           "Invalid line" },
    { GRBL_ERROR_VALUE_LESS_THAN_ZERO,        "Value < 0" },
    { GRBL_ERROR_HOMING_DISABLED,             "Homing off" },
    { GRBL_ERROR_EEPROM_READ_FAIL,            "EEPROM error" },
    { GRBL_ERROR_NOT_IDLE,                    "Machine not idle" },
    { GRBL_ERROR_GCODE_LOCK,                  "G-code locked" },
    { GRBL_ERROR_HOMING_NOT_ENABLED,          "No homing" },
    { GRBL_ERROR_LINE_OVERFLOW,               "Line too long" },
    { GRBL_ERROR_LINE_LENGTH_EXCEEDED,        "Limit exceeded" },
    { GRBL_ERROR_TRAVEL_EXCEEDED,             "Travel limit" },
    { GRBL_ERROR_SETTING_DISABLED,            "Setting off" },
    { GRBL_ERROR_UNSUPPORTED_COMMAND,         "Unsupported cmd" },
    { GRBL_ERROR_MODAL_GROUP_VIOLATION,       "Modal error" },
    { GRBL_ERROR_UNDEFINED_FEED_RATE,         "No feed rate" }
};

#define GRBL_ALARM_HARD_LIMITS          1
#define GRBL_ALARM_SOFT_LIMITS          2
#define GRBL_ALARM_RESET                3
#define GRBL_ALARM_PROBE_FAIL_1         4
#define GRBL_ALARM_PROBE_FAIL_2         5
#define GRBL_ALARM_HOMING_FAIL_1        6
#define GRBL_ALARM_HOMING_FAIL_2        7
#define GRBL_ALARM_HOMING_FAIL_3        8
#define GRBL_ALARM_HOMING_FAIL_4        9

const QMap<int, QString> StateBehavior::ALARMS = {
    { GRBL_ALARM_HARD_LIMITS,      "Hard limits" },
    { GRBL_ALARM_SOFT_LIMITS,      "Soft limits" },
    { GRBL_ALARM_RESET,            "Reset" },
    { GRBL_ALARM_PROBE_FAIL_1,     "Probe fail" },
    { GRBL_ALARM_PROBE_FAIL_2,     "Probe fail" },
    { GRBL_ALARM_HOMING_FAIL_1,    "Homing fail" },
    { GRBL_ALARM_HOMING_FAIL_2,    "Homing fail" },
    { GRBL_ALARM_HOMING_FAIL_3,    "Homing fail" },
    { GRBL_ALARM_HOMING_FAIL_4,    "Homing fail" }
};

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
    m_communicator->stopQueryingMachineState();
    stopTimer();
    stopTimeoutTimer();
    emit asyncCompleted();

    return Result::Ok;
}

void StateBehavior::stopTimer()
{
    if (m_timer) {
        m_timer->stop();
        delete m_timer;
        m_timer = nullptr;
    }
}

void StateBehavior::stopTimeoutTimer()
{
    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
        delete m_timeoutTimer;
        m_timeoutTimer = nullptr;
    }
}

StateBehavior::Result StateBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
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

bool StateBehavior::transitionToPreviousState() {
    if (m_previous) {
        emit transition(this, m_previous);
    }

    return (bool) m_previous;
}

void StateBehavior::setTimeout(int milliseconds, std::function<void ()> callback)
{
    stopTimeoutTimer();

    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    m_timeoutTimer->setInterval(milliseconds);
    if (callback != nullptr) {
        connect(m_timeoutTimer, &QTimer::timeout, this, [this, callback]() {
            stopTimeoutTimer();
            callback();
        });
    } else {
        connect(m_timeoutTimer, &QTimer::timeout, this, &StateBehavior::onTimeoutSlot);
    }
}

QString StateBehavior::enrichErrorMessage(QString message) {
    if (message.startsWith("error:")) {
        int code = message.mid(6).toInt();

        return QString("%1 (error %2)").arg(ERRORS.value(code, "Unknown error")).arg(code);
    }

    return message;
}

void StateBehavior::onTimeoutSlot()
{
    timeout();
}

bool StateBehavior::action(const Action &action)
{
    bool result = doAction(action);
    if (!result) {
        log(QString("[%1] Action rejected: %2").arg(name()).arg(action.name()));
    }

    return false;
}

bool StateBehavior::handleMachineConfigurationActions(const Action &action)
{
    switch (action.type()) {
        case Action::Type::QueryMachineConfiguration:
            m_communicator->queryMachineConfiguration();
            return true;

        case Action::Type::SaveMachineConfigurationParam:
            return handleSaveMachineConfigurationParamAction(action);

        default:
            return false;
    }
}

bool StateBehavior::handleSaveMachineConfigurationParamAction(const Action &action)
{
    SaveMachineConfigurationParamAction saveAction = static_cast<const SaveMachineConfigurationParamAction&>(action);
    QString command = QString("$%1=%2").arg(saveAction.index()).arg(saveAction.value());
    qDebug() << "[" << name() << "] Saving machine configuration parameter:" << command;
    m_communicator->sendCommand(CommandSource::System, command);

    return true;
}
