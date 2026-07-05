// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "virtualucncworkerthread.h"
#include "simulatordefs.h"
#include <QDebug>
#include <QLibrary>

#ifdef WINDOWS
    #include <windows.h>
    #ifndef _MSC_VER
        // #define STATIC_UCNC
    #endif
#endif
#ifdef LINUX
    #define STATIC_UCNC
#endif

#ifdef STATIC_UCNC
extern "C" {
    void uCNC(QString serverName, QAtomicInt* stopFlag);
}
#else
typedef void (*uCNCFunction)(QString serverName, QAtomicInt* stopFlag);
#endif

VirtualUCNCWorkerThread::VirtualUCNCWorkerThread(QString serverName, QAtomicInt* stopFlag)
    : QThread(nullptr)
    , m_serverName(serverName)
    , m_stopFlag(stopFlag)
{
}

void VirtualUCNCWorkerThread::run()
{
    qInfo() << "[IO][uCNC] Starting virtual uCNC, server" << m_serverName;
#ifdef STATIC_UCNC
    #ifdef WINDOWS
        uCNC(m_serverName.toStdString().c_str(), m_stopFlag);
    #endif
#else
    qDebug() << "[IO][uCNC] Dynamic mode";
    QLibrary lib("uCNC.dll");
    if (!lib.load()) {
        qWarning() << "[IO][uCNC] Library could not be loaded!";

        return;
    }
    uCNCFunction uCNC = (uCNCFunction) lib.resolve("uCNC");
    if (uCNC != nullptr) {
        qDebug() << "[IO][uCNC] Calling uCNC()";
        uCNC(m_serverName.toStdString().c_str(), m_stopFlag);
    } else {
        qWarning() << "[IO][uCNC] uCNC() not found in library!";
    }
    lib.unload();
#endif
    qInfo() << "[IO][uCNC] Stopped.";
    *m_stopFlag = Simulator::Stopped;
}
