#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "module/abstractconfigurationmodule.h"
#include "module/configurationconnection.h"
#include "module/configurationvisualizer.h"
#include "module/configurationsender.h"
#include "module/configurationconsole.h"
#include "module/configurationparser.h"
#include "module/configurationui.h"
#include "module/configurationmachine.h"
#include "module/configurationheightmap.h"
#include "module/configurationjogging.h"
#include "module/configurationpendant.h"
#include "module/configurationai.h"
#include "module/configurationmacros.h"
#include "persistence/abstractpersister.h"
#include "persistence/abstractprovider.h"
#include <QObject>

class Configuration : public QObject
{
    Q_OBJECT;

    public:
        Configuration();
        QString language();
        void setLanguage(QString);
        // Call this before loading/saving to set the config type (json, ini, xml)
        bool init(const QString& appPath, const QString& configType);
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
        ConfigurationAI& aiModule() { return m_ai; };
        ConfigurationPendant& pendantModule() { return m_pendant; };
        ConfigurationMacros& macrosModule() { return m_macros; };

    private:
        QString m_language;
        QList<AbstractConfigurationModule*> m_modules;

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
        ConfigurationAI m_ai;
        ConfigurationPendant m_pendant;
        ConfigurationMacros m_macros;

        // Read/Write
        AbstractPersister* m_persister = nullptr;
        AbstractProvider* m_provider = nullptr;

        void saveModule(AbstractConfigurationModule*);
        void setModuleDefaults(AbstractConfigurationModule*);
        void loadModule(AbstractConfigurationModule*);
        bool persistByType(QString module, QString name, QVariant value, QString type);

    signals:
        void configurationChanged();
        void defaultConfigurationLoaded();
};

#endif // CONFIGURATION_H
