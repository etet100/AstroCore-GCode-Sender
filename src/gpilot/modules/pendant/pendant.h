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

    private:
        QTcpServer *m_server = nullptr;
        QTcpSocket *m_socket = nullptr;
        Configuration &m_configuration;
        Communicator &m_communicator;
        void sendState();
        void sendWifiConfig();
        void sendStepSizeSelections();
        void sendFeedRateSelections();
};

#endif // PENDANT_H
