// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef ACTION_H
#define ACTION_H

#include <QMap>
#include <QString>

class Action
{
    public:
        enum Type : int {
            None = 0,
            Reset,
            Start,
            Stop,
            Pause,
            Resume,
            FeedHold,
            CycleStart,
            Jog,
            Home,
            Probe,
            Unlock,
            QueryMachineConfiguration,
            SaveMachineConfigurationParam,
        };

        Action(Type type);
        Type type() const { return m_type; }
        QString name() const {
            return NAMES.value(static_cast<int>(m_type), "Unknown");
        }

    private:
        Type m_type;

        static const QMap<int, QString> NAMES;
};

class SaveMachineConfigurationParamAction : public Action
{
    public:
        SaveMachineConfigurationParamAction(int index, double value)
            : Action(Action::Type::SaveMachineConfigurationParam), m_index(index), m_value(value) {
        }

        int index() const { return m_index; }
        double value() const { return m_value; }

    private:
        int m_index;
        double m_value;
};

#endif // ACTION_H
