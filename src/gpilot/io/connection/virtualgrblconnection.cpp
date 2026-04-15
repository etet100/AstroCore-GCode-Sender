// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "virtualgrblconnection.h"

VirtualGRBLConnection::VirtualGRBLConnection(QObject *parent)
    : VirtualConnection("GRBL", parent)
{
}

VirtualGRBLConnection::~VirtualGRBLConnection()
{
}

#ifndef VIRTUAL_SIMULATOR_PROCESS

#include "virtualgrblworkerthread.h"

QThread* VirtualGRBLConnection::createWorkerThread(const QString& serverName)
{
    return new VirtualGRBLWorkerThread(serverName, &m_stopFlag);
}

#endif // !VIRTUAL_SIMULATOR_PROCESS
