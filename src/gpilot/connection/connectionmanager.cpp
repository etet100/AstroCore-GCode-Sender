// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "connectionmanager.h"
#include "../globals.h"

ConnectionManager::ConnectionManager(QObject *parent, const ConfigurationConnection &configurationConnection)
    : QObject(parent)
    , m_configurationConnection(configurationConnection)
{
}

Connection *ConnectionManager::createConnection(ConfigurationConnection::ConnectionMode mode)
{
    switch (mode) {
        case ConfigurationConnection::ConnectionMode::SERIAL:
            return initializeSerialConnection();
        case ConfigurationConnection::ConnectionMode::VIRTUAL_UCNC:
            return initializeVirtualUcncConnection();
        case ConfigurationConnection::ConnectionMode::VIRTUAL_GRBL:
            return initializeVirtualGrblConnection();
        case ConfigurationConnection::ConnectionMode::RAW_TCP:
            return initializeRawTcpConnection();
        default:
            assert(false);
            break;
    }

    return nullptr;
}

SerialConnection *ConnectionManager::initializeSerialConnection()
{
    SerialConnection* serialConnection = new SerialConnection(this);
    serialConnection->setPortName(m_configurationConnection.serialPort());
    serialConnection->setBaudRate(m_configurationConnection.serialBaud());

    return serialConnection;
}

VirtualUCNCConnection *ConnectionManager::initializeVirtualUcncConnection()
{
    VirtualUCNCConnection* virtualConnection = new VirtualUCNCConnection(this);

    return virtualConnection;
}

VirtualGRBLConnection *ConnectionManager::initializeVirtualGrblConnection()
{
    VirtualGRBLConnection* virtualConnection = new VirtualGRBLConnection(this);

    return virtualConnection;
}

RawTcpConnection *ConnectionManager::initializeRawTcpConnection()
{
    RawTcpConnection* rawTcpConnection = new RawTcpConnection(this);
    rawTcpConnection->setHost(m_configurationConnection.rawTcpHost());
    rawTcpConnection->setPort(m_configurationConnection.rawTcpPort());

    return rawTcpConnection;
}
