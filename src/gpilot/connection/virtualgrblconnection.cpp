// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "virtualgrblconnection.h"
#include <QDebug>
#include <QLibrary>
#include <QUuid>
#ifdef WINDOWS
Q_OS_WIN
    #include <windows.h>
    #ifndef _MSC_VER
        #define STATIC_GRBL
    #endif
#endif
#ifdef LINUX
    #define STATIC_GRBL
#endif

#ifdef STATIC_GRBL
extern "C" {
    // Q_DECL_IMPORT
    void GRBL(QString serverName);
}
#else
typedef void (*GRBLFunction)(QString serverName);
#endif

VirtualGRBLConnection::VirtualGRBLConnection(QObject *parent) : Connection(parent)
{
    m_socket = nullptr;
    m_server = nullptr;
}

VirtualGRBLConnection::~VirtualGRBLConnection()
{
}

void VirtualGRBLConnection::startLocalServer()
{
    m_server = new QLocalServer(this);
    connect(m_server, &QLocalServer::newConnection, this, &VirtualGRBLConnection::onNewConnection);
    m_server->listen("gpilotgrbl_" + QUuid::createUuid().toString());
}

void VirtualGRBLConnection::startWorkerThread()
{
    m_thread = new VirtualGRBLWorkerThread(m_server->serverName());
    m_thread->start();
}

bool VirtualGRBLConnection::openConnection()
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

void VirtualGRBLConnection::flushOutgoingData()
{
    if (!m_socket) {
        qDebug() << "No socket connection!";
        return;
    }
    if (m_socket->bytesToWrite()) {
        m_socket->waitForBytesWritten(5);
    }
}

void VirtualGRBLConnection::sendByteArray(QByteArray byteArray)
{
    assert(m_socket != nullptr);

    flushOutgoingData();

    #ifdef DEBUG_GRBL_COMMUNICATION
        qDebug() << "GRBL (byte) >> " << byteArray.toHex();
    #endif

    m_socket->write(byteArray.data(), 1);
    m_socket->flush();
}

void VirtualGRBLConnection::sendLine(QString line)
{
    flushOutgoingData();

    #ifdef DEBUG_GRBL_COMMUNICATION
        qDebug() << "GRBL >> " << line;
    #endif

    std::string str = QString(line + "\n").toStdString();
    m_socket->write(str.c_str(), str.length());

    m_socket->flush();
}

void VirtualGRBLConnection::closeConnection()
{
    m_thread->terminate();
    m_thread->wait();
    delete m_thread;
    m_thread = nullptr;

    setState(ConnectionState::Disconnected);
    if (m_socket != nullptr) {
        if (m_socket->isOpen()) {
            disconnect(m_socket, &QLocalSocket::disconnected, this, &VirtualGRBLConnection::onDisconnected);
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

void VirtualGRBLConnection::onNewConnection()
{
    if (m_socket != nullptr) {
        qWarning() << "Virtual GRBL connection already exists!";
        return;
    }

    m_socket = m_server->nextPendingConnection();
    connect(m_socket, &QIODevice::readyRead, this, &VirtualGRBLConnection::onReadyRead);
    connect(m_socket, &QLocalSocket::disconnected, this, &VirtualGRBLConnection::onDisconnected);

    setState(ConnectionState::Connected);
}

void VirtualGRBLConnection::onDisconnected()
{
    closeConnection();
}

void VirtualGRBLConnection::onReadyRead()
{
    while (m_socket->bytesAvailable() > 0) {
        m_incoming += m_socket->readAll();
        processIncomingData();
    }
}

void VirtualGRBLConnection::processIncomingData()
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

        #ifdef DEBUG_GRBL_COMMUNICATION
            qDebug() << "GRBL << " << line;
        #endif

        emit this->lineReceived(line);
    }
}

VirtualGRBLWorkerThread::VirtualGRBLWorkerThread(QString serverName) : QThread(nullptr), m_serverName(serverName) {
}

void VirtualGRBLWorkerThread::run() {
    qInfo() << "Starting virtual GRBL, server " << m_serverName;
    #ifdef STATIC_GRBL
        GRBL(m_serverName.toStdString().c_str());
    #else
        qDebug() << "GRBL dynamic mode";
        QLibrary lib("GRBL.dll");
        if (!lib.load()) {
            qWarning() << "GRBL library could not be loaded!";
            return;
        }
        GRBLFunction GRBL = (GRBLFunction) lib.resolve("GRBL");
        if (GRBL != nullptr) {
            qDebug() << "Calling GRBL() function";
            GRBL(m_serverName.toStdString().c_str()); // Wywołanie funkcji z biblioteki
        } else {
            qInfo() << "GRBL not initialized. GRBL() not found!";
        }
        lib.unload();
    #endif
    qInfo() << "GRBL stopped!";
}
