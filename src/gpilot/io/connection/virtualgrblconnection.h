// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef VIRTUALGRBLCONNECTION_H
#define VIRTUALGRBLCONNECTION_H

#include <QObject>
#include "virtualconnection.h"

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

#ifndef VIRTUAL_SIMULATOR_PROCESS
    QThread* createWorkerThread(const QString& serverName) override;
#else
    QString simulatorType() const override { return "grbl"; }
#endif
};

#endif // VIRTUALGRBLCONNECTION_H
