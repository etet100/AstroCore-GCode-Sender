// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#include "virtualconnection.h"
#include <QDebug>
#include <QUuid>
#include <QJsonObject>
#include <QJsonDocument>

#ifdef VIRTUAL_SIMULATOR_PROCESS
    #include <QProcess>
    #include <QCoreApplication>
#endif

VirtualConnection::VirtualConnection(QString deviceName, QObject *parent)
    : Connection(parent)
    , m_deviceName(deviceName)
{
#ifndef VIRTUAL_SIMULATOR_PROCESS
    m_stopFlag = WorkerStopFlag::Running;
#endif
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

bool VirtualConnection::open()
{
    m_isCleaningUp = false;

    if (m_state == ConnectionState::Connecting) {
        return false;
    }
    if (m_state == ConnectionState::Connected) {
        return true;
    }

    m_stopFlag = WorkerStopFlag::Running;
    m_incoming.clear();

    setState(ConnectionState::Connecting);

    startLocalServer();

#ifndef VIRTUAL_SIMULATOR_PROCESS
    startWorkerThread();
#else
    startProcess();
#endif

    return false;
}

void VirtualConnection::close()
{
    cleanup();
}

void VirtualConnection::cleanup()
{
    if (m_isCleaningUp) {
        return;
    }

    m_isCleaningUp = true;

    qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Closing connection";

    if (m_state == ConnectionState::Disconnected) {
        return;
    }

    setState(ConnectionState::Disconnected);

    if (m_socket != nullptr) {
        QPointer<QLocalSocket> socket = m_socket;
        m_socket = nullptr;

        disconnect(socket, &QLocalSocket::disconnected, this, &VirtualConnection::onDisconnected);
        disconnect(socket, &QLocalSocket::readyRead, this, &VirtualConnection::onReadyRead);
        if (socket != nullptr) {
            socket->blockSignals(true);
        }
        if (socket != nullptr && socket->isOpen()) {
            socket->abort();
            if (!socket->waitForDisconnected(5000)) {
                qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Failed to disconnect socket cleanly!";
            } else {
                qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Socket disconnected cleanly.";
            }
        }
        if (socket != nullptr) {
            socket->deleteLater();
        }
    }
    if (m_controlSocket != nullptr) {
        QPointer<QLocalSocket> controlSocket = m_controlSocket;
        m_controlSocket = nullptr;

        disconnect(controlSocket, &QLocalSocket::disconnected, this, &VirtualConnection::onControlDisconnected);
        if (controlSocket != nullptr) {
            controlSocket->blockSignals(true);
        }
        if (controlSocket != nullptr && controlSocket->isOpen()) {
            controlSocket->abort();
            if (!controlSocket->waitForDisconnected(5000)) {
                qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Failed to disconnect control socket cleanly!";
            } else {
                qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control socket disconnected cleanly.";
            }
        }
        if (controlSocket != nullptr) {
            controlSocket->deleteLater();
        }
    }
    if (m_server != nullptr && m_server->isListening()) {
        disconnect(m_server, &QLocalServer::newConnection, this, &VirtualConnection::onNewConnection);
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }

#ifndef VIRTUAL_SIMULATOR_PROCESS
    cleanupThread();
#else
    killProcess();
#endif
}

// DLL / QThread mode
#ifndef VIRTUAL_SIMULATOR_PROCESS

void VirtualConnection::startWorkerThread()
{
    m_thread = createWorkerThread(m_server->serverName());
    m_thread->start();
}

void VirtualConnection::cleanupThread()
{
    if (m_thread == nullptr) {
        return;
    }

    qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Stopping thread...";

    m_stopFlag = WorkerStopFlag::StopRequested;

    if (!m_thread->wait(1500)) {
        m_thread->terminate();
    }
    m_thread->deleteLater();
    m_thread = nullptr;
}

#endif // !VIRTUAL_SIMULATOR_PROCESS

// QProcess mode
#ifdef VIRTUAL_SIMULATOR_PROCESS

void VirtualConnection::startProcess()
{
    // The simulator exe lives next to the main application binary.
    QString exePath = QCoreApplication::applicationDirPath() + "/gpilot-simulator.exe";

    qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName))
             << "Launching simulator process:" << exePath
             << "server:" << m_server->serverName()
             << "type:" << simulatorType();

    m_process = new QProcess(this);
    m_process->start(exePath, {m_server->serverName(), simulatorType()});

    if (!m_process->waitForStarted(3000)) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName))
                   << "Failed to start simulator process:" << m_process->errorString();
        delete m_process;
        m_process = nullptr;
        setState(ConnectionState::Disconnected);
    }
}

void VirtualConnection::killProcess()
{
    if (m_process == nullptr) {
        return;
    }

    qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Killing simulator process...";

    m_process->kill();
    m_process->waitForFinished(2000);
    delete m_process;
    m_process = nullptr;
}

#endif // VIRTUAL_SIMULATOR_PROCESS

// Data transfer
void VirtualConnection::flushOutgoingData()
{
    if (m_isCleaningUp || !m_socket) {
        qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName))
                 << "[Barrier] flushOutgoingData skipped."
                 << "cleanup:" << m_isCleaningUp
                 << "socketNull:" << (m_socket == nullptr);

        return;
    }
    if (m_socket->bytesToWrite()) {
        m_socket->waitForBytesWritten(5);
    }
}

