// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#include "virtualfluidncconnection.h"
#include <QDebug>
#include <QLibrary>
#include <QUuid>
#ifdef WINDOWS
#include <windows.h>
#ifndef _MSC_VER
// #define STATIC_FLUIDNC
#endif
#endif
#ifdef LINUX
#define STATIC_FLUIDNC
#endif

#ifdef STATIC_FLUIDNC
    extern "C" {
    Q_DECL_IMPORT void FluidNC(QString serverName);
}
#else
    typedef void (*FluidNCFunction)(QString serverName);
#endif

VirtualFluidNCConnection::VirtualFluidNCConnection(QObject *parent)
    : Connection(parent)
    , m_stopFlag(0)
{
    m_socket = nullptr;
    m_server = nullptr;
}

VirtualFluidNCConnection::~VirtualFluidNCConnection()
{
    close();
}

void VirtualFluidNCConnection::startLocalServer()
{
    m_server = new QLocalServer(this);
    connect(m_server, &QLocalServer::newConnection, this, &VirtualFluidNCConnection::onNewConnection);
    m_server->listen("gpilotfluidnc_" + QUuid::createUuid().toString());
}

void VirtualFluidNCConnection::startWorkerThread()
{
    m_thread = new VirtualFluidNCWorkerThread(m_server->serverName(), &m_stopFlag);
    m_thread->start();
}

bool VirtualFluidNCConnection::open()
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

    // still waiting for connection, return 'not connected'
    return false;
}

void VirtualFluidNCConnection::flushOutgoingData()
{
    if (!m_socket) {
        qDebug() << "[IO][FluidNC] No socket connection!";
        return;
    }
    if (m_socket->bytesToWrite()) {
        m_socket->waitForBytesWritten(5);
    }
}

void VirtualFluidNCConnection::sendByteArray(QByteArray byteArray)
{
    assert(m_socket != nullptr);

    flushOutgoingData();

#ifdef DEBUG_FluidNC_COMMUNICATION
    qDebug() << "[IO][FluidNC] FluidNC (byte) >> " << byteArray.toHex();
#endif

    m_socket->write(byteArray.data(), 1);
    m_socket->flush();
}

void VirtualFluidNCConnection::sendLine(QString line)
{
    flushOutgoingData();

#ifdef DEBUG_FluidNC_COMMUNICATION
    qDebug() << "[IO][FluidNC] FluidNC >> " << line;
#endif

    std::string str = QString(line + "\n").toStdString();
    m_socket->write(str.c_str(), str.length());

    m_socket->flush();
}

void VirtualFluidNCConnection::close()
{
    qDebug() << "[IO][FluidNC] Closing connection";

    if (m_state == ConnectionState::Disconnected) {
        return;
    }

    setState(ConnectionState::Disconnected);
    m_stopFlag = 2;

    if (m_socket != nullptr) {
        if (m_socket->isOpen()) {
            disconnect(m_socket, &QLocalSocket::disconnected, this, &VirtualFluidNCConnection::onDisconnected);
            m_socket->abort();
        }
        delete m_socket;
        m_socket = nullptr;
    }
    if (m_server != nullptr && m_server->isListening()) {
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }

    if (m_thread != nullptr) {
        qDebug() << "[IO][FluidNC] Stopping FluidNC thread...";
        if (!m_thread->wait(1500)) {
            m_thread->terminate();
        }
        m_thread->deleteLater();
        m_thread = nullptr;
    }
}

void VirtualFluidNCConnection::onNewConnection()
{
    if (m_socket != nullptr) {
        qWarning() << "[IO][FluidNC] Connection already exists!";
        return;
    }

    qDebug() << "[IO][FluidNC] New connection received.";

    m_socket = m_server->nextPendingConnection();
    connect(m_socket, &QIODevice::readyRead, this, &VirtualFluidNCConnection::onReadyRead);
    connect(m_socket, &QLocalSocket::disconnected, this, &VirtualFluidNCConnection::onDisconnected);

    setState(ConnectionState::Connected);
}

void VirtualFluidNCConnection::onDisconnected()
{
    qDebug() << "[IO][FluidNC] Disconnected from FluidNC.";

    close();
}

void VirtualFluidNCConnection::onReadyRead()
{
    while (m_socket->bytesAvailable() > 0) {
        m_incoming += m_socket->readAll();
        processIncomingData();
    }
}

void VirtualFluidNCConnection::processIncomingData()
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

#ifdef DEBUG_FluidNC_COMMUNICATION
        qDebug() << "[IO][FluidNC] FluidNC << " << line;
#endif

        emit this->lineReceived(line);
    }
}

VirtualFluidNCWorkerThread::VirtualFluidNCWorkerThread(QString serverName, QAtomicInt* stopFlag)
    : QThread(nullptr)
    , m_serverName(serverName)
    , m_stopFlag(stopFlag)
{
}

void VirtualFluidNCWorkerThread::run() {
    qInfo() << "[IO][FluidNC] Starting virtual FluidNC, server " << m_serverName;
#ifdef STATIC_FLUIDNC
#ifdef WINDOWS
    FluidNC(m_serverName.toStdString().c_str());
#endif
#else
    qDebug() << "[IO][FluidNC] FluidNC dynamic mode";
    QLibrary lib("FluidNC.dll");
    if (!lib.load()) {
        qWarning() << "[IO][FluidNC] FluidNC library could not be loaded!";
        return;
    }
    FluidNCFunction FluidNC = (FluidNCFunction) lib.resolve("FluidNC");
    if (FluidNC != nullptr) {
        qDebug() << "Calling FluidNC() function";
        FluidNC(m_serverName.toStdString().c_str());
    } else {
        qInfo() << "[IO][FluidNC] FluidNC not initialized. FluidNC() not found!";
    }
    lib.unload();
#endif
    qInfo() << "[IO][FluidNC] FluidNC stopped!";

    *m_stopFlag = 3;
}
