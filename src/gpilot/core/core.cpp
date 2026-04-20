#include "core.h"
#include "core/config/implementations.h"
#include "core/communicator/communicator.h"
#include "io/connection/connectionmanager.h"
#include "io/connection/abstractconnection.h"
#include <QCoreApplication>
#include <QDebug>

Core::Core()
    : m_timeEstimator(m_timer)
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
