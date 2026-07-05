#include <QApplication>
#include <QDataStream>
#include <QLocalServer>
#include <QLocalSocket>
#include <QCommandLineParser>
#include "frmlog.h"
#include "logreceiver.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("AstroCore Log Viewer");
    app.setOrganizationName("G-Pilot");

    qDebug("Starting log viewer...");

    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("socket-name", "Name of the socket to listen on");

    QCommandLineOption doNotCloseOption(QStringList{"t", "do-not-close"}, "Hide window instead of closing app");
    parser.addOption(doNotCloseOption);

    parser.process(app);

    QString socketName = "test";
    QStringList args = parser.positionalArguments();
    if (args.length() > 0) {
        socketName = args[0];

        qDebug("Using socket name: %s", qPrintable(socketName));
    }

    FrmLog* logForm = new FrmLog(parser.isSet(doNotCloseOption));
    logForm->show();

    logForm->log(QtMsgType::QtDebugMsg, "[A][B] Test");

    QLocalServer* server = new QLocalServer(&app);
    // Remove any leftover socket file from a previous crashed instance
    QLocalServer::removeServer(socketName);
    if (!server->listen(socketName)) {
        qCritical("Failed to start log server on '%s': %s",
                  qPrintable(socketName),
                  qPrintable(server->errorString()));

        return 1;
    }

    LogReceiver* receiver = new LogReceiver(logForm, &app);

    QObject::connect(server, &QLocalServer::newConnection, &app, [server, receiver]() {
        QLocalSocket* socket = server->nextPendingConnection();
        socket->setParent(receiver);
        receiver->setSocket(socket);
    });

    int result = app.exec();

    qDebug("Log viewer exiting with code %d", result);

    return result;
}

