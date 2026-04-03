// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef VIRTUALUCNCCONNECTION_H
#define VIRTUALUCNCCONNECTION_H

#include <QObject>
#include "virtualconnection.h"

#ifndef VIRTUAL_SIMULATOR_PROCESS
class VirtualUCNCWorkerThread : public QThread
{
    public:
        VirtualUCNCWorkerThread(QString serverName, QAtomicInt* stopFlag);
        void run() override;
    private:
        QString     m_serverName;
        QAtomicInt* m_stopFlag;
};
#endif

class VirtualUCNCConnection : public VirtualConnection
{
    Q_OBJECT

public:
    VirtualUCNCConnection(QObject* parent = nullptr);
    ~VirtualUCNCConnection();

    ConfigurationConnection::ConnectionMode supportedMode() override { return ConfigurationConnection::ConnectionMode::VIRTUAL_UCNC; }
    QString name() override { return "Virtual UCNC"; }

protected:
    QString deviceName() const override { return "uCNC"; }
    QString serverPrefix() const override { return "gpilotucnc_"; }

#ifndef VIRTUAL_SIMULATOR_PROCESS
    QThread* createWorkerThread(const QString& serverName) override;
#else
    QString simulatorType() const override { return "ucnc"; }
#endif
};

#endif // VIRTUALUCNCCONNECTION_H
