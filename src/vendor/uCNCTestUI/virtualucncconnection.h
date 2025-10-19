// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef VIRTUALUCNCCONNECTION_H
#define VIRTUALUCNCCONNECTION_H

#include <QObject>
#include "connection.h"
#include <QLocalSocket>
#include <QLocalServer>

class VirtualUCNCConnection : public Connection
{
    Q_OBJECT

public:
    VirtualUCNCConnection(QObject*);
    ~VirtualUCNCConnection();
    bool openConnection() override;
    void sendByteArray(QByteArray) override;
    bool isConnected() override;
    void sendLine(QString) override;
    void closeConnection() override;
    // ConnectionMode getSupportedMode() override { return ConnectionMode::VIRTUAL; }

private:
    QLocalSocket* m_socket;
    QLocalServer* m_server;
    QString m_incoming;
    bool m_connected;
    void flushOutgoingData();
    void processIncomingData();

private slots:
    void onNewConnection();
    void onReadyRead();

};

#endif // VIRTUALUCNCCONNECTION_H
