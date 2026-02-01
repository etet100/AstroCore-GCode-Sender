// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#include "virtualfluidncconnection.h"
#include <QDebug>
#include <QLibrary>
#include <QUuid>
#ifdef WINDOWS
#include <windows.h>
#ifndef _MSC_VER
// #define STATIC_FLUIDNC
#endif
#endif
#ifdef LINUX
#define STATIC_FLUIDNC
#endif

#ifdef STATIC_FLUIDNC
    extern "C" {
    Q_DECL_IMPORT void FluidNC(QString serverName);
}
#else
    typedef void (*FluidNCFunction)(QString serverName);
#endif

VirtualFluidNCConnection::VirtualFluidNCConnection(QObject *parent)
    : VirtualConnection("FluidNC", parent)
{
}

VirtualFluidNCConnection::~VirtualFluidNCConnection()
{
}

QThread* VirtualFluidNCConnection::createWorkerThread(const QString& serverName)
{
    return new VirtualFluidNCWorkerThread(serverName, &m_stopFlag);
}

VirtualFluidNCWorkerThread::VirtualFluidNCWorkerThread(QString serverName, QAtomicInt* stopFlag)
    : QThread(nullptr)
    , m_serverName(serverName)
    , m_stopFlag(stopFlag)
{
}

void VirtualFluidNCWorkerThread::run() {
    qInfo() << "[IO][FluidNC] Starting virtual FluidNC, server " << m_serverName;
#ifdef STATIC_FLUIDNC
#ifdef WINDOWS
    FluidNC(m_serverName.toStdString().c_str());
#endif
#else
    qDebug() << "[IO][FluidNC] FluidNC dynamic mode";
    QLibrary lib("FluidNC.dll");
    if (!lib.load()) {
        qWarning() << "[IO][FluidNC] FluidNC library could not be loaded!";
        return;
    }
    FluidNCFunction FluidNC = (FluidNCFunction) lib.resolve("FluidNC");
    if (FluidNC != nullptr) {
        qDebug() << "[IO][FluidNC] Calling FluidNC() function";
        FluidNC(m_serverName.toStdString().c_str());
    } else {
        qInfo() << "[IO][FluidNC] FluidNC not initialized. FluidNC() not found!";
    }
    lib.unload();
#endif
    qInfo() << "[IO][FluidNC] FluidNC stopped!";

    *m_stopFlag = 3;
}
