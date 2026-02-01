// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "virtualucncconnection.h"
#include <QDebug>
#include <QLibrary>
#include <QUuid>
#ifdef WINDOWS
Q_OS_WIN
    #include <windows.h>
    #ifndef _MSC_VER
        #define STATIC_UCNC
    #endif
#endif
#ifdef LINUX
    #define STATIC_UCNC
#endif

#ifdef STATIC_UCNC
extern "C" {
    // Q_DECL_IMPORT
    void uCNC(QString serverName, QAtomicInt* stopFlag);
}
#else
typedef void (*uCNCFunction)(QString serverName, QAtomicInt* stopFlag);
#endif

VirtualUCNCConnection::VirtualUCNCConnection(QObject *parent)
    : VirtualConnection("uCNC", parent)
{
}

VirtualUCNCConnection::~VirtualUCNCConnection()
{
}

QThread* VirtualUCNCConnection::createWorkerThread(const QString& serverName)
{
    return new VirtualUCNCWorkerThread(serverName, &m_stopFlag);
}

VirtualUCNCWorkerThread::VirtualUCNCWorkerThread(QString serverName, QAtomicInt* stopFlag)
    : QThread(nullptr)
    , m_serverName(serverName)
    , m_stopFlag(stopFlag)
{
}

void VirtualUCNCWorkerThread::run() {
    qInfo() << "Starting virtual uCNC, server " << m_serverName;
    #ifdef STATIC_UCNC
        #ifdef WINDOWS
             uCNC(m_serverName.toStdString().c_str(), m_stopFlag);
        #endif
    #else
        qDebug() << "[IO][uCNC] Dynamic mode";
        QLibrary lib("uCNC.dll");
        if (!lib.load()) {
            qWarning() << "[IO][uCNC] uCNC library could not be loaded!";
            return;
        }
        uCNCFunction uCNC = (uCNCFunction) lib.resolve("uCNC");
        if (uCNC != nullptr) {
            qDebug() << "[IO][uCNC] Calling uCNC() function";
            uCNC(m_serverName.toStdString().c_str(), m_stopFlag);
        } else {
            qInfo() << "[IO][uCNC] uCNC not initialized. uCNC() not found!";
        }
        lib.unload();
    #endif
    qInfo() << "uCNC stopped!";

    *m_stopFlag = 3;
}
