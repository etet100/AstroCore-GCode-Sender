// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef ACTION_H
#define ACTION_H

#include <QMap>
#include <QString>
#include <QPointF>
#include <QVector3D>
#include <utility>
#include "core/gcode/gcode.h"
#include "core/heightmap/heightmap.h"

class Action
{
    public:
        enum Type : int {
            None = 0,
            Reset,
            Run,
            Abort,
            Pause,
            Resume,
            FeedHold,
            CycleStart,
            Jog,
            GoTo,
            Home,
            Probe,
            Unlock,
            QueryMachineConfiguration,
            SaveMachineConfigurationParam,
            ToolChange,
            ZeroZ,
            ZeroXY,
            Connect,
            Disconnect,
            CheckMode,
            ScanTable,
            RunMacro,
        };

        Action(Type type);
        virtual ~Action() = default;
        Type type() const { return m_type; }
        QString name() const {
            return NAMES.value(
                static_cast<int>(m_type),
                QString("Unknown: %1").arg(static_cast<int>(m_type))
            );
        }

    private:
        Type m_type;

        static const QMap<int, QString> NAMES;
};

class ScanTableAction : public Action
{
    public:
        explicit ScanTableAction(Heightmap* heightmap, int probeFeed = 100)
            : Action(Action::Type::ScanTable), m_heightmap(heightmap), m_probeFeed(probeFeed) {}
        Heightmap* heightmap() const { return m_heightmap; }
        int probeFeed() const { return m_probeFeed; }

    private:
        Heightmap* m_heightmap;
        int m_probeFeed;
};

class RunAction : public Action
{
    public:
        RunAction(GCode &program)
            : Action(Action::Type::Run)
            , m_program(program) {}
        GCode &program() const { return m_program; }

    private:
        GCode &m_program;
};

class RunMacroAction : public Action
{
    public:
        explicit RunMacroAction(GCode *macro)
            : Action(Action::Type::RunMacro)
            , m_macro(macro) {}
        // Transfers ownership of the macro GCode to the caller.
        GCode *takeMacro() const { return std::exchange(m_macro, nullptr); }

    private:
        mutable GCode *m_macro;
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

class JoggingAction : public Action
{
    public:
        JoggingAction(QVector3D vector, double distance, bool continuous, int feedRate, int feedRateZ)
            : Action(Action::Type::Jog), m_vector(vector), m_distance(distance), m_continuous(continuous), m_feedRate(feedRate), m_feedRateZ(feedRateZ) {
        }

        QVector3D vector() const { return m_vector; }
        double distance() const { return m_distance; }
        bool continuous() const { return m_continuous; }
        int feedRate() const { return m_feedRate; }
        int feedRateZ() const { return m_feedRateZ; }

    private:
        QVector3D m_vector;
        double m_distance;
        bool m_continuous;
        int m_feedRate;
        int m_feedRateZ;
};

class GoToAction : public Action
{
    public:
        GoToAction(const QPointF &target, int feedRate)
            : Action(Action::Type::GoTo), m_target(target), m_feedRate(feedRate) {
        }

        QPointF target() const { return m_target; }
        int feedRate() const { return m_feedRate; }

    private:
        QPointF m_target;
        int m_feedRate;
};

// Forward declaration
class ProbingBehavior;

class ProbeAction : public Action
{
    public:
        struct ProbeParameters {
            double fastFeedRate = 200.0;
            double slowFeedRate = 50.0;
            double maxDistance = 30.0;
            double retractDistance = 2.0;
            double safeDistance = 5.0;
            bool doubleProbe = false;   // true = fast + slow dual probe
            bool setZeroAtProbe = true;
            bool useAbsolute = false;
        };

        ProbeAction()
            : Action(Action::Type::Probe), m_params{} {
        }

        ProbeAction(ProbeParameters params)
            : Action(Action::Type::Probe), m_params(params) {
        }

        ProbeParameters params() const { return m_params; }

    private:
        ProbeParameters m_params;
};

#endif // ACTION_H
