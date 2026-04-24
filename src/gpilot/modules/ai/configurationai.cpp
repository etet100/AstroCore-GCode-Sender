#include "configurationai.h"
#include "core/config/configuration.h"

const QMap<QString,QVariant> DEFAULTS = {
    {"openAIKey", ""}
};

ConfigurationAI::ConfigurationAI(QObject *parent) : AbstractConfigurationModule(parent, DEFAULTS)
{
}

ConfigurationAI& ConfigurationAI::instance()
{
    static ConfigurationAI inst;
    return inst;
}

void ConfigurationAI::registerWith(Configuration& cfg)
{
    cfg.registerModule(&instance());
}
