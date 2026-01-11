#ifndef VIRTUALFLUIDNCCONNECTION_H
#define VIRTUALFLUIDNCCONNECTION_H

#include <QThread>
#include "connection.h"
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

class VirtualFluidNCConnection : public Connection
{
    public:
        VirtualFluidNCConnection(QObject*);
        ~VirtualFluidNCConnection();
        bool open() override;
        void sendByteArray(QByteArray) override;
        void sendLine(QString) override;
        void close() override;
        ConfigurationConnection::ConnectionMode supportedMode() override { return ConfigurationConnection::ConnectionMode::VIRTUAL_FLUIDNC; }
        QString name() override { return "Virtual FluidNC"; }

    private:
        QLocalSocket* m_socket;
        QLocalServer* m_server;
        QAtomicInt m_stopFlag;
        VirtualFluidNCWorkerThread* m_thread;
        QString m_incoming;
        void flushOutgoingData();
        void processIncomingData();
        void startLocalServer();
        void startWorkerThread();

    private slots:
        void onNewConnection();
        void onDisconnected();
        void onReadyRead();
};

#endif // VIRTUALFLUIDNCCONNECTION_H
