// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#ifndef VIRTUALCONNECTION_H
#define VIRTUALCONNECTION_H

#include <QObject>
#include <QThread>
#include <QLocalSocket>
#include <QLocalServer>
#include "connection.h"

class VirtualConnection : public Connection
{
    Q_OBJECT

public:
    explicit VirtualConnection(QString deviceName, QObject* parent = nullptr);
    virtual ~VirtualConnection();

    bool open() override;
    void sendByteArray(QByteArray byteArray) override;
    void sendLine(QString line) override;
    void close() override;

    // Control commands
    void lockProbeAtCurrentPosition();
    void resetProbePosition();
    void setHome(bool abs, double x, double y, double z);
    void setSingleLimit(Axis axis, float pos);
    void estop();

protected:
    virtual QString deviceName() const;
    virtual QString serverPrefix() const = 0;
    virtual QThread* createWorkerThread(const QString& serverName) = 0;
//    virtual void cleanupThread();

    QLocalSocket* m_socket;
    QLocalSocket* m_controlSocket;
    QLocalServer* m_server;
    QThread* m_thread;
    QString m_incoming;
    QAtomicInt m_stopFlag;
    QString m_deviceName;

    // void sendControlCommand(QString command);
    void sendControlCommand(QJsonObject cmd);

private:
    void flushOutgoingData();
    void processIncomingData();
    void startLocalServer();
    void startWorkerThread();
    void cleanup();
    void cleanupThread();

private slots:
    void onNewConnection();
    void onDisconnected();
    void onControlDisconnected();
    void onReadyRead();
};

#endif // VIRTUALCONNECTION_H
