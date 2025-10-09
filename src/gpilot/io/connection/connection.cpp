#include "connection.h"

Connection::Connection(QObject *parent) : QObject(parent)
{
    setState(ConnectionState::Initialization);
}

void Connection::sendChar(QChar char_)
{
    sendChar(char_.toLatin1());
}

void Connection::sendChar(char char_)
{
    sendByteArray(QByteArray(1, char_));
}

void Connection::setState(ConnectionState state)
{
    if (m_state != state) {
        m_state = state;
        emit stateChanged(state);
    }
}
