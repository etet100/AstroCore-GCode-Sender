#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "module/configurationmodule.h"
#include "module/configurationconnection.h"
#include "module/configurationvisualizer.h"
#include "module/configurationsender.h"
#include "module/configurationconsole.h"
#include "module/configurationparser.h"
#include "module/configurationui.h"
#include "module/configurationmachine.h"
#include "module/configurationheightmap.h"
#include "module/configurationjogging.h"
#include "persistence/ini/inipersister.h"
#include "persistence/ini/iniprovider.h"
#include <QObject>

class Configuration : public QObject
{
    Q_OBJECT;

    public:
        Configuration(QObject *parent);
        QString language();
        void setLanguage(QString);
        void save();
        void load();
        void setDefaults();
        ConfigurationConnection& connectionModule() { return m_connection; };
        ConfigurationVisualizer& visualizerModule() { return m_visualizer; };
        ConfigurationSender& senderModule() { return m_sender; };
        ConfigurationConsole& consoleModule() { return m_console; };
        ConfigurationParser& parserModule() { return m_parser; };
        ConfigurationUI& uiModule() { return m_ui; };
        ConfigurationMachine& machineModule() { return m_machine; };
        ConfigurationHeightmap& heightmapModule() { return m_heightmap; };
        ConfigurationJogging& joggingModule() { return m_jogging; };
    private:
        QString m_language;
        QList<ConfigurationModule*> m_modules;

        // Modules
        ConfigurationSender m_sender;
        ConfigurationConnection m_connection;
        ConfigurationVisualizer m_visualizer;
        ConfigurationConsole m_console;
        ConfigurationParser m_parser;
        ConfigurationUI m_ui;
        ConfigurationMachine m_machine;
        ConfigurationHeightmap m_heightmap;
        ConfigurationJogging m_jogging;

        // Read/Write
        IniPersister m_persister;
        IniProvider m_provider;

        void saveModule(ConfigurationModule*);
        void setModuleDefaults(ConfigurationModule*);
        void loadModule(ConfigurationModule*);
        bool persistByType(QString module, QString name, QVariant value, QString type);

    signals:
        void configurationChanged();
        void defaultConfigurationLoaded();
};

#endif // CONFIGURATION_H
