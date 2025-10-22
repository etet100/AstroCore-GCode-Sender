#ifndef PHYSICALMACHINECONFIGURATIONPARSER_H
#define PHYSICALMACHINECONFIGURATIONPARSER_H

#include <QMap>

#include "physicalmachineconfiguration.h"
#include "core/config/module/configurationmachine.h"

class PhysicalMachineConfigurationParser
{
    public:
        PhysicalMachineConfigurationParser(ConfigurationMachine &configuration);
        const PhysicalMachineConfiguration parse(QStringList rawData);

    private:
        ConfigurationMachine &m_configuration;
};

#endif // PHYSICALMACHINECONFIGURATIONPARSER_H
