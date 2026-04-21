// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "abstractstatebehavior.h"
#include "core/communicator/communicator.h"
#include <QRegularExpression>
#include <algorithm>
#include "core/state_behavior/resetbehavior.h"
#include "core/state_behavior/alarmbehavior.h"
#include "core/state_behavior/disconnectingbehavior.h"
#include <QCoroSignal>

const QMap<int, QString> AbstractStateBehavior::ERRORS = {
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

const QMap<int, QString> AbstractStateBehavior::ALARMS = {
    { GRBL_ALARM_HARD_LIMITS,      "Hard limits" },
    { GRBL_ALARM_SOFT_LIMITS,      "Soft limits" },
    { GRBL_ALARM_RESET,            "Reset" },
    { GRBL_ALARM_PROBE_FAIL_1,     "Probe fail" },
    { GRBL_ALARM_PROBE_FAIL_2,     "Probe fail" },
    { GRBL_ALARM_HOMING_FAIL_1,    "Homing fail" },
    { GRBL_ALARM_HOMING_FAIL_2,    "Homing fail" },
    { GRBL_ALARM_HOMING_FAIL_3,    "Homing fail" },
    { GRBL_ALARM_HOMING_FAIL_4,    "Homing fail" },
    { UCNC_ALARM_FAILED_AUTOLEVEL, "Auto-level fail (uCNC)" },
    { UCNC_ALARM_LIMITS_ACTIVE,    "Limits active (uCNC)" },
    { UCNC_ALARM_TOOL_SYNC_FAIL,   "Tool sync fail (uCNC)" },
    { UCNC_ALARM_LIMITS_TRIPPED,   "Limits tripped (uCNC)" }
};

AbstractStateBehavior::AbstractStateBehavior(QObject *parent) : QObject(nullptr)
{
}

void AbstractStateBehavior::reset()
{
    emit transition(this, new ResetBehavior());
}

void AbstractStateBehavior::disconnectAction()
{
    emit transition(this, new DisconnectingBehavior());
}

void AbstractStateBehavior::onMachineState(MachineState state) {
    QList<StateResponseEntry> toFire;

    m_stateResponseCallbacks.removeIf([&](const StateResponseEntry& entry) {
        if (entry.targetState == MachineState::Unknown || entry.targetState == state) {
            toFire.append(entry);

            return true;
        }

        return false;
    });

    for (const auto& entry : toFire) {
        if (entry.timerId != -1) {
            clearTimeout(entry.timerId);
        }
        entry.callback(state);
    }

    emit machineStateSignal(state);
    doOnMachineState(state);
}

AbstractStateBehavior::Result AbstractStateBehavior::onRawResponse(QString response) {
    Q_UNUSED(response);
    // if (dataIsStartupMessage(response)) {
    //     qDebug() << "[Behavior] Unexpected reset?";

    //     // Dangerous situation, reset detected unexpectedly
    //     // What to do? For now, just transition to ResetBehavior
    //     emit transition(this, new ResetBehavior());

    //     return true;
    // }

    return Result::Unhandled;
}

AbstractStateBehavior::Result AbstractStateBehavior::onExit(AbstractStateBehavior *next)
{
    m_communicator->stopQueryingMachineState();
    stopTimer();
    clearAllTimeouts();
    m_stateResponseCallbacks.clear();
    emit asyncCompleted();

    return doOnExit(next);
}

void AbstractStateBehavior::stopTimer()
{
    if (m_timer) {
        m_timer->stop();
        delete m_timer;
        m_timer = nullptr;
    }
}

void AbstractStateBehavior::clearTimeout(int id)
{
    QTimer* timer = m_timers.take(id);
    if (timer) {
        timer->stop();
        delete timer;
    }
}

void AbstractStateBehavior::clearAllTimeouts()
{
    for (QTimer* timer : m_timers) {
        timer->stop();
        delete timer;
    }
    m_timers.clear();
}

AbstractStateBehavior::Result AbstractStateBehavior::onEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    m_communicator = communicator;
    m_exitData.clear();
    m_previousType = ctx.previousType;

    return doOnEntry(communicator, ctx);
}

int AbstractStateBehavior::waitForStateResponse(StateResponseCallback callback, MachineState targetState, int milliseconds)
{
    static int nextId = 0;
    int id = ++nextId;
    int timerId = -1;

    if (milliseconds > 0) {
        timerId = setTimeout(milliseconds, [this, id]() {
            auto it = std::find_if(m_stateResponseCallbacks.begin(), m_stateResponseCallbacks.end(),
                                   [id](const StateResponseEntry& e) { return e.id == id; });
            if (it != m_stateResponseCallbacks.end()) {
                StateResponseCallback cb = it->callback;
                m_stateResponseCallbacks.erase(it);
                cb(MachineState::Unknown);
            }
        });
    }

    m_stateResponseCallbacks.append({id, targetState, timerId, callback});

    return id;
}

void AbstractStateBehavior::clearWaitForStateResponse(int id)
{
    auto it = std::find_if(m_stateResponseCallbacks.begin(), m_stateResponseCallbacks.end(),
                           [id](const StateResponseEntry& e) { return e.id == id; });
    if (it != m_stateResponseCallbacks.end()) {
        if (it->timerId != -1) {
            clearTimeout(it->timerId);
        }
        m_stateResponseCallbacks.erase(it);
    }
}

void AbstractStateBehavior::log(QString message, QStringList context)
{
    if (!context.isEmpty()) {
        message = QString("[%1] %2").arg(context.join("]["), message);
    }

    emit logSignal(message);
}

void AbstractStateBehavior::log(QString message, std::initializer_list<QString> context)
{
    QStringList contextList;

    for (const auto& ctx : context) {
        contextList << ctx;
    }

    log(message, contextList);
}

