#ifndef MACHINECONFIGURATIONPARSER_H
#define MACHINECONFIGURATIONPARSER_H

#include <QMap>

#include "machineconfiguration.h"
#include "core/config/module/configurationmachine.h"

class MachineConfigurationParser
{
    public:
        MachineConfigurationParser(ConfigurationMachine &configuration);
        const MachineConfiguration parse(QStringList rawData);

    private:
        ConfigurationMachine &m_configuration;
};

#endif // MACHINECONFIGURATIONPARSER_H
