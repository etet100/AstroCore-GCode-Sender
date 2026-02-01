#ifndef VIRTUALFLUIDNCCONNECTION_H
#define VIRTUALFLUIDNCCONNECTION_H

#include <QThread>
#include "virtualconnection.h"
#include <QLocalSocket>
#include <QLocalServer>

class VirtualFluidNCWorkerThread : public QThread
{
    public:
        VirtualFluidNCWorkerThread(QString serverName, QAtomicInt* stopFlag);

        void run() override;
    private:
        QString m_serverName;
        QAtomicInt* m_stopFlag;
};

class VirtualFluidNCConnection : public VirtualConnection
{
    Q_OBJECT

public:
    VirtualFluidNCConnection(QObject* parent = nullptr);
    ~VirtualFluidNCConnection();

    ConfigurationConnection::ConnectionMode supportedMode() override { return ConfigurationConnection::ConnectionMode::VIRTUAL_FLUIDNC; }
    QString name() override { return "Virtual FluidNC"; }

protected:
    QString deviceName() const override { return "FluidNC"; }
    QString serverPrefix() const override { return "gpilotfluidnc_"; }
    QThread* createWorkerThread(const QString& serverName) override;
};

#endif // VIRTUALFLUIDNCCONNECTION_H
