// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "virtualucncconnection.h"

VirtualUCNCConnection::VirtualUCNCConnection(QObject *parent)
    : VirtualConnection("uCNC", parent)
{
}

VirtualUCNCConnection::~VirtualUCNCConnection()
{
}

#ifndef VIRTUAL_SIMULATOR_PROCESS

#include "virtualucncworkerthread.h"

QThread* VirtualUCNCConnection::createWorkerThread(const QString& serverName)
{
    return new VirtualUCNCWorkerThread(serverName, &m_stopFlag);
}

#endif // !VIRTUAL_SIMULATOR_PROCESS
