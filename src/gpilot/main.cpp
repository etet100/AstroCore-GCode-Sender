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
}

int main(int argc, char *argv[])
{
#ifdef UNIX
    bool styleOverrided = false;
    for (int i = 0; i < argc; i++) if (QString(argv[i]).toUpper() == "-STYLE") {
        styleOverrided = true;
        break;
    }
#endif
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

    QCommandLineOption configTypeOption(QStringList{"c", "config-type"}, "Set config type (ini, json).", "type", "ini");
    parser.addOption(configTypeOption);

    parser.process(app);

    if (parser.isSet(logToFileOption)) {
        qInstallMessageHandler(messageHandler);
    }

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

#ifdef UNIX
    if (!styleOverrided) {
        foreach (QString str, QStyleFactory::keys()) {
            qDebug() << "style" << str;
            if (str.contains("GTK+")) {
                app.setStyle(QStyleFactory::create(str));
                break;
            }
        }
    }
#endif

    Provider *provider = nullptr;
    Persister *persister = nullptr;
    QString configFilePath = app.applicationDirPath() + "/config.";
    if (parser.value(configTypeOption) == "json") {
        provider = new JsonProvider(nullptr, configFilePath + "json");
        persister = new JsonPersister(nullptr, configFilePath + "json");
    } else {
        provider = new IniProvider(nullptr, configFilePath + "ini");
        persister = new IniPersister(nullptr, configFilePath + "ini");
    }

    Configuration configuration(nullptr, persister, provider);
    configuration.load();

    ThemeManager::instance().initialize(&app, configuration.uiModule().darkTheme());

    FrmMain form(configuration);
    form.show();

    return app.exec();
}
