// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "abstractconfigurationmodule.h"
#include <QColor>
#include <QRect>
#include <QVector3D>
#include <QByteArray>

AbstractConfigurationModule::AbstractConfigurationModule(QObject *parent, QMap<QString, QVariant> defaults) : QObject(parent), m_defaults(defaults)
{
    ConfigurationRegistry::registerStruct(
        "AbstractConfigurationModule::MinMax",
        [](const char* data) -> QVariantMap {
            AbstractConfigurationModule::MinMax *minMax = (AbstractConfigurationModule::MinMax*)data;

            return {
                {"min", minMax->min},
                {"max", minMax->max},
            };
        },
        [](QVariantMap map) -> QVariant {
            return QVariant::fromValue(AbstractConfigurationModule::MinMax{
                map["min"].toInt(),
                map["max"].toInt(),
            });
        }
    );
    ConfigurationRegistry::registerStruct(
        "AbstractConfigurationModule::MinMaxDouble",
        [](const char* data) -> QVariantMap {
            AbstractConfigurationModule::MinMaxDouble *minMax = (AbstractConfigurationModule::MinMaxDouble*)data;

            return {
                {"min", minMax->min},
                {"max", minMax->max},
            };
        },
        [](QVariantMap map) -> QVariant {
            return QVariant::fromValue(AbstractConfigurationModule::MinMaxDouble{
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
    ConfigurationRegistry::registerValue(
        "QByteArray",
        [](QVariant raw) -> QVariant {
            return QString::fromLatin1(raw.toByteArray().toBase64());
        },
        [](QVariant serialized) -> QVariant {
            return QByteArray::fromBase64(serialized.toString().toLatin1());
        }
    );
}
