// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#include <QApplication>
#include <QCommandLineParser>
#include <QLocalServer>
#include "simulatordefs.h"
#include "virtualgrblworkerthread.h"
#include "virtualfluidncworkerthread.h"
#include "virtualucncworkerthread.h"
#include "frmsimulator.h"

static QThread* createWorkerThread(Simulator::Type type,
                                   const QString& serverName,
                                   QAtomicInt* stopFlag)
{
    switch (type) {
    case Simulator::GRBL:    return new VirtualGRBLWorkerThread(serverName, stopFlag);
    case Simulator::FluidNC: return new VirtualFluidNCWorkerThread(serverName, stopFlag);
    case Simulator::UCNC:    return new VirtualUCNCWorkerThread(serverName, stopFlag);
    }
    return nullptr;
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("GPilot Simulator");
    app.setOrganizationName("G-Pilot");

    QCommandLineParser parser;
    parser.setApplicationDescription("G-Pilot CNC Simulator");
    parser.addHelpOption();

    // Two launch modes:
    // 1. By main app: gpilot-simulator <serverName> <type>
    // 2. Standalone:  gpilot-simulator --type grbl
    QCommandLineOption typeOption("type", "Simulator type (grbl, fluidnc, ucnc)", "type");
    parser.addOption(typeOption);
    parser.addPositionalArgument("server-name", "QLocalServer name", "[server-name]");
    parser.addPositionalArgument("type", "Simulator type", "[type]");

    parser.process(app);

    QString serverName;
    QString typeStr;
    auto posArgs = parser.positionalArguments();

    if (posArgs.size() >= 2) {
        serverName = posArgs[0];
        typeStr = posArgs[1];
    } else if (parser.isSet(typeOption)) {
        typeStr = parser.value(typeOption);
    } else if (posArgs.size() == 1) {
        typeStr = posArgs[0];
    } else {
        qCritical("Usage: gpilot-simulator --type <grbl|fluidnc|ucnc>");
        qCritical("   or: gpilot-simulator <serverName> <type>");
        return 1;
    }

    auto type = Simulator::typeFromString(typeStr);

    // In standalone mode create our own local server
    QLocalServer* ownServer = nullptr;
    if (serverName.isEmpty()) {
        ownServer = new QLocalServer(&app);
        QString name = "gpilot-simulator-" + Simulator::typeToString(type);
        QLocalServer::removeServer(name);
        if (!ownServer->listen(name)) {
            qCritical("Failed to start local server '%s': %s",
                      qPrintable(name),
                      qPrintable(ownServer->errorString()));
            return 1;
        }
        serverName = ownServer->serverName();
        qInfo() << "Standalone mode, server:" << serverName;
    }

    QAtomicInt stopFlag(Simulator::Running);

    FrmSimulator window(type);
    window.show();

    QThread* worker = createWorkerThread(type, serverName, &stopFlag);
    worker->start();

    int result = app.exec();

    // Stop the simulator thread
    stopFlag = Simulator::StopRequested;
    if (!worker->wait(2000)) {
        worker->terminate();
    }
    delete worker;

    return result;
}
