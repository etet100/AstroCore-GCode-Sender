// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef VIRTUALGRBLCONNECTION_H
#define VIRTUALGRBLCONNECTION_H

#include <QObject>
#include "connection.h"
#include <QLocalSocket>
#include <QLocalServer>
#include <QThread>

class VirtualGRBLWorkerThread : public QThread
{
    public:
        VirtualGRBLWorkerThread(QString serverName);

        void run() override;
    private:
        QString m_serverName;
};

class VirtualGRBLConnection : public Connection
{
    Q_OBJECT

public:
    VirtualGRBLConnection(QObject*);
    ~VirtualGRBLConnection();
    bool openConnection() override;
    void sendByteArray(QByteArray) override;
    void sendLine(QString) override;
    void closeConnection() override;
    ConfigurationConnection::ConnectionMode getSupportedMode() override { return ConfigurationConnection::ConnectionMode::VIRTUAL_GRBL; }

private:
    QLocalSocket* m_socket;
    QLocalServer* m_server;
    VirtualGRBLWorkerThread* m_thread;
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

#endif // VIRTUALGRBLCONNECTION_H
