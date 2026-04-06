// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PENDANT_H
#define PENDANT_H

#include <QObject>
#include <QTcpServer>
#include "core/communicator/communicator.h"

class Configuration;

class Pendant : public QObject
{
    Q_OBJECT

    public:
        explicit Pendant(
            Configuration &configuration,
            Communicator &communicator,
            QObject *parent = nullptr
        );

        bool isConnectionTimedOut(qint64 timeoutMs = 100) const;

    private:
        QTcpServer *m_server = nullptr;
        QTcpSocket *m_socket = nullptr;
        Configuration &m_configuration;
        Communicator &m_communicator;
        qint64 m_lastMessageTime = 0;
        void initialize();
        void deinitialize();
        void sendState();
        void sendWifiConfig(const QString &ssid, const QString &password);
        void sendStepSizeSelections();
        void sendFeedRateSelections();
        void sendStepSize(float step);
        void sendFeedRate(float feed);
        void sendFeedRateZ(float feed);
        void updateLastMessageTime();

        void handlePingMessage(const uint8_t* data, uint8_t size);
        void handleCmdMessage(const uint8_t* data, uint8_t size);
        void handleJoggingParamMessage(const uint8_t* data, uint8_t size);
        void handleJogMessage(const uint8_t* data, uint8_t size);
};

#endif // PENDANT_H
