#include "configuration.h"
#include "registry.h"
#include <QMetaObject>
#include <QMetaProperty>
#include <QDebug>
#include <QCoreApplication>
#include "core/config/persistence/ini/iniprovider.h"
#include "core/config/persistence/ini/inipersister.h"
#include "core/config/persistence/json/jsonprovider.h"
#include "core/config/persistence/json/jsonpersister.h"
#include "core/config/persistence/xml/xmlprovider.h"
#include "core/config/persistence/xml/xmlpersister.h"

Configuration::Configuration()
    : QObject(nullptr),
    m_sender(),
    m_connection(),
    m_visualizer(),
    m_console(),
    m_parser(),
    m_ui(),
    m_machine(),
    m_heightmap(),
    m_jogging(),
    m_ai(),
    m_pendant(),
    m_macros()
{
    m_modules << &m_sender
        << &m_connection
        << &m_visualizer
        << &m_console
        << &m_parser
        << &m_ui
        << &m_machine
        << &m_heightmap
        << &m_jogging
        << &m_ai
        << &m_pendant
        << &m_macros;
}

bool Configuration::init(const QString& appPath, const QString& configType)
{
    QString configFilePath = appPath + "/config.";

    if (configType == "json") {
        m_provider = new JsonProvider(nullptr, configFilePath + "json");
        m_persister = new JsonPersister(nullptr, configFilePath + "json");
    } else if (configType == "ini") {
        m_provider = new IniProvider(nullptr, configFilePath + "ini");
        m_persister = new IniPersister(nullptr, configFilePath + "ini");
    } else if (configType == "xml") {
        m_provider = new XmlProvider(nullptr, configFilePath + "xml");
        m_persister = new XmlPersister(nullptr, configFilePath + "xml");
    } else {
        qCritical() << "[Configuration] Unknown config type:" << configType;

        return false;
    }

    load();

    return true;
}

QString Configuration::language()
{
    return this->m_language;
}

void Configuration::setLanguage(QString language)
{
    this->m_language = language;
}

void Configuration::save()
{
    qDebug() << "[Configuration] Save configurations";

    m_persister->open();
    for (ConfigurationModule* module : std::as_const(m_modules)) {
        saveModule(module);
    }
    m_persister->close();
}

bool Configuration::persistByType(QString module, QString name, QVariant value, QString type)
{
    if (type == "QString") {
        m_persister->setString(module, name, value.toString());
    } else if (type == "int") {
        m_persister->setInt(module, name, value.toInt());
    } else if (type == "bool") {
        m_persister->setBool(module, name, value.toBool());
    } else if (type == "double" || type == "float") {
        m_persister->setDouble(module, name, value.toDouble());
    } else if (type == "QStringList") {
        m_persister->setStringList(module, name, value.toStringList());
    } else if (type == "QVariantMap") {
        m_persister->setVariantMap(module, name, value.toMap());
    } else if (type == "QVariantList") {
        m_persister->setVariantList(module, name, value.toList());
    } else if (type == "QVariant") {
        m_persister->setVariant(module, name, value);
    } else {
        return false;
    }

    return true;
}

void Configuration::saveModule(ConfigurationModule *module)
{
    const QMetaObject *metaObj = module->metaObject();

    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);
        if (QString(prop.name()) == "objectName") continue;

        QString typeName = prop.typeName();

        QVariant value = prop.read(module);
        if (prop.isEnumType()) {
            // Convert enum value to its string representation
            QStringList typeNameElements = typeName.split("::");
            int indexOfEnum = metaObj->indexOfEnumerator(typeNameElements.last().toStdString().c_str());
            if (indexOfEnum > -1) {
                QMetaEnum metaEnum = metaObj->enumerator(indexOfEnum);
                value = QString(metaEnum.valueToKey(value.toInt()));
                typeName = "QString";
            }
        }

        if (!persistByType(module->getSectionName(), prop.name(), value, typeName)) {
            auto registryItem = ConfigurationRegistry::getInfo(typeName);
            switch (registryItem.type) {
                case ConfigurationRegistry::Type::Unknown:
                    break;
                case ConfigurationRegistry::Type::Struct: {
                    QVariant normalized = registryItem.normalizeStruct((const char*)value.constData());
                    persistByType(module->getSectionName(), prop.name(), normalized, "QVariantMap");

                    break;
                }
                case ConfigurationRegistry::Type::StructList: {
                    QVariantList normalized = registryItem.normalizeStructList((const char*)value.constData());
                    persistByType(module->getSectionName(), prop.name(), QVariant::fromValue(normalized), "QVariantList");

                    break;
                }
                case ConfigurationRegistry::Type::Value: {
                    QVariant normalized = registryItem.normalizeValue(value);
                    persistByType(module->getSectionName(), prop.name(), normalized, "QVariant");

                    break;
                }
                case ConfigurationRegistry::Type::Enum:
                    persistByType(module->getSectionName(), prop.name(), prop.read(module), "int");

                    break;
            }

        }
    }
}

