#include "configurationai.h"

const QMap<QString,QVariant> DEFAULTS = {
    {"openAIKey", ""}
};

ConfigurationAI::ConfigurationAI(QObject *parent) : AbstractConfigurationModule(parent, DEFAULTS)
{
}
