// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2024 BTS

#ifndef VIRTUALUCNCWORKERTHREAD_H
#define VIRTUALUCNCWORKERTHREAD_H

#include <QThread>
#include <QAtomicInt>

class VirtualUCNCWorkerThread : public QThread
{
public:
    VirtualUCNCWorkerThread(QString serverName, QAtomicInt* stopFlag);
    void run() override;

private:
    QString     m_serverName;
    QAtomicInt* m_stopFlag;
};

#endif // VIRTUALUCNCWORKERTHREAD_H
