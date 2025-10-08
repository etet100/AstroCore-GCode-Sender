// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationmodule.h"
#include <QColor>
#include <QRect>
#include <QVector3D>

ConfigurationModule::ConfigurationModule(QObject *parent, QMap<QString, QVariant> defaults) : QObject(parent), m_defaults(defaults)
{
    ConfigurationRegistry::registerStruct(
        "ConfigurationModule::MinMax",
        [](const char* data) -> QVariantMap {
            ConfigurationModule::MinMax *minMax = (ConfigurationModule::MinMax*)data;

            return {
                {"min", minMax->min},
                {"max", minMax->max},
            };
        },
        [](QVariantMap map) -> QVariant {
            return QVariant::fromValue(ConfigurationModule::MinMax{
                map["min"].toInt(),
                map["max"].toInt(),
            });
        }
    );
    ConfigurationRegistry::registerStruct(
        "ConfigurationModule::MinMaxDouble",
        [](const char* data) -> QVariantMap {
            ConfigurationModule::MinMaxDouble *minMax = (ConfigurationModule::MinMaxDouble*)data;

            return {
                {"min", minMax->min},
                {"max", minMax->max},
            };
        },
        [](QVariantMap map) -> QVariant {
            return QVariant::fromValue(ConfigurationModule::MinMaxDouble{
                map["min"].toDouble(),
                map["max"].toDouble(),
            });
        }
    );
    ConfigurationRegistry::registerStruct(
        "QVector3D",
        [](const char* data) -> QVariantMap {
            QVector3D *vec = (QVector3D*)data;

            return {
                {"x", (double)vec->x()},
                {"y", (double)vec->y()},
                {"z", (double)vec->z()},
            };
        },
        [](QVariantMap map) -> QVariant {
            return QVariant::fromValue(QVector3D(
                map["x"].toFloat(),
                map["y"].toFloat(),
                map["z"].toFloat()
            ));
        }
    );
    ConfigurationRegistry::registerStruct(
        "QRect",
        [](const char* data) -> QVariantMap {
            QRect *rect = (QRect*)data;

            return {
                {"x", rect->x()},
                {"y", rect->y()},
                {"width", rect->width()},
                {"height", rect->height()},
            };
        },
        [](QVariantMap map) -> QVariant {
            return QVariant::fromValue(QRect{
                map["x"].toInt(),
                map["y"].toInt(),
                map["width"].toInt(),
                map["height"].toInt()
            });
        }
    );
    ConfigurationRegistry::registerValue(
        "QColor",
        [](QVariant raw) -> QVariant {
            return raw.value<QColor>().name(QColor::HexArgb);
        },
        [](QVariant serialized) -> QVariant {
            return QColor(serialized.toString());
        }
    );
}
