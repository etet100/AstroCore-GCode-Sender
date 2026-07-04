// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATION_MODULE_H
#define CONFIGURATION_MODULE_H

#include <QObject>
#include <QVariant>
#include "../registry.h"

class AbstractConfigurationModule : public QObject
{
    friend class FrmSettings;

    Q_OBJECT

    public:
        AbstractConfigurationModule(QObject *parent, QMap<QString, QVariant> defaults);

        QMap<QString, QVariant> getDefaults() { return m_defaults; }
        virtual QString getSectionName() = 0;

        struct MinMax {
            int min;
            int max;

            bool operator!=(const AbstractConfigurationModule::MinMax& other) const {
                return min != other.min || max != other.max;
            }

            bool operator==(const AbstractConfigurationModule::MinMax& other) const {
                return min == other.min && max == other.max;
            }
        };

        struct MinMaxDouble {
            double min;
            double max;

            bool operator!=(const AbstractConfigurationModule::MinMaxDouble& other) const {
                return min != other.min || max != other.max;
            }
        };

    private:
        QMap<QString, QVariant> m_defaults;
        void emitChanged() { emit changed(); }

    signals:
        void changed();
};

Q_DECLARE_METATYPE(AbstractConfigurationModule::MinMax);
Q_DECLARE_METATYPE(AbstractConfigurationModule::MinMaxDouble);

#endif // CONFIGURATION_MODULE_H
