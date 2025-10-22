#include "machineconfigurationparser.h"
#include <QRegularExpression>

MachineConfigurationParser::MachineConfigurationParser(ConfigurationMachine &configuration)
    : m_configuration(configuration)
{
}

const MachineConfiguration MachineConfigurationParser::parse(QStringList rawData)
{
    static QRegularExpression gs("^\\$(\\d+)\\=([^;]+)$");

    QMap<int, double> rawMachineConfiguration;

    for (QString &line : rawData) {
        QRegularExpressionMatch match = gs.match(line);
        if (match.hasMatch()) {
            rawMachineConfiguration[match.captured(1).toInt()] = match.captured(2).toDouble();
        }
    }

    MachineConfiguration machineConfiguration(
        rawMachineConfiguration,
        m_configuration
    );

    return machineConfiguration;
}
