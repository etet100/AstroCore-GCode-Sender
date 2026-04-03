// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "virtualgrblconnection.h"
#include <QDebug>

VirtualGRBLConnection::VirtualGRBLConnection(QObject *parent)
    : VirtualConnection("GRBL", parent)
{
}

VirtualGRBLConnection::~VirtualGRBLConnection()
{
}

// DLL / QThread mode only

#ifndef VIRTUAL_SIMULATOR_PROCESS

#include <QLibrary>
#ifdef WINDOWS
    #include <windows.h>
    #ifndef _MSC_VER
        // #define STATIC_GRBL
    #endif
#endif
#ifdef LINUX
    #define STATIC_GRBL
#endif

#ifdef STATIC_GRBL
extern "C" {
    Q_DECL_IMPORT void GRBL(QString serverName, QAtomicInt* stopFlag);
}
#else
typedef void (*GRBLFunction)(QString serverName, QAtomicInt* stopFlag);
#endif

QThread* VirtualGRBLConnection::createWorkerThread(const QString& serverName)
{
    return new VirtualGRBLWorkerThread(serverName, &m_stopFlag);
}

VirtualGRBLWorkerThread::VirtualGRBLWorkerThread(QString serverName, QAtomicInt* stopFlag)
    : QThread(nullptr)
    , m_serverName(serverName)
    , m_stopFlag(stopFlag)
{
}

void VirtualGRBLWorkerThread::run()
{
    qInfo() << "[IO][GRBL] Starting virtual GRBL, server" << m_serverName;
#ifdef STATIC_GRBL
    #ifdef WINDOWS
        GRBL(m_serverName.toStdString().c_str(), m_stopFlag);
    #endif
#else
    qDebug() << "[IO][GRBL] Dynamic mode";
    QLibrary lib("grblHal.dll");
    if (!lib.load()) {
        qWarning() << "[IO][GRBL] Library could not be loaded!";

        return;
    }
    GRBLFunction GRBL = (GRBLFunction) lib.resolve("GRBL");
    if (GRBL != nullptr) {
        qDebug() << "[IO][GRBL] Calling GRBL()";
        GRBL(m_serverName.toStdString().c_str(), m_stopFlag);
    } else {
        qWarning() << "[IO][GRBL] GRBL() not found in library!";
    }
    lib.unload();
#endif
    qInfo() << "[IO][GRBL] Stopped.";
    *m_stopFlag = 3;
}

#endif // !VIRTUAL_SIMULATOR_PROCESS
