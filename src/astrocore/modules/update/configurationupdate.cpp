#include "configurationupdate.h"
#include "core/config/configuration.h"

const QMap<QString,QVariant> DEFAULTS = {
    {"checkForUpdates", true},
    {"checkIntervalDays", 7}
};

ConfigurationUpdate::ConfigurationUpdate(QObject *parent) : AbstractConfigurationModule(parent, DEFAULTS)
{
}

ConfigurationUpdate& ConfigurationUpdate::instance()
{
    static ConfigurationUpdate inst;

    return inst;
}

void ConfigurationUpdate::registerWith(Configuration& cfg)
{
    cfg.registerModule(&instance());
}
