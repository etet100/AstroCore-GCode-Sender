// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef HANDSHAKEBEHAVIOR_H
#define HANDSHAKEBEHAVIOR_H

#include "statebehavior.h"

// Performs a non-destructive handshake after connecting:
//   1. Queries current machine state (?).
//   2. If the machine is actively running an external process (Run/Jog/Check),
//      transitions immediately to ExternalProcessBehavior.
//   3. Otherwise queries device settings ($$) and coordinate offsets ($#).
//   4. Transitions to the appropriate behavior based on the initial machine state.
class HandshakeBehavior : public StateBehavior
{
    Q_OBJECT

public:
    explicit HandshakeBehavior(QObject *parent = nullptr);
    QString description() override;

    Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
    Result onExit(StateBehavior *next = nullptr) override;
    Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
    void onMachineState(MachineState state) override;

protected:
    QString name() const override { return "Handshake"; }

private:
    enum Stage {
        QueryingState,
        QueryingSettings,
        QueryingOffsets,
        Completed
    };

    Stage m_stage = QueryingState;
    MachineState m_initialState = MachineState::Unknown;

    void transitionBasedOnInitialState();
};

#endif // HANDSHAKEBEHAVIOR_H
