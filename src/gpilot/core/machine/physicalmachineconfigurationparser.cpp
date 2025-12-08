#include "physicalmachineconfigurationparser.h"
#include <QRegularExpression>

PhysicalMachineConfigurationParser::PhysicalMachineConfigurationParser(ConfigurationMachine &configuration)
    : m_configuration(configuration)
{
}

const PhysicalMachineConfiguration PhysicalMachineConfigurationParser::parse(QStringList rawData)
{
    static QRegularExpression gs("^\\$(\\d+)\\=([^;]+)$");

    QMap<int, double> rawMachineConfiguration;

    for (QString &line : rawData) {
        QRegularExpressionMatch match = gs.match(line);
        if (match.hasMatch()) {
            rawMachineConfiguration[match.captured(1).toInt()] = match.captured(2).toDouble();
        } else {
            qDebug() << "[PhysicalMachineConfigurationParser] Invalid configuration line:" << line;
        }
    }

    // PhysicalMachineConfiguration machineConfiguration(
    //     rawMachineConfiguration,
    //     m_configuration
    // );

    return PhysicalMachineConfiguration(rawMachineConfiguration, m_configuration);
}
