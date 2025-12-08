// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef ACTION_H
#define ACTION_H

class Action
{
    public:
        enum Type {
            None,
            Reset,
            Start,
            Stop,
            Pause,
            Resume,
            FeedHold,
            CycleStart,
            Jog,
            Home,
            Unlock,
            QueryMachineConfiguration,
            SaveMachineConfigurationParam,
        };

        Action(Type type);
        Type type() const { return m_type; }

    private:
        Type m_type;
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