void Configuration::setModuleDefaults(ConfigurationModule *module)
{
    QMap<QString, QVariant> defaults = module->getDefaults();

    const QMetaObject *metaObj = module->metaObject();
    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);

        if (QString(prop.name()) == "objectName" || QString(prop.name()) == "DEFAULTS") continue;

        QString type(prop.typeName());
        if (prop.isEnumType()) {
            prop.write(module, defaults[prop.name()].toInt());
        } else if (type == "QString") {
            prop.write(module, defaults[prop.name()].toString());
        } else if (type == "int") {
            prop.write(module, defaults[prop.name()].toInt());
        } else if (type == "bool") {
            prop.write(module, defaults[prop.name()].toBool());
        } else if (type == "double" || type == "float") {
            prop.write(module, defaults[prop.name()].toDouble());
        } else if (type == "QStringList") {
            //m_persister->setStringList(module, name, value.toStringList());
        } else if (type == "QVariantMap") {
            prop.write(module, defaults[prop.name()].toMap());
        } else if (type == "QVariantList") {
            prop.write(module, defaults[prop.name()].toList());
        } else if (type == "QVariant") {
            prop.write(module, defaults[prop.name()]);
        };
    }
}

void Configuration::load()
{
    qInfo() << "Load configurations";

    m_provider->open();
    for (ConfigurationModule* module : std::as_const(m_modules)) {
        loadModule(module);
    }
    m_provider->close();

    qDebug() << "[Configuration] Configurations loaded";
}

void Configuration::setDefaults()
{
    for (ConfigurationModule* module : std::as_const(m_modules)) {
        setModuleDefaults(module);
    }

    emit defaultConfigurationLoaded();
}

void Configuration::loadModule(ConfigurationModule *module)
{
    QMap<QString, QVariant> defaults = module->getDefaults();

    qDebug() << "[Configuration] Loading config module" << module->getSectionName();

    const QMetaObject *metaObj = module->metaObject();

    for (int i = 0; i < metaObj->propertyCount(); ++i) {
        QMetaProperty prop = metaObj->property(i);

        if (QString(prop.name()) == "objectName" || QString(prop.name()) == "DEFAULTS") continue;

        QString type(prop.typeName());
        QString name(prop.name());
        if (type == "QString") {
            prop.write(module, m_provider->getString(module->getSectionName(), name, defaults[prop.name()].toString()));
        } else if (type == "int") {
            prop.write(module, m_provider->getInt(module->getSectionName(), name, defaults[prop.name()].toInt()));
        } else if (type == "bool") {
            prop.write(module, m_provider->getBool(module->getSectionName(), name, defaults[prop.name()].toBool()));
        } else if (type == "double" || type == "float") {
            prop.write(module, m_provider->getDouble(module->getSectionName(), name, defaults[prop.name()].toDouble()));
        } else if (type == "QStringList") {
            prop.write(module, m_provider->getStringList(module->getSectionName(), name, defaults[prop.name()].toStringList()));
        } else if (type == "QVariantList") {
            prop.write(module, m_provider->getVariantList(module->getSectionName(), name, defaults[prop.name()].toList()));
        } else if (prop.isEnumType()) {
            QString value = m_provider->getString(module->getSectionName(), name, defaults[prop.name()].toString());
            QStringList typeNameElements = QString(prop.typeName()).split("::");
            int indexOfEnum = metaObj->indexOfEnumerator(typeNameElements.last().toStdString().c_str());
            if (indexOfEnum == -1) {
                qDebug() << "[Configuration] Enum not found" << prop.typeName() << prop.name() << "; trying to find in registry..";
                auto registryItem = ConfigurationRegistry::getInfo(prop.typeName());
                if (registryItem.type == ConfigurationRegistry::Type::Enum) {
                    prop.write(module, m_provider->getInt(module->getSectionName(), QString(prop.name()), defaults[prop.name()].toInt()));
                } else {
                    qDebug() << "[Configuration] Enum not found in registry" << prop.typeName() << prop.name();
                }

                continue;
            }
            QMetaEnum metaEnum = metaObj->enumerator(indexOfEnum);

            bool ok;
            int enumValue = value.toInt(&ok);
            if (ok) {
                prop.write(module, enumValue);
                continue;
            }

            enumValue = metaEnum.keyToValue(value.toStdString().c_str(), &ok);
            if (!ok) {
                qDebug() << "[Configuration] Enum value not found" << value << prop.name();
                continue;
            }

            prop.write(module, enumValue);
        } else {
            auto registryItem = ConfigurationRegistry::getInfo(prop.typeName());

            switch (registryItem.type) {
                case ConfigurationRegistry::Type::Unknown:
                    qDebug() << "[Configuration] Unknown type" << prop.typeName();
                    break;
                case ConfigurationRegistry::Type::Value: {
                    QVariant denormalized = registryItem.denormalizeValue(
                        m_provider->getVariant(module->getSectionName(), prop.name(), defaults[prop.name()])
                    );

                    prop.write(module, denormalized);

                    break;
                }
                case ConfigurationRegistry::Type::Struct: {
                    QVariant denormalized = registryItem.denormalizeStruct(
                        m_provider->getVariantMap(module->getSectionName(), prop.name(), defaults[prop.name()].toMap())
                    );

                    prop.write(module, denormalized);

                    break;
                }
                case ConfigurationRegistry::Type::StructList: {
                    QVariantList normalizedDefaults;
                    if (defaults.contains(prop.name())) {
                        normalizedDefaults = registryItem.normalizeStructList(
                            (const char*)defaults[prop.name()].constData()
                        );
                    }
                    QVariantList rawList = m_provider->getVariantList(
                        module->getSectionName(), prop.name(), normalizedDefaults
                    );
                    QVariant denormalized = registryItem.denormalizeStructList(rawList);
                    prop.write(module, denormalized);

                    break;
                }
                case ConfigurationRegistry::Type::Enum:
                    prop.write(module, m_provider->getInt(module->getSectionName(), QString(prop.name()), defaults[prop.name()].toInt()));

                    break;
            }
        }
    }
}

