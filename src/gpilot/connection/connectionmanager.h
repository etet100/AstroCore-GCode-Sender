// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QObject>
#include "config/module/configurationconnection.h"
#include "serialconnection.h"
#include "rawtcpconnection.h"
#include "virtualucncconnection.h"
#include "virtualgrblconnection.h"
#include "connection.h"

class ConnectionManager : public QObject
{
    Q_OBJECT

    public:
        ConnectionManager(QObject *parent, const ConfigurationConnection &configurationConnection);
        Connection* createConnection(ConfigurationConnection::ConnectionMode mode);

    private:
        SerialConnection* initializeSerialConnection();
        VirtualUCNCConnection* initializeVirtualUcncConnection();
        VirtualGRBLConnection* initializeVirtualGrblConnection();
        RawTcpConnection* initializeRawTcpConnection();
        const ConfigurationConnection& m_configurationConnection;
};

#endif // CONNECTIONMANAGER_H
