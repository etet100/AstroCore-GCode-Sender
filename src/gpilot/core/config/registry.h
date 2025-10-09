// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIG_REGISTRY_H
#define CONFIG_REGISTRY_H

#include <QDebug>
#include <QVector>

class ConfigurationRegistry
{
    public:
        enum class Type {
            Unknown,
            Struct,
            Enum,
            Value
        };

    private:
        typedef std::function<QVariantMap(const char*)> NormalizeStructFunc;
        typedef std::function<QVariant(QVariantMap)> DenormalizeStructFunc;
        typedef std::function<QVariant(QVariant)> NormalizeValueFunc;
        typedef std::function<QVariant(QVariant)> DenormalizeValueFunc;

        struct StructInfo {
            Type type;
            QString name;
            NormalizeStructFunc normalizeStruct = nullptr;
            DenormalizeStructFunc denormalizeStruct = nullptr;
            NormalizeValueFunc normalizeValue = nullptr;
            DenormalizeValueFunc denormalizeValue = nullptr;
        };

        typedef QMap<QString, StructInfo> ConfigRegistryItem;

        static ConfigRegistryItem& getRegistry() {
            static ConfigRegistryItem registry;

            return registry;
        }

    public:
        static void registerStruct(
            const QString& structName,
            const NormalizeStructFunc& normalizeFunc,
            const DenormalizeStructFunc& denormalizeFunc
        ) {
            getRegistry()[structName] = {
                .type=Type::Struct,
                .name=structName,
                .normalizeStruct=normalizeFunc,
                .denormalizeStruct=denormalizeFunc,
                .normalizeValue=nullptr,
                .denormalizeValue=nullptr,
            };
        }

        static void registerEnum(const QString& enumName) {
            getRegistry()[enumName] = { .type=Type::Enum, .name=enumName };
        }

        static void registerValue(
            const QString& valueName,
            const NormalizeValueFunc& normalizeFunc,
            const DenormalizeValueFunc& denormalizeFunc
        ) {
            getRegistry()[valueName] = {
                .type=Type::Value,
                .name=valueName,
                .normalizeStruct=nullptr,
                .denormalizeStruct=nullptr,
                .normalizeValue=normalizeFunc,
                .denormalizeValue=denormalizeFunc,
            };
        }

        static StructInfo& getInfo(const QString& name) {
            ConfigRegistryItem &registry = getRegistry();
            if (registry.find(name) == registry.end()) {
                qDebug() << "Nie ma " << name;
            }
            return getRegistry()[name];
        }
};

#endif // CONFIG_REGISTRY_H
