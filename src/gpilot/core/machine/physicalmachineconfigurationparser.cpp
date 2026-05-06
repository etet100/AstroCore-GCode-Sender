#include "physicalmachineconfigurationparser.h"
#include <QRegularExpression>
#include <QtGlobal>

ConfigurationMachine *PhysicalMachineConfigurationParser::s_configuration = nullptr;

void PhysicalMachineConfigurationParser::setConfiguration(ConfigurationMachine *configuration)
{
    s_configuration = configuration;
}

const PhysicalMachineConfiguration PhysicalMachineConfigurationParser::parse(QStringList rawData)
{
    Q_ASSERT(s_configuration != nullptr);

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

    return PhysicalMachineConfiguration(rawMachineConfiguration, *s_configuration);
}
