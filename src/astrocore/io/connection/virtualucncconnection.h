// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef VIRTUALUCNCCONNECTION_H
#define VIRTUALUCNCCONNECTION_H

#include <QObject>
#include "virtualconnection.h"

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
    QString serverPrefix() const override { return "astrocoreucnc_"; }

#ifndef VIRTUAL_SIMULATOR_PROCESS
    QThread* createWorkerThread(const QString& serverName) override;
#else
    QString simulatorType() const override { return "ucnc"; }
#endif
};

#endif // VIRTUALUCNCCONNECTION_H
