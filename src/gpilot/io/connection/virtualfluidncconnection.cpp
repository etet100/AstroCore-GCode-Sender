// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#include "virtualfluidncconnection.h"

VirtualFluidNCConnection::VirtualFluidNCConnection(QObject *parent)
    : VirtualConnection("FluidNC", parent)
{
}

VirtualFluidNCConnection::~VirtualFluidNCConnection()
{
}

#ifndef VIRTUAL_SIMULATOR_PROCESS

#include "virtualfluidncworkerthread.h"

QThread* VirtualFluidNCConnection::createWorkerThread(const QString& serverName)
{
    return new VirtualFluidNCWorkerThread(serverName, &m_stopFlag);
}

#endif // !VIRTUAL_SIMULATOR_PROCESS
