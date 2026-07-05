// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef VIRTUALGRBLWORKERTHREAD_H
#define VIRTUALGRBLWORKERTHREAD_H

#include <QThread>
#include <QAtomicInt>

class VirtualGRBLWorkerThread : public QThread
{
public:
    VirtualGRBLWorkerThread(QString serverName, QAtomicInt* stopFlag);
    void run() override;

private:
    QString     m_serverName;
    QAtomicInt* m_stopFlag;
};

#endif // VIRTUALGRBLWORKERTHREAD_H