// bool AbstractStateBehavior::dataIsStartupMessage(QString data)
// {
//     // "GRBL" in either case, optionally followed by a number of non-whitespace characters,
//     // followed by a version number in the format x.y.
//     // This matches e.g.
//     // Grbl 1.1h ['$' for help]
//     // GrblHAL 1.1f ['$' or '' for help]
//     // Grbl 1.8 [uCNC v1.8.8 '$' for help]
//     // Gcarvin ?? https://github.com/inventables/gCarvin
//     static QRegularExpression re("^(GrblHAL|GRBL|GCARVIN)\\s\\d\\.\\d.", QRegularExpression::CaseInsensitiveOption);

//     return data.contains(re);
// }

void AbstractStateBehavior::onAlarm(int code)
{
    qDebug() << QString("[%1] Alarm: %2").arg(name()).arg(ALARMS.value(code, QString("Unknown (%1)").arg(code)));

    m_alarmOccurred = true;
    m_alarmCode = code;

    // CommandResult alarmResult;
    // alarmResult.command = QString("ALARM:%1").arg(code);
    // alarmResult.status.ok = false;
    // alarmResult.status.errorCode = code;

    emit transition(this, new AlarmBehavior(code));
}

void AbstractStateBehavior::onMachineStateChanged(MachineState state)
{
    emit machineStateChangedSignal(state);
}

AbstractStateBehavior::Result AbstractStateBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes,
                                                        CmdStatus cmdStatus, QString response,
                                                        QStringList fullResponse)
{
    emit commandResponseReceived({command, commandAttributes, cmdStatus, response, fullResponse});

    return Result::Ok;
}

QCoro::Task<std::optional<AbstractStateBehavior::CommandResult>> AbstractStateBehavior::awaitResponse(
    int commandIndex, std::chrono::milliseconds timeout)
{
    using namespace std::chrono;
    auto deadline = steady_clock::now() + timeout;

    while (true) {
        auto remaining = duration_cast<milliseconds>(deadline - steady_clock::now());
        if (remaining <= milliseconds::zero()) {
            co_return std::nullopt;
        }

        auto result = co_await qCoro(this, &AbstractStateBehavior::commandResponseReceived, remaining);
        if (!result) {
            co_return std::nullopt;
        }

        if (m_alarmOccurred) {
            co_return *result;
        }

        if (result->attributes.commandIndex == commandIndex) {
            co_return *result;
        }

        qDebug() << QString("[%1] Skipping response for command index %2 (waiting for %3)")
                        .arg(name()).arg(result->attributes.commandIndex).arg(commandIndex);
    }
}

QCoro::Task<std::optional<MachineState>> AbstractStateBehavior::awaitMachineState(
    std::function<bool(MachineState)> predicate,
    std::chrono::milliseconds timeout)
{
    using namespace std::chrono;
    auto deadline = steady_clock::now() + timeout;

    while (true) {
        if (m_alarmOccurred) {
            co_return std::nullopt;
        }

        auto remaining = duration_cast<milliseconds>(deadline - steady_clock::now());
        if (remaining <= milliseconds::zero()) {
            co_return std::nullopt;
        }

        auto result = co_await qCoro(this, &AbstractStateBehavior::machineStateSignal, remaining);
        if (!result) {
            co_return std::nullopt;
        }

        if (m_alarmOccurred) {
            co_return std::nullopt;
        }

        if (predicate(*result)) {
            co_return *result;
        }
    }
}

QCoro::Task<std::optional<AbstractStateBehavior::CommandResult>> AbstractStateBehavior::sendAndAwait(
    const QString &command, std::chrono::milliseconds timeout)
{
    auto r = m_communicator->sendCommand(CommandSource::AbstractStateBehavior, command, TABLE_INDEX_UI);

    co_return co_await awaitResponse(r.commandIndex, timeout);
}

int AbstractStateBehavior::setTimeout(int milliseconds, std::function<void ()> callback)
{
    static int nextId = 0;
    int id = ++nextId;

    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(milliseconds);
    connect(timer, &QTimer::timeout, this, [this, id, callback]() {
        m_timers.remove(id);
        if (callback) {
            callback();
        }
    });
    m_timers.insert(id, timer);
    timer->start();

    return id;
}

QString AbstractStateBehavior::enrichErrorMessage(QString message)
{
    if (message.startsWith("error:")) {
        int code = message.mid(6).toInt();

        return QString("%1 (error %2)").arg(ERRORS.value(code, "Unknown error")).arg(code);
    }

    return message;
}

bool AbstractStateBehavior::action(const Action &action)
{
    if (canExecute(action.type())) {
        switch (action.type()) {
            case Action::Type::Reset:
                this->reset();

                return true;

            case Action::Type::Disconnect:
                this->disconnectAction();

                return true;
        }
    }

    bool result = doAction(action);
    if (!result) {
        qDebug() << qPrintable(QString("[Behavior][%1] Action rejected: %2").arg(name()).arg(action.name()));

        log(QString("[Behavior][%1] Action rejected: %2").arg(name()).arg(action.name()));
    }

    return false;
}

bool AbstractStateBehavior::handleMachineConfigurationActions(const Action &action)
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

bool AbstractStateBehavior::handleSaveMachineConfigurationParamAction(const Action &action)
{
    SaveMachineConfigurationParamAction saveAction = static_cast<const SaveMachineConfigurationParamAction&>(action);

    QString command = QString("$%1=%2").arg(saveAction.index()).arg(saveAction.value());

    qDebug() << qPrintable(QString("[Behavior][%1] Saving machine configuration parameter: %2").arg(name()).arg(command));

    m_communicator->sendCommand(CommandSource::System, command);

    return true;
}
