#include "abstractconnection.h"

AbstractConnection::AbstractConnection(QObject *parent) : QObject(parent)
{
    setState(ConnectionState::Initialization);
}

void AbstractConnection::sendChar(QChar char_)
{
    sendChar(char_.toLatin1());
}

void AbstractConnection::sendChar(char char_)
{
    sendByteArray(QByteArray(1, char_));
}

void AbstractConnection::setState(ConnectionState state)
{
    if (m_state != state) {
        m_state = state;
        emit stateChanged(state);
    }
}
