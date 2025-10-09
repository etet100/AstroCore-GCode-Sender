// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATION_MODULE_H
#define CONFIGURATION_MODULE_H

#include <QObject>
#include <QVariant>
#include "../registry.h"

class ConfigurationModule : public QObject
{
    Q_OBJECT

    public:
        ConfigurationModule(QObject *parent, QMap<QString, QVariant> defaults);

        QMap<QString, QVariant> getDefaults() { return m_defaults; }
        virtual QString getSectionName() = 0;

        struct MinMax {
            int min;
            int max;

            bool operator!=(const ConfigurationModule::MinMax& other) const {
                return min != other.min || max != other.max;
            }
        };

        struct MinMaxDouble {
            double min;
            double max;

            bool operator!=(const ConfigurationModule::MinMaxDouble& other) const {
                return min != other.min || max != other.max;
            }
        };

    private:
        QMap<QString, QVariant> m_defaults;

    signals:
        void changed();
};

Q_DECLARE_METATYPE(ConfigurationModule::MinMax);
Q_DECLARE_METATYPE(ConfigurationModule::MinMaxDouble);

#endif // CONFIGURATION_MODULE_H
