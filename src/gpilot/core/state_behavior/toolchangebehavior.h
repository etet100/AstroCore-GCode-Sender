// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef TOOLCHANGEBEHAVIOR_H
#define TOOLCHANGEBEHAVIOR_H

#include "statebehavior.h"
#include <QTimer>

class ToolChangeBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        enum class ToolChangeSource {
            Program,        // Tool change from G-code (M6)
            Manual          // Manual tool change request
        };

        enum class ToolChangeState {
            MovingToSafePosition,
            WaitingForUserConfirmation,
            ReturningToWorkPosition,
            Completed
        };

        explicit ToolChangeBehavior(int toolNumber, ToolChangeSource source = ToolChangeSource::Program, QObject *parent = nullptr);
        QString description() override;
        Type type() const override { return Type::ToolChange; }
        QSet<Action::Type> availableActions() const override {
            return { Action::Resume, Action::CycleStart };
        }
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;
        void onMachineStateChanged(MachineState state) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;

        int toolNumber() const { return m_toolNumber; }
        ToolChangeState changeState() const { return m_changeState; }

    protected:
        QString name() const override { return "ToolChange"; }
        bool doAction(const Action &action) override;

    private:
        int m_toolNumber;
        ToolChangeSource m_source;
        ToolChangeState m_changeState;
        QVector3D m_savedPosition;  // Position before tool change
        bool m_hasProbe;

        void moveToSafePosition();
        void waitForUserConfirmation();
        void returnToWorkPosition();
        void complete();

};

#endif // TOOLCHANGEBEHAVIOR_H
