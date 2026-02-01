// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef VIRTUALGRBLCONNECTION_H
#define VIRTUALGRBLCONNECTION_H

#include <QObject>
#include "virtualconnection.h"

class VirtualGRBLWorkerThread : public QThread
{
    public:
        VirtualGRBLWorkerThread(QString serverName, QAtomicInt* stopFlag);

        void run() override;
    private:
        QString m_serverName;
        QAtomicInt* m_stopFlag;
};

class VirtualGRBLConnection : public VirtualConnection
{
    Q_OBJECT

public:
    VirtualGRBLConnection(QObject* parent = nullptr);
    ~VirtualGRBLConnection();

    ConfigurationConnection::ConnectionMode supportedMode() override { return ConfigurationConnection::ConnectionMode::VIRTUAL_GRBL; }
    QString name() override { return "Virtual GRBL"; }

protected:
    QString deviceName() const override { return "GRBL"; }
    QString serverPrefix() const override { return "gpilotgrbl_"; }
    QThread* createWorkerThread(const QString& serverName) override;
};

#endif // VIRTUALGRBLCONNECTION_H
