// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QObject>
#include "core/config/module/configurationconnection.h"
#include "serialconnection.h"
#include "rawtcpconnection.h"
#include "virtualucncconnection.h"
#include "virtualgrblconnection.h"
#include "virtualfluidncconnection.h"
#include "abstractconnection.h"

class ConnectionManager : public QObject
{
    Q_OBJECT

    public:
        ConnectionManager(QObject *parent, const ConfigurationConnection &configurationConnection);
        AbstractConnection* createConnection(ConfigurationConnection::ConnectionMode mode);

    private:
        SerialConnection* initializeSerialConnection();
        VirtualUCNCConnection* initializeVirtualUcncConnection();
        VirtualGRBLConnection* initializeVirtualGrblConnection();
        RawTcpConnection* initializeRawTcpConnection();
        VirtualFluidNCConnection* initializeVirtualFluidNcConnection();
        const ConfigurationConnection& m_configurationConnection;
};

#endif // CONNECTIONMANAGER_H
