// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#include "virtualconnection.h"
#include <QDebug>
#include <QUuid>

VirtualConnection::VirtualConnection(QString deviceName, QObject *parent)
    : Connection(parent)
    , m_stopFlag(0)
    , m_deviceName(deviceName)
{
    m_socket = nullptr;
    m_controlSocket = nullptr;
    m_server = nullptr;
    m_thread = nullptr;
}

VirtualConnection::~VirtualConnection()
{
    cleanup();
}

void VirtualConnection::startLocalServer()
{
    m_server = new QLocalServer(this);
    connect(m_server, &QLocalServer::newConnection, this, &VirtualConnection::onNewConnection);
    m_server->listen(serverPrefix() + QUuid::createUuid().toString());
}

void VirtualConnection::startWorkerThread()
{
    m_thread = createWorkerThread(m_server->serverName());
    m_thread->start();
}

bool VirtualConnection::open()
{
    if (m_state == ConnectionState::Connecting) {
        return false;
    }
    if (m_state == ConnectionState::Connected) {
        return true;
    }

    setState(ConnectionState::Connecting);

    startLocalServer();
    startWorkerThread();

    return false;
}

void VirtualConnection::flushOutgoingData()
{
    if (!m_socket) {
        qDebug() << "[IO][" + deviceName() + "]" << "No socket connection!";
        return;
    }
    if (m_socket->bytesToWrite()) {
        m_socket->waitForBytesWritten(5);
    }
}

void VirtualConnection::sendByteArray(QByteArray byteArray)
{
    assert(m_socket != nullptr);

    flushOutgoingData();

    m_socket->write(byteArray.data(), 1);
    m_socket->flush();
}

void VirtualConnection::sendLine(QString line)
{
    flushOutgoingData();

    qDebug() << "[IO][" + deviceName() + "]" << ">>" << line;

    std::string str = QString(line + "\n").toStdString();
    m_socket->write(str.c_str(), str.length());

    m_socket->flush();
}

void VirtualConnection::sendControlCommand(QString command)
{
    if (!m_controlSocket || !m_controlSocket->isOpen()) {
        qWarning() << "[IO][" + deviceName() + "]" << "Control socket not available!";
        return;
    }

    qDebug() << "[IO][" + deviceName() + "]" << "Control >>" << command;
    m_controlSocket->flush();
}

QString VirtualConnection::deviceName() const
{
    return m_deviceName;
}

void VirtualConnection::cleanupThread()
{
    if (m_thread == nullptr) {
        return;
    }

    qDebug() << "[IO][" << deviceName() << "]" << "Stopping thread...";

    if (!m_thread->wait(1500)) {
        m_thread->terminate();
    }
    m_thread->deleteLater();
    m_thread = nullptr;
}

void VirtualConnection::close()
{
    cleanup();
}

void VirtualConnection::cleanup()
{
    qDebug() << "[IO][" << deviceName() << "]" << "Closing connection";

    if (m_state == ConnectionState::Disconnected) {
        return;
    }

    setState(ConnectionState::Disconnected);

    m_stopFlag = 2;

    if (m_socket != nullptr) {
        if (m_socket->isOpen()) {
            disconnect(m_socket, &QLocalSocket::disconnected, this, &VirtualConnection::onDisconnected);
            m_socket->abort();
        }
        delete m_socket;
        m_socket = nullptr;
    }
    if (m_controlSocket != nullptr) {
        if (m_controlSocket->isOpen()) {
            disconnect(m_controlSocket, &QLocalSocket::disconnected, this, &VirtualConnection::onControlDisconnected);
            m_controlSocket->abort();
        }
        delete m_controlSocket;
        m_controlSocket = nullptr;
    }
    if (m_server != nullptr && m_server->isListening()) {
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }

    cleanupThread();
}

void VirtualConnection::onNewConnection()
{
    if (m_socket == nullptr) {
        qDebug() << "[IO][" + deviceName() + "]" << "Main connection received.";

        m_socket = m_server->nextPendingConnection();
        connect(m_socket, &QIODevice::readyRead, this, &VirtualConnection::onReadyRead);
        connect(m_socket, &QLocalSocket::disconnected, this, &VirtualConnection::onDisconnected);

        setState(ConnectionState::Connected);
    }
    else if (m_controlSocket == nullptr) {
        qDebug() << "[IO][" + deviceName() + "]" << "Control connection received.";

        m_controlSocket = m_server->nextPendingConnection();
        connect(m_controlSocket, &QLocalSocket::disconnected, this, &VirtualConnection::onControlDisconnected);
    }
    else {
        qWarning() << "[IO][" + deviceName() + "]" << "Too many connections, rejecting.";
        QLocalSocket* extraSocket = m_server->nextPendingConnection();
        extraSocket->abort();
        extraSocket->deleteLater();
    }
}

void VirtualConnection::onDisconnected()
{
    qDebug() << "[IO][" + deviceName() + "]" << "Disconnected.";

    close();
}

void VirtualConnection::onControlDisconnected()
{
    qDebug() << "[IO][" + deviceName() + "]" << "Control connection disconnected.";

    if (m_controlSocket != nullptr) {
        m_controlSocket->deleteLater();
        m_controlSocket = nullptr;
    }
}

void VirtualConnection::onReadyRead()
{
    while (m_socket->bytesAvailable() > 0) {
        m_incoming += m_socket->readAll();
        processIncomingData();
    }
}

void VirtualConnection::processIncomingData()
{
    while (true) {
        if (m_incoming.isEmpty()) {
            return;
        }
        int pos = m_incoming.indexOf("\n");
        if (pos == -1) {
            return;
        }

        QString line = m_incoming.left(pos).trimmed();
        m_incoming.remove(0, pos + 1);

        emit this->lineReceived(line);
    }
}
