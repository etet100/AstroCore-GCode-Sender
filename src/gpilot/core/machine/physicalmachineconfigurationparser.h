#ifndef PHYSICALMACHINECONFIGURATIONPARSER_H
#define PHYSICALMACHINECONFIGURATIONPARSER_H

#include <QMap>

#include "physicalmachineconfiguration.h"
#include "core/config/module/configurationmachine.h"

class PhysicalMachineConfigurationParser
{
    public:
        PhysicalMachineConfigurationParser() = delete;

        static void setConfiguration(ConfigurationMachine *configuration);
        static const PhysicalMachineConfiguration parse(QStringList rawData);

    private:
        static ConfigurationMachine *s_configuration;
};

#endif // PHYSICALMACHINECONFIGURATIONPARSER_H
