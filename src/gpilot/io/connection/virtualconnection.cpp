// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#include "virtualconnection.h"
#include <QDebug>
#include <QUuid>
#include <QJsonObject>
#include <QJsonDocument>

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
        qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "No socket connection!";

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

    qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << ">>" << line;

    std::string str = QString(line + "\n").toStdString();
    m_socket->write(str.c_str(), str.length());

    m_socket->flush();
}

// void VirtualConnection::sendControlCommand(QString command)
// {
//     if (!m_controlSocket || !m_controlSocket->isOpen()) {
//         qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control socket not available!";
//         return;
//     }

//     qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control >>" << command;
//     m_controlSocket->flush();
// }

void VirtualConnection::sendControlCommand(QJsonObject cmd)
{
    QByteArray json = QJsonDocument(cmd).toJson(QJsonDocument::Compact);
    json.append("\n");
    m_controlSocket->write(json);
    m_controlSocket->flush();

    qDebug() << qPrintable(QString("[IO][%1][Ctrl]").arg(m_deviceName)) << "Control >>" << QString::fromUtf8(json).trimmed();
}

void VirtualConnection::lockProbeAtCurrentPosition()
{
    if (!m_controlSocket || !m_controlSocket->isOpen()) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control socket not available!";

        return;
    }

    qDebug() << qPrintable(QString("[IO][%1][Ctrl]").arg(m_deviceName)) << "Locking probe at current position.";

    QJsonObject cmd;
    cmd["cmd"] = "probe_at_current";

    sendControlCommand(cmd);
}

void VirtualConnection::resetProbePosition()
{
    if (!m_controlSocket || !m_controlSocket->isOpen()) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control socket not available!";

        return;
    }

    qDebug() << qPrintable(QString("[IO][%1][Ctrl]").arg(m_deviceName)) << "Resetting probe position.";

    QJsonObject cmd;
    cmd["cmd"] = "reset_probe";

    sendControlCommand(cmd);
}

void VirtualConnection::setHome(bool abs, double x, double y, double z)
{
    if (!m_controlSocket || !m_controlSocket->isOpen()) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control socket not available!";

        return;
    }

    qDebug() << qPrintable(QString("[IO][%1][Ctrl]").arg(m_deviceName)) << "Setting home position to"
             << (abs ? "absolute" : "relative") << "(" << x << "," << y << "," << z << ")";

    QJsonObject cmd;
    cmd["cmd"] = "set_home";
    cmd["abs"] = abs;
    cmd["x"] = x;
    cmd["y"] = y;
    cmd["z"] = z;

    sendControlCommand(cmd);
}

void VirtualConnection::setSingleLimit(Axis axis, float pos)
{
    if (!m_controlSocket || !m_controlSocket->isOpen()) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control socket not available!";

        return;
    }

    qDebug() << qPrintable(QString("[IO][%1][Ctrl]").arg(m_deviceName)) << "Setting single limit for axis" << (int) axis << "to" << pos;

    QJsonObject cmd;
    cmd["cmd"] = "set_single_limit";
    cmd["axis"] = (int) axis;
    cmd["pos"] = pos;

    sendControlCommand(cmd);
}

void VirtualConnection::estop()
{
    if (!m_controlSocket || !m_controlSocket->isOpen()) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control socket not available!";

        return;
    }

    qDebug() << qPrintable(QString("[IO][%1][Ctrl]").arg(m_deviceName)) << "Emergency stop!";

    QJsonObject cmd;
    cmd["cmd"] = "estop";

    sendControlCommand(cmd);
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

    qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Stopping thread...";

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
    qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Closing connection";

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
        qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Main connection received.";

        m_socket = m_server->nextPendingConnection();
        connect(m_socket, &QIODevice::readyRead, this, &VirtualConnection::onReadyRead);
        connect(m_socket, &QLocalSocket::disconnected, this, &VirtualConnection::onDisconnected);

        setState(ConnectionState::Connected);
    }
    else if (m_controlSocket == nullptr) {
        qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control connection received.";

        m_controlSocket = m_server->nextPendingConnection();
        connect(m_controlSocket, &QLocalSocket::disconnected, this, &VirtualConnection::onControlDisconnected);
    }
    else {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Too many connections, rejecting.";
        QLocalSocket* extraSocket = m_server->nextPendingConnection();
        extraSocket->abort();
        extraSocket->deleteLater();
    }
}

void VirtualConnection::onDisconnected()
{
    qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Disconnected.";

    close();
}

void VirtualConnection::onControlDisconnected()
{
    qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control connection disconnected.";

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
