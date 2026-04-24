#include "core.h"
#include "core/config/implementations.h"
#include "core/communicator/communicator.h"
#include "core/communicator/commandscanner.h"
#include "core/state_behavior/action.h"
#include "io/connection/connectionmanager.h"
#include "io/connection/abstractconnection.h"
#include "modules/ai/openaimanager.h"
#include "modules/ai/configurationai.h"
#include "ui/config/uiconfigs.h"
#include <QCoreApplication>
#include <QDebug>

Core::Core()
    : QObject(nullptr),
      m_timeEstimator(m_timer)
{
}

Core::~Core()
{
    // Delete in reverse dependency order. Communicator may hold a pointer
    // to the connection; delete it first to avoid use-after-free during
    // its own shutdown.
    delete m_communicator;
    m_communicator = nullptr;

    delete m_connection;
    m_connection = nullptr;

    delete m_connectionManager;
    m_connectionManager = nullptr;
}

ConnectionManager& Core::connectionManager()
{
    if (!m_connectionManager) {
        m_connectionManager = new ConnectionManager(nullptr, m_configuration.connectionModule());
    }
    return *m_connectionManager;
}

Communicator* Core::createCommunicator(QObject* parent)
{
    if (m_communicator) {
        qWarning() << "[Core] Communicator already created, returning existing";
        return m_communicator;
    }
    m_communicator = new Communicator(parent, nullptr, &m_configuration);
    return m_communicator;
}

void Core::setConnection(AbstractConnection* c)
{
    if (c == m_connection) {
        return;
    }
    delete m_connection;
    m_connection = c;
}

void Core::handleConsoleCommand(QString command)
{
    command = command.trimmed();
    if (command.isEmpty()) {
        return;
    }

    using Handler = ConsoleCommandResult (Core::*)(const QString&);
    const Handler steps[] = {
        &Core::tryHandleInternalCommand,
        &Core::tryHandleMacro,
        &Core::tryHandleScanned,
    };

    for (Handler step : steps) {
        ConsoleCommandResult r = (this->*step)(command);
        if (r.rewritten) {
            command = *r.rewritten;
        }
        if (r.action == ConsoleCommandResult::Action::Stop) {
            return;
        }
    }

    if (!m_communicator) {
        emit log("Cannot send command — communicator not ready");
        return;
    }
    m_communicator->sendCommand(CommandSource::Console, command, TABLE_INDEX_UI);
}

Core::ConsoleCommandResult Core::tryHandleInternalCommand(const QString& command)
{
    if (!command.startsWith(':')) {
        return ConsoleCommandResult::forward();
    }

    const QString body = command.mid(1).trimmed();
    const int space = body.indexOf(' ');
    const QString head = (space >= 0 ? body.left(space) : body).toLower();
    const QString args = space >= 0 ? body.mid(space + 1).trimmed() : QString();

    if (head == "ai") {
        if (args.isEmpty()) {
            emit log("[AI] missing prompt");
            return ConsoleCommandResult::stop();
        }
        OpenAIManager& ai = OpenAIManager::instance();
        ai.setApiKey(ConfigurationAI::instance().openAIKey());
        ai.sendRequest(args,
            [this](const QString& response) { emit log("[AI] " + response); },
            [this](const QString& error) { emit log("[AI][Error] " + error); },
            "gpt-4o");

        return ConsoleCommandResult::stop();
    }

    if (!m_communicator) {
        emit log(QString("Cannot execute :%1 — communicator not ready").arg(head));

        return ConsoleCommandResult::stop();
    }

    AbstractStateBehavior* sb = m_communicator->sb();
    if (head == "start")      { sb->action(Action::Run);        return ConsoleCommandResult::stop(); }
    if (head == "pause")      { sb->action(Action::Pause);      return ConsoleCommandResult::stop(); }
    if (head == "resume")     { sb->action(Action::Resume);     return ConsoleCommandResult::stop(); }
    if (head == "reset")      { sb->action(Action::Unlock);     return ConsoleCommandResult::stop(); }
    if (head == "abort")      { sb->action(Action::Abort);      return ConsoleCommandResult::stop(); }
    if (head == "connect")    { sb->action(Action::Connect);    return ConsoleCommandResult::stop(); }
    if (head == "disconnect") { sb->action(Action::Disconnect); return ConsoleCommandResult::stop(); }
    if (head == "open")       { emit openFileRequested();       return ConsoleCommandResult::stop(); }

    emit log(QString("Unknown internal command: %1").arg(head));

    return ConsoleCommandResult::stop();
}

Core::ConsoleCommandResult Core::tryHandleMacro(const QString& command)
{
    const int firstSpace = command.indexOf(' ');
    const QString firstToken = firstSpace >= 0 ? command.left(firstSpace) : command;
    for (const Macro& macro : m_macros) {
        if (macro.enabled && macro.name.compare(firstToken, Qt::CaseInsensitive) == 0) {
            emit log(QString("Macro match: %1 (execution not implemented yet)").arg(macro.name));

            return ConsoleCommandResult::stop();
        }
    }

    return ConsoleCommandResult::forward();
}

Core::ConsoleCommandResult Core::tryHandleScanned(const QString& command)
{
    switch (CommandScanner::classify(command)) {
        case CommandScanner::CommandType::WorkOffset:
        case CommandScanner::CommandType::Homing:
        case CommandScanner::CommandType::Pause:
        case CommandScanner::CommandType::ToolChange:
        case CommandScanner::CommandType::None:
            break;
    }

    return ConsoleCommandResult::forward();
}

void Core::addRecentFile(QString fileName)
{
    UiConfigs::instance().ui().addRecentFile(fileName);
    m_configuration.save();
    emit recentFilesChanged();
}

void Core::addRecentHeightmap(QString fileName)
{
    UiConfigs::instance().ui().addRecentHeightmap(fileName);
    m_configuration.save();
    emit recentFilesChanged();
}

void Core::clearRecentFiles(bool heightmapMode)
{
    if (heightmapMode) {
        UiConfigs::instance().ui().clearRecentHeightmaps();
    } else {
        UiConfigs::instance().ui().clearRecentFiles();
    }
    m_configuration.save();
    emit recentFilesChanged();
}
