#ifndef VIRTUALFLUIDNCCONNECTION_H
#define VIRTUALFLUIDNCCONNECTION_H

#include "virtualconnection.h"

#ifndef VIRTUAL_SIMULATOR_PROCESS
#include <QThread>
class VirtualFluidNCWorkerThread : public QThread
{
    public:
        VirtualFluidNCWorkerThread(QString serverName, QAtomicInt* stopFlag);
        void run() override;
    private:
        QString     m_serverName;
        QAtomicInt* m_stopFlag;
};
#endif

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

#ifndef VIRTUAL_SIMULATOR_PROCESS
    QThread* createWorkerThread(const QString& serverName) override;
#else
    QString simulatorType() const override { return "fluidnc"; }
#endif
};

#endif // VIRTUALFLUIDNCCONNECTION_H
