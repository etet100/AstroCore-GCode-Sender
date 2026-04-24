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
#include "ui/forms/frmlog.h"
#include "ui/utils/thememanager.h"
#include "core/core.h"
#include "ui/config/uiconfigs.h"
#include "modules/ai/configurationai.h"
#include "modules/pendant/configurationpendant.h"
#include "core/heightmap/configurationheightmap.h"
#ifdef WINDOWS
#include <windows.h>
#endif

struct TempLogFormItem {
    QtMsgType type;
    QString msg;
};
QList<TempLogFormItem>* tempLogFormBuffer = nullptr;
FrmLog* logForm = nullptr;

bool logToFile = false;

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

    if (logToFile) {
        QFile outFile("GPilot.log");
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt << Qt::endl;
        QTextStream(stdout) << txt << Qt::endl;
        ts.flush();
    }

    if (logForm != nullptr) {
        logForm->log(type, msg);
    } else if (tempLogFormBuffer != nullptr) {
        tempLogFormBuffer->append({type, msg});
    }
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

    QCommandLineOption logWndOption(QStringList{"lw", "log-wnd"}, "Show log browser window.");
    parser.addOption(logWndOption);

#ifdef WINDOWS
    QCommandLineOption consoleOption(QStringList{"co", "console"}, "Show console window (Windows only).");
    parser.addOption(consoleOption);
#endif

    parser.process(app);

    if (parser.isSet(logWndOption)) {
        // Why we do this? We don't want to create log form before creating main form,
        // so we buffer log messages until log form is created.
        tempLogFormBuffer = new QList<TempLogFormItem>();
        qInstallMessageHandler(messageHandler);
    }

    if (parser.isSet(logToFileOption)) {
        if (parser.isSet(trimLogOption)) {
            QFile::remove("GPilot.log");
        }

        logToFile = true;
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

    Configuration& cfg = Core::instance().configuration();

    // Register non-core modules before init() so they are loaded in the first
    // pass. Lazy registration (after init()) is also allowed — the module is
    // loaded from the config file immediately on registerModule().
    UiConfigs::instance().registerAll(cfg);
    ConfigurationAI::registerWith(cfg);
    ConfigurationPendant::registerWith(cfg);
    ConfigurationHeightmap::registerWith(cfg);

    if (!cfg.init(QCoreApplication::applicationDirPath(), parser.value(configTypeOption))) {
        return -1;
    }

    ThemeManager::instance().initialize(&app, UiConfigs::instance().ui().darkTheme());

    FrmMain form;
    form.show();

    if (tempLogFormBuffer != nullptr) {
        logForm = new FrmLog();
        form.setLogFormWindow(logForm);
        logForm->setModal(false);
        for (const TempLogFormItem& item : *tempLogFormBuffer) {
            logForm->log(item.type, item.msg);
        }
        delete tempLogFormBuffer;
        logForm->show();
    }

    return app.exec();
}