void VirtualConnection::sendByteArray(QByteArray byteArray)
{
    if (m_isCleaningUp || m_socket == nullptr) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName))
                   << "[Barrier] sendByteArray ignored."
                   << "cleanup:" << m_isCleaningUp
                   << "socketNull:" << (m_socket == nullptr);

        return;
    }

    flushOutgoingData();

    m_socket->write(byteArray.data(), 1);
    m_socket->flush();
}

void VirtualConnection::sendLine(QString line)
{
    if (m_isCleaningUp || m_socket == nullptr) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName))
                   << "[Barrier] sendLine ignored."
                   << "cleanup:" << m_isCleaningUp
                   << "socketNull:" << (m_socket == nullptr);

        return;
    }

    flushOutgoingData();

    qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << ">>" << line;

    std::string str = QString(line + "\n").toStdString();
    m_socket->write(str.c_str(), str.length());
    m_socket->flush();
}

void VirtualConnection::sendControlCommand(QJsonObject cmd)
{
    QByteArray json = QJsonDocument(cmd).toJson(QJsonDocument::Compact);
    json.append("\n");
    m_controlSocket->write(json);
    m_controlSocket->flush();

    qDebug() << qPrintable(QString("[IO][%1][Ctrl]").arg(m_deviceName))
             << "Control >>" << QString::fromUtf8(json).trimmed();
}

void VirtualConnection::lockProbeAtCurrentPosition()
{
    if (!m_controlSocket || !m_controlSocket->isOpen()) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control socket not available!";

        return;
    }
    qDebug() << qPrintable(QString("[IO][%1][Ctrl]").arg(m_deviceName)) << "Locking probe at current position.";
    sendControlCommand({{"cmd", "probe_at_current"}});
}

void VirtualConnection::resetProbePosition()
{
    if (!m_controlSocket || !m_controlSocket->isOpen()) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control socket not available!";

        return;
    }
    qDebug() << qPrintable(QString("[IO][%1][Ctrl]").arg(m_deviceName)) << "Resetting probe position.";
    sendControlCommand({{"cmd", "reset_probe"}});
}

void VirtualConnection::setHome(bool abs, double x, double y, double z)
{
    if (!m_controlSocket || !m_controlSocket->isOpen()) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control socket not available!";

        return;
    }
    qDebug() << qPrintable(QString("[IO][%1][Ctrl]").arg(m_deviceName))
             << "Setting home position to" << (abs ? "absolute" : "relative")
             << "(" << x << "," << y << "," << z << ")";
    sendControlCommand({{"cmd", "set_home"}, {"abs", abs}, {"x", x}, {"y", y}, {"z", z}});
}

void VirtualConnection::setSingleLimit(Axis axis, float pos)
{
    if (!m_controlSocket || !m_controlSocket->isOpen()) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control socket not available!";

        return;
    }
    qDebug() << qPrintable(QString("[IO][%1][Ctrl]").arg(m_deviceName))
             << "Setting single limit for axis" << (int) axis << "to" << pos;
    sendControlCommand({{"cmd", "set_single_limit"}, {"axis", (int) axis}, {"pos", pos}});
}

void VirtualConnection::estop()
{
    if (!m_controlSocket || !m_controlSocket->isOpen()) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control socket not available!";

        return;
    }
    qDebug() << qPrintable(QString("[IO][%1][Ctrl]").arg(m_deviceName)) << "Emergency stop!";
    sendControlCommand({{"cmd", "estop"}});
}

QString VirtualConnection::deviceName() const
{
    return m_deviceName;
}

// Socket slots
void VirtualConnection::onNewConnection()
{
    if (m_isCleaningUp || m_server == nullptr) {
        qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName))
                 << "[Barrier] onNewConnection ignored."
                 << "cleanup:" << m_isCleaningUp
                 << "serverNull:" << (m_server == nullptr);

        return;
    }

    if (m_socket == nullptr) {
        qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Main connection received.";

        m_socket = m_server->nextPendingConnection();
        m_socket->setParent(this);
        connect(m_socket, &QLocalSocket::readyRead, this, &VirtualConnection::onReadyRead);
        connect(m_socket, &QLocalSocket::disconnected, this, &VirtualConnection::onDisconnected);

        setState(ConnectionState::Connected);
    } else if (m_controlSocket == nullptr) {
        qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Control connection received.";

        m_controlSocket = m_server->nextPendingConnection();
        m_controlSocket->setParent(this);
        connect(m_controlSocket, &QLocalSocket::disconnected, this, &VirtualConnection::onControlDisconnected);
    } else {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Too many connections, rejecting.";
        QLocalSocket* extra = m_server->nextPendingConnection();
        extra->abort();
        extra->deleteLater();
    }
}

void VirtualConnection::onDisconnected()
{
    qDebug() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "Disconnected.";
    cleanup();
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
    if (m_isCleaningUp) {
        return;
    }

    if (m_socket == nullptr) {
        qWarning() << qPrintable(QString("[IO][%1]").arg(m_deviceName)) << "readyRead received with null socket.";

        return;
    }

    while (true) {
        QPointer<QLocalSocket> socket = m_socket;
        if (socket == nullptr || !socket->isOpen()) {
            return;
        }

        QByteArray chunk = socket->readAll();
        if (chunk.isEmpty()) {
            return;
        }

        m_incoming += chunk;
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
