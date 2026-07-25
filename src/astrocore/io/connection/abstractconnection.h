// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef ABSTRACTCONNECTION_H
#define ABSTRACTCONNECTION_H

#include "core/globals.h"
#include "core/config/module/configurationconnection.h"

class AbstractConnection : public QObject
{
    Q_OBJECT

    public:
        AbstractConnection(QObject *parent);
        virtual ~AbstractConnection() {}

        // true = waiting for connection, false = already connected or failed to connect
        virtual bool open() = 0;
        virtual void sendChar(QChar);
        virtual void sendChar(char);
        virtual void sendByteArray(QByteArray) = 0;
        virtual void sendLine(QString) = 0;
        virtual void close() = 0;
        virtual ConfigurationConnection::ConnectionMode supportedMode() = 0;
        virtual QString name() = 0;

        ConnectionState state() const { return m_state; }
        bool isConnected() const { return m_state == ConnectionState::Connected; }

    protected:
        ConnectionState m_state = ConnectionState::NoConnectionDevice;
        void setState(ConnectionState state);

    signals:
        void lineReceived(const QString &line);
        void error(const QString &text);
        void stateChanged(ConnectionState state);
};

#endif
