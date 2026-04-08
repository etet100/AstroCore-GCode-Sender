// This file is a part of "G-Pilot GCode Sender" application.
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
            Value,
            StructList
        };

    private:
        typedef std::function<QVariantMap(const char*)> NormalizeStructFunc;
        typedef std::function<QVariant(QVariantMap)> DenormalizeStructFunc;
        typedef std::function<QVariant(QVariant)> NormalizeValueFunc;
        typedef std::function<QVariant(QVariant)> DenormalizeValueFunc;
        typedef std::function<QVariantList(const char*)> NormalizeStructListFunc;
        typedef std::function<QVariant(QVariantList)> DenormalizeStructListFunc;

        struct StructInfo {
            Type type;
            QString name;
            NormalizeStructFunc normalizeStruct = nullptr;
            DenormalizeStructFunc denormalizeStruct = nullptr;
            NormalizeValueFunc normalizeValue = nullptr;
            DenormalizeValueFunc denormalizeValue = nullptr;
            NormalizeStructListFunc normalizeStructList = nullptr;
            DenormalizeStructListFunc denormalizeStructList = nullptr;
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

        static void registerStructList(
            const QString& listTypeName,
            const NormalizeStructListFunc& normalizeFunc,
            const DenormalizeStructListFunc& denormalizeFunc
        ) {
            getRegistry()[listTypeName] = {
                .type=Type::StructList,
                .name=listTypeName,
                .normalizeStructList=normalizeFunc,
                .denormalizeStructList=denormalizeFunc,
            };
        }

        // Template helper: auto-builds list normalize/denormalize from element struct registration
        template<typename T>
        static void registerStructList(const QString& listTypeName, const QString& elementTypeName) {
            registerStructList(
                listTypeName,
                [elementTypeName](const char* data) -> QVariantList {
                    auto& elementInfo = getInfo(elementTypeName);
                    QList<T>* list = (QList<T>*)data;
                    QVariantList result;
                    for (const T& item : *list) {
                        result.append(elementInfo.normalizeStruct((const char*)&item));
                    }

                    return result;
                },
                [elementTypeName](QVariantList list) -> QVariant {
                    auto& elementInfo = getInfo(elementTypeName);
                    QList<T> result;
                    for (const QVariant& item : list) {
                        result.append(elementInfo.denormalizeStruct(item.toMap()).template value<T>());
                    }

                    return QVariant::fromValue(result);
                }
            );
        }

        static StructInfo& getInfo(const QString& name) {
            ConfigRegistryItem &registry = getRegistry();
            if (registry.find(name) == registry.end()) {
                qWarning() << "[Configuration] Missing " << name;
            }
            return getRegistry()[name];
        }
};

#endif // CONFIG_REGISTRY_H
