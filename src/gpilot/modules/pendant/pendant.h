// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PENDANT_H
#define PENDANT_H

#include <QObject>
#include <QTcpServer>
#include "communicator.h"

class Pendant : public QObject
{
    Q_OBJECT

    public:
        explicit Pendant(QObject *parent, Communicator &communicator);

    private:
        QTcpServer *m_server;
        QTcpSocket *m_socket;
        Communicator &m_communicator;
        void sendState();
        void sendWifiConfig();
        void sendStepSizeConfig();
        void sendFeedRateConfig();
};

#endif // PENDANT_H
