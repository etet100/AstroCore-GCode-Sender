// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include <QApplication>
#include <QDebug>
#include <QOpenGLWidget>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QStyleFactory>
#include <QStyleHints>
#include <QFontDatabase>
#include <QCommandLineParser>
#include <QLoggingCategory>
#include "core/globals.h"
#include "ui/forms/frmmain.h"
#include "ui/utils/thememanager.h"
#include "core/config/implementations.h"
#ifdef WINDOWS
#include <windows.h>
#endif

void messageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    QString txt;
    switch (type) {
        case QtDebugMsg:
            txt = QString("Debug: %1").arg(msg);
            break;
        case QtWarningMsg:
            txt = QString("Warning: %1").arg(msg);
            break;
        case QtCriticalMsg:
            txt = QString("Critical: %1").arg(msg);
            break;
        case QtInfoMsg:
            txt = QString("Info: %1").arg(msg);
            break;
        case QtFatalMsg:
            txt = QString("Fatal: %1").arg(msg);
            abort();
    }

    QFile outFile("GPilot.log");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << Qt::endl;
    QTextStream(stdout) << txt << Qt::endl;
    ts.flush();
}

#ifdef WINDOWS
void initConsole()
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
}
#endif

int main(int argc, char *argv[])
{
// #ifdef UNIX
//     bool styleOverrided = false;
//     for (int i = 0; i < argc; i++) if (QString(argv[i]).toUpper() == "-STYLE") {
//         styleOverrided = true;
//         break;
//     }
// #endif
    // It is necessary to share OpenGL contexts between widgets. This way,
    // opengl resources don't need to be recreated when docking/undocking
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication app(argc, argv);
    app.setApplicationDisplayName("G-Pilot");
    app.setOrganizationName("G-Pilot");

    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);

    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption logToFileOption(QStringList{"l", "log-to-file"}, "Log debug info to `GPilot.log`.");
    parser.addOption(logToFileOption);

    QCommandLineOption trimLogOption(QStringList{"t", "trim-log"}, "Trim existing log file on start.");
    parser.addOption(trimLogOption);

    QCommandLineOption configTypeOption(QStringList{"c", "config-type"}, "Set config type (ini, json).", "type", "ini");
    parser.addOption(configTypeOption);

#ifdef WINDOWS
    QCommandLineOption consoleOption(QStringList{"co", "console"}, "Show console window (Windows only).");
    parser.addOption(consoleOption);
#endif

    parser.process(app);

    if (parser.isSet(logToFileOption)) {
        if (parser.isSet(trimLogOption)) {
            QFile::remove("GPilot.log");
        }

        qInstallMessageHandler(messageHandler);
    }

#ifdef WINDOWS
    if (parser.isSet(consoleOption)) {
        initConsole();
    }
#endif

#ifdef GLES
    QFontDatabase::addApplicationFont(":/fonts/Ubuntu-R.ttf");
#endif
    QSettings set(app.applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    QString loc = set.value("language", "en").toString();

    QString translationsFolder = qApp->applicationDirPath() + "/translations/";
    QString translationFileName = translationsFolder + "candle_" + loc + ".qm";

    if (QFile::exists(translationFileName)) {
        QTranslator* translator = new QTranslator();
        if (translator->load(translationFileName)) app.installTranslator(translator); else delete translator;
    }

    QString baseTranslationFileName = translationsFolder + "qt_" + loc + ".qm";

    if (QFile::exists(translationFileName)) {
        QTranslator* baseTranslator = new QTranslator();
        if (baseTranslator->load(baseTranslationFileName)) app.installTranslator(baseTranslator); else delete baseTranslator;
    }

// #ifdef UNIX
//     if (!styleOverrided) {
//         foreach (QString str, QStyleFactory::keys()) {
//             qDebug() << "style" << str;
//             if (str.contains("GTK+")) {
//                 app.setStyle(QStyleFactory::create(str));
//                 break;
//             }
//         }
//     }
// #endif

    Provider *provider = nullptr;
    Persister *persister = nullptr;
    QString configFilePath = app.applicationDirPath() + "/config.";
    if (parser.value(configTypeOption) == "json") {
        provider = new JsonProvider(nullptr, configFilePath + "json");
        persister = new JsonPersister(nullptr, configFilePath + "json");
    } else if (parser.value(configTypeOption) == "ini") {
        provider = new IniProvider(nullptr, configFilePath + "ini");
        persister = new IniPersister(nullptr, configFilePath + "ini");
    } else if (parser.value(configTypeOption) == "xml") {
        provider = new XmlProvider(nullptr, configFilePath + "xml");
        persister = new XmlPersister(nullptr, configFilePath + "xml");
    } else {
        qCritical() << "[Main] Unknown config type specified:" << parser.value(configTypeOption);
        return -1;
    }

    Configuration configuration(nullptr, persister, provider);
    configuration.load();

    ThemeManager::instance().initialize(&app, configuration.uiModule().darkTheme());

    FrmMain form(configuration);
    form.show();

    return app.exec();
}
