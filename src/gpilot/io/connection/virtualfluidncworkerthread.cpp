// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#include "virtualfluidncworkerthread.h"
#include "simulatordefs.h"
#include <QDebug>
#include <QLibrary>

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
    Q_DECL_IMPORT void FluidNC(QString serverName, QAtomicInt* stopFlag);
}
#else
typedef void (*FluidNCFunction)(QString serverName, QAtomicInt* stopFlag);
#endif

VirtualFluidNCWorkerThread::VirtualFluidNCWorkerThread(QString serverName, QAtomicInt* stopFlag)
    : QThread(nullptr)
    , m_serverName(serverName)
    , m_stopFlag(stopFlag)
{
}

void VirtualFluidNCWorkerThread::run()
{
    qInfo() << "[IO][FluidNC] Starting virtual FluidNC, server" << m_serverName;
#ifdef STATIC_FLUIDNC
    #ifdef WINDOWS
        FluidNC(m_serverName.toStdString().c_str(), m_stopFlag);
    #endif
#else
    qDebug() << "[IO][FluidNC] Dynamic mode";
    QLibrary lib("FluidNC.dll");
    if (!lib.load()) {
        qWarning() << "[IO][FluidNC] Library could not be loaded!";

        return;
    }
    FluidNCFunction FluidNC = (FluidNCFunction) lib.resolve("FluidNC");
    if (FluidNC != nullptr) {
        qDebug() << "[IO][FluidNC] Calling FluidNC()";
        FluidNC(m_serverName.toStdString().c_str(), m_stopFlag);
    } else {
        qWarning() << "[IO][FluidNC] FluidNC() not found in library!";
    }
    lib.unload();
#endif
    qInfo() << "[IO][FluidNC] FluidNC stopped!";
    *m_stopFlag = Simulator::Stopped;
}
