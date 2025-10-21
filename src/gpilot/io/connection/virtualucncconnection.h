// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef VIRTUALUCNCCONNECTION_H
#define VIRTUALUCNCCONNECTION_H

#include <QObject>
#include "connection.h"
#include <QLocalSocket>
#include <QLocalServer>
#include <QThread>

class VirtualUCNCWorkerThread : public QThread
{
    public:
        VirtualUCNCWorkerThread(QString serverName);

        void run() override;
    private:
        QString m_serverName;
};

class VirtualUCNCConnection : public Connection
{
    Q_OBJECT

public:
    VirtualUCNCConnection(QObject*);
    ~VirtualUCNCConnection();
    bool open() override;
    void sendByteArray(QByteArray) override;
    void sendLine(QString) override;
    void close() override;
    ConfigurationConnection::ConnectionMode supportedMode() override { return ConfigurationConnection::ConnectionMode::VIRTUAL_UCNC; }
    QString name() override { return "Virtual UCNC"; }

private:
    QLocalSocket* m_socket;
    QLocalServer* m_server;
    VirtualUCNCWorkerThread* m_thread;
    QString m_incoming;
    void flushOutgoingData();
    void processIncomingData();
    void startLocalServer();
    void startWorkerThread();

private slots:
    void onNewConnection();
    void onDisconnected();
    void onReadyRead();
};

#endif // VIRTUALUCNCCONNECTION_H
