#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "module/abstractconfigurationmodule.h"
#include "module/configurationconnection.h"
#include "module/configurationsender.h"
#include "module/configurationparser.h"
#include "module/configurationmachine.h"
#include "module/configurationjogging.h"
#include "module/configurationmacros.h"
#include "persistence/abstractpersister.h"
#include "persistence/abstractprovider.h"
#include <QObject>

// Configuration is both an owner of CORE modules and a registry for modules
// coming from other layers (UI, optional features). Non-core modules live in
// their own directories and plug themselves in via registerModule() before
// init() is called (eager) or after init() (lazy — the new module is loaded
// from the config file immediately on register).
class Configuration : public QObject
{
    Q_OBJECT;

    public:
        Configuration();
        QString language();
        void setLanguage(QString);

        // Opens the persistence backend and loads every already-registered
        // module. Call AFTER every eager module has registered itself.
        bool init(const QString& appPath, const QString& configType);

        // Adds the module to the registry. If init() already ran, the module
        // is loaded from disk immediately. Returns false on duplicate or when
        // the load step fails.
        bool registerModule(AbstractConfigurationModule* module);

        // Removes the module from the registry. No-op if not present.
        // Does NOT save — call save() before unregistering if you need to.
        void unregisterModule(AbstractConfigurationModule* module);

        void save();
        void load();
        void setDefaults();

        // Core-only accessors. Non-core modules live behind their own owner
        // classes (UiConfigs, ConfigurationAI::instance() etc.).
        ConfigurationConnection& connectionModule() { return m_connection; };
        ConfigurationSender& senderModule() { return m_sender; };
        ConfigurationParser& parserModule() { return m_parser; };
        ConfigurationMachine& machineModule() { return m_machine; };
        ConfigurationJogging& joggingModule() { return m_jogging; };
        ConfigurationMacros& macrosModule() { return m_macros; };

    private:
        QString m_language;
        bool m_initialized = false;
        QList<AbstractConfigurationModule*> m_modules;

        ConfigurationSender m_sender;
        ConfigurationConnection m_connection;
        ConfigurationParser m_parser;
        ConfigurationMachine m_machine;
        ConfigurationJogging m_jogging;
        ConfigurationMacros m_macros;

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
