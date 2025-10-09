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
#include "globals.h"
#include "frmmain.h"
#include "phantomstyle/src/phantom/phantomstyle.h"
#include "config/implementations.h"

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

void initAppInfo()
{
    QCoreApplication::setApplicationName("G-Pilot");
    QCoreApplication::setOrganizationName("BTS");
}

void loadStyleSheets(QApplication &app, bool dark)
{
    QFile stylesheetFile(":/stylesheets/main.qss");
    if (!stylesheetFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Cannot open stylesheet file");
        app.exit(-1);
    }

    QString stylesheet = stylesheetFile.readAll();
    stylesheetFile.close();
    if (dark) {
        stylesheetFile.setFileName(":/stylesheets/dark.qss");
    } else {
        stylesheetFile.setFileName(":/stylesheets/light.qss");
    }
    if (!stylesheetFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Cannot open stylesheet file");
        app.exit(-1);
    }
    stylesheet += "\n\n" + stylesheetFile.readAll();
    stylesheetFile.close();
    app.setStyleSheet(stylesheet);
}

void setTheme(QApplication &app, bool dark)
{
    if (dark) {
        app.styleHints()->setColorScheme(Qt::ColorScheme::Dark);
    } else {
        app.styleHints()->setColorScheme(Qt::ColorScheme::Light);
    }
    app.setStyle(new PhantomStyle());

    QPalette palette;
    palette.setColor(QPalette::Highlight, QColor(204, 204, 254));
    palette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
    app.setPalette(palette);
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

    QApplication app(argc, argv);
    app.setApplicationDisplayName("G-Pilot");
    app.setOrganizationName("G-Pilot");

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

    setTheme(app, configuration.uiModule().darkTheme());
    loadStyleSheets(app, configuration.uiModule().darkTheme());

    frmMain form(configuration);
    form.show();

    return app.exec();
}
