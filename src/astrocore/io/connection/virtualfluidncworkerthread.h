// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#ifndef VIRTUALFLUIDNCWORKERTHREAD_H
#define VIRTUALFLUIDNCWORKERTHREAD_H

#include <QThread>
#include <QAtomicInt>

class VirtualFluidNCWorkerThread : public QThread
{
public:
    VirtualFluidNCWorkerThread(QString serverName, QAtomicInt* stopFlag);
    void run() override;

private:
    QString     m_serverName;
    QAtomicInt* m_stopFlag;
};

#endif // VIRTUALFLUIDNCWORKERTHREAD_H
