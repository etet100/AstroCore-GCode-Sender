// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "virtualucncconnection.h"
#include <QThread>

extern "C" {
Q_DECL_IMPORT
    void uCNC(QString serverName);
}

class WorkerThread : public QThread
{
public:
    WorkerThread(QString serverName) : QThread(nullptr), m_serverName(serverName) {
    }

    void run() override {
        qDebug() << "Starting virtual uCNC, server " << m_serverName;
        uCNC(m_serverName.toStdString().c_str());
    }
private:
    QString m_serverName;
};

VirtualUCNCConnection::VirtualUCNCConnection(QObject *parent) : Connection(parent)
{
    m_connected = false;
    m_socket = nullptr;
    m_server = nullptr;
}

VirtualUCNCConnection::~VirtualUCNCConnection()
{

}

bool VirtualUCNCConnection::openConnection()
{
    if (m_server != nullptr) {
        return m_connected;
    }

    m_server = new QLocalServer(this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    m_server->listen("gpilotucnc");

    WorkerThread* thread = new WorkerThread(m_server->serverName());
    thread->start();

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
    flushOutgoingData();

    m_socket->write(byteArray.data(), 1);

    m_socket->flush();
}

bool VirtualUCNCConnection::isConnected()
{
    return m_connected;
}

void VirtualUCNCConnection::sendLine(QString line)
{
    flushOutgoingData();

    std::string str = QString(line + "\n").toLatin1().toStdString();
    m_socket->write(str.c_str(), str.length());

    m_socket->flush();
}

void VirtualUCNCConnection::closeConnection()
{

}

void VirtualUCNCConnection::onNewConnection()
{
    if (m_socket) {
        qDebug() << "Virtual uCNC connection already exists!";
        return;
    }

    m_socket = m_server->nextPendingConnection();
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

    m_connected = true;

    qDebug() << "Virtual uCNC connection established!";
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

        emit this->lineReceived(line);
    }
}
