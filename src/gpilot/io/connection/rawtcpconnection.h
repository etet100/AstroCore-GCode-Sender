// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef RAWTCPCONNECTION_H
#define RAWTCPCONNECTION_H

#include <QObject>
#include "connection.h"
#include <QTcpServer>
#include <QTcpSocket>

class RawTcpConnection : public Connection
{
    Q_OBJECT

    public:
        RawTcpConnection(QObject*);
        ~RawTcpConnection();
        bool openConnection() override;
        void setHost(QString);
        void setPort(int);
        void sendByteArray(QByteArray) override;
        void sendLine(QString) override;
        void closeConnection() override;
        ConfigurationConnection::ConnectionMode getSupportedMode() override { return ConfigurationConnection::ConnectionMode::RAW_TCP; };

    private:
        QString m_host;
        int m_port;
        bool m_connected = false;
        QString m_incoming = "";
        QTcpSocket* m_socket = nullptr;
        QTcpServer* m_server = nullptr;
        void flushOutgoingData();
        void processIncomingData();
};

#endif // RAWTCPCONNECTION_H
