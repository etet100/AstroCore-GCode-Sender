// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#ifndef VIRTUALCONNECTION_H
#define VIRTUALCONNECTION_H

#include <QObject>
#include <QLocalSocket>
#include <QLocalServer>
#include <QAtomicInt>
#include <QThread>
#include <QPointer>
#ifdef VIRTUAL_SIMULATOR_PROCESS
    #include <QProcess>
#endif
#include "abstractconnection.h"

// Base class for virtual (emulated) machine connections.
//
// Compile-time mode selection via #define in the .pro / CMakeLists:
//
//   Default (no define):
//     Spawns a QThread, loads a .dll via QLibrary and calls the simulator
//     function directly in that thread.
//
//   VIRTUAL_SIMULATOR_PROCESS:
//     Launches gpilot-simulator.exe via QProcess.
//     The exe receives two command-line arguments:
//       1. serverName  — the QLocalServer name to connect to
//       2. simulatorType — "grbl" | "fluidnc" | "ucnc"
//     The exe connects back using two QLocalSocket connections
//     (data + control), exactly like the DLL mode.
class VirtualConnection : public AbstractConnection
{
    Q_OBJECT

public:
    explicit VirtualConnection(QString deviceName, QObject* parent = nullptr);
    virtual ~VirtualConnection();

    bool open() override;
    void sendByteArray(QByteArray byteArray) override;
    void sendLine(QString line) override;
    void close() override;

    // Control commands sent over the secondary (control) socket.
    void lockProbeAtCurrentPosition();
    void resetProbePosition();
    void setHome(bool abs, double x, double y, double z);
    void setSingleLimit(Axis axis, float pos);
    void estop();

protected:
    virtual QString deviceName() const;
    virtual QString serverPrefix() const = 0;

#ifndef VIRTUAL_SIMULATOR_PROCESS
    virtual QThread* createWorkerThread(const QString& serverName) = 0;
    QAtomicInt m_stopFlag;
    QThread*   m_thread = nullptr;
#else
    // Subclass returns the simulator type string for the exe argument.
    virtual QString simulatorType() const = 0;
    QProcess* m_process = nullptr;
#endif

    QPointer<QLocalSocket> m_socket;
    QPointer<QLocalSocket> m_controlSocket;
    QPointer<QLocalServer> m_server;
    QString       m_incoming;
    QString       m_deviceName;
    bool          m_isCleaningUp = false;

    void sendControlCommand(QJsonObject cmd);

private:
    void flushOutgoingData();
    void processIncomingData();
    void startLocalServer();
    void cleanup();

#ifndef VIRTUAL_SIMULATOR_PROCESS
    void startWorkerThread();
    void cleanupThread();
#else
    void startProcess();
    void killProcess();
#endif

private slots:
    void onNewConnection();
    void onDisconnected();
    void onControlDisconnected();
    void onReadyRead();
};

#endif // VIRTUALCONNECTION_H
