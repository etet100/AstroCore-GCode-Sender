#include "core.h"
#include "core/config/implementations.h"
#include "core/communicator/communicator.h"
#include "core/state_behavior/action.h"
#include "io/connection/connectionmanager.h"
#include "io/connection/abstractconnection.h"
#include "modules/ai/openaimanager.h"
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

    if (command.startsWith(':')) {
        const QString body = command.mid(1).trimmed();
        const int space = body.indexOf(' ');
        const QString head = (space >= 0 ? body.left(space) : body).toLower();
        const QString args = space >= 0 ? body.mid(space + 1).trimmed() : QString();

        if (head == "ai") {
            if (args.isEmpty()) {
                emit log("[AI] missing prompt");
                return;
            }
            OpenAIManager& ai = OpenAIManager::instance();
            ai.setApiKey(m_configuration.aiModule().openAIKey());
            ai.sendRequest(args,
                [this](const QString& response) { emit log("[AI] " + response); },
                [this](const QString& error) { emit log("[AI][Error] " + error); },
                "gpt-4o");
            return;
        }

        if (!m_communicator) {
            emit log(QString("Cannot execute :%1 — communicator not ready").arg(head));
            return;
        }

        AbstractStateBehavior* sb = m_communicator->sb();
        if (head == "start")      { sb->action(Action::Run);        return; }
        if (head == "pause")      { sb->action(Action::Pause);      return; }
        if (head == "resume")     { sb->action(Action::Resume);     return; }
        if (head == "reset")      { sb->action(Action::Unlock);     return; }
        if (head == "abort")      { sb->action(Action::Abort);      return; }
        if (head == "connect")    { sb->action(Action::Connect);    return; }
        if (head == "disconnect") { sb->action(Action::Disconnect); return; }
        if (head == "open")       { emit openFileRequested();       return; }

        emit log(QString("Unknown internal command: %1").arg(head));

        return;
    }

    const int firstSpace = command.indexOf(' ');
    const QString firstToken = firstSpace >= 0 ? command.left(firstSpace) : command;
    for (const Macro& macro : m_macros) {
        if (macro.enabled && macro.name.compare(firstToken, Qt::CaseInsensitive) == 0) {
            emit log(QString("Macro match: %1 (execution not implemented yet)").arg(macro.name));

            return;
        }
    }

    if (!m_communicator) {
        emit log("Cannot send command — communicator not ready");
        return;
    }
    m_communicator->sendCommand(CommandSource::Console, command, TABLE_INDEX_UI);
}
