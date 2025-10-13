// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "virtualucncconnection.h"
#include <QDebug>
#include <QLibrary>
#include <QUuid>
#ifdef WINDOWS
Q_OS_WIN
    #include <windows.h>
    #ifndef _MSC_VER
        #define STATIC_UCNC
    #endif
#endif
#ifdef LINUX
    #define STATIC_UCNC
#endif

#ifdef STATIC_UCNC
extern "C" {
    // Q_DECL_IMPORT
    void uCNC(QString serverName);
}
#else
typedef void (*uCNCFunction)(QString serverName);
#endif

VirtualUCNCConnection::VirtualUCNCConnection(QObject *parent) : Connection(parent)
{
    m_socket = nullptr;
    m_server = nullptr;
}

VirtualUCNCConnection::~VirtualUCNCConnection()
{
}

void VirtualUCNCConnection::startLocalServer()
{
    m_server = new QLocalServer(this);
    connect(m_server, &QLocalServer::newConnection, this, &VirtualUCNCConnection::onNewConnection);    
    m_server->listen("gpilotucnc_" + QUuid::createUuid().toString());
}

void VirtualUCNCConnection::startWorkerThread()
{
    m_thread = new VirtualUCNCWorkerThread(m_server->serverName());
    m_thread->start();
}

bool VirtualUCNCConnection::open()
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

void VirtualUCNCConnection::flushOutgoingData()
{
    if (!m_socket) {
        qDebug() << "No socket connection!";
        return;
    }
    if (m_socket->bytesToWrite()) {
        m_socket->waitForBytesWritten(5);
    }
}

void VirtualUCNCConnection::sendByteArray(QByteArray byteArray)
{
    assert(m_socket != nullptr);

    flushOutgoingData();

    #ifdef DEBUG_UCNC_COMMUNICATION
        qDebug() << "uCNC (byte) >> " << byteArray.toHex();
    #endif

    m_socket->write(byteArray.data(), 1);
    m_socket->flush();
}

void VirtualUCNCConnection::sendLine(QString line)
{
    flushOutgoingData();

    #ifdef DEBUG_UCNC_COMMUNICATION
        qDebug() << "uCNC >> " << line;
    #endif

    std::string str = QString(line + "\n").toStdString();
    m_socket->write(str.c_str(), str.length());

    m_socket->flush();
}

void VirtualUCNCConnection::closeConnection()
{
    m_thread->terminate();
    m_thread->wait();
    delete m_thread;
    m_thread = nullptr;

    setState(ConnectionState::Disconnected);
    if (m_socket != nullptr) {
        if (m_socket->isOpen()) {
            disconnect(m_socket, &QLocalSocket::disconnected, this, &VirtualUCNCConnection::onDisconnected);
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
}

void VirtualUCNCConnection::onNewConnection()
{
    if (m_socket != nullptr) {
        qWarning() << "Virtual uCNC connection already exists!";
        return;
    }

    m_socket = m_server->nextPendingConnection();
    connect(m_socket, &QIODevice::readyRead, this, &VirtualUCNCConnection::onReadyRead);
    connect(m_socket, &QLocalSocket::disconnected, this, &VirtualUCNCConnection::onDisconnected);

    setState(ConnectionState::Connected);
}

void VirtualUCNCConnection::onDisconnected()
{
    closeConnection();
}

void VirtualUCNCConnection::onReadyRead()
{
    while (m_socket->bytesAvailable() > 0) {
        m_incoming += m_socket->readAll();
        processIncomingData();
    }
}

void VirtualUCNCConnection::processIncomingData()
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

        #ifdef DEBUG_UCNC_COMMUNICATION
            qDebug() << "uCNC << " << line;
        #endif

        emit this->lineReceived(line);
    }
}

VirtualUCNCWorkerThread::VirtualUCNCWorkerThread(QString serverName) : QThread(nullptr), m_serverName(serverName) {
}

void VirtualUCNCWorkerThread::run() {
    qInfo() << "Starting virtual uCNC, server " << m_serverName;
    #ifdef STATIC_UCNC
        uCNC(m_serverName.toStdString().c_str());
    #else
        qDebug() << "uCNC dynamic mode";
        QLibrary lib("uCNC.dll");
        if (!lib.load()) {
            qWarning() << "uCNC library could not be loaded!";
            return;
        }
        uCNCFunction uCNC = (uCNCFunction) lib.resolve("uCNC");
        if (uCNC != nullptr) {
            qDebug() << "Calling uCNC() function";
            uCNC(m_serverName.toStdString().c_str()); // Wywołanie funkcji z biblioteki
        } else {
            qInfo() << "uCNC not initialized. uCNC() not found!";
        }
        lib.unload();
    #endif
    qInfo() << "uCNC stopped!";
}
