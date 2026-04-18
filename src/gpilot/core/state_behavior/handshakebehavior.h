// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef HANDSHAKEBEHAVIOR_H
#define HANDSHAKEBEHAVIOR_H

#include "abstractstatebehavior.h"

// Performs a non-destructive handshake after connecting:
//   1. Queries current machine state (?).
//   2. If the machine is actively running an external process (Run/Jog/Check),
//      transitions immediately to ExternalProcessBehavior.
//   3. Otherwise queries device settings ($$) and coordinate offsets ($#).
//   4. Transitions to the appropriate behavior based on the initial machine state.
class HandshakeBehavior : public AbstractStateBehavior
{
    Q_OBJECT

public:
    explicit HandshakeBehavior(QObject *parent = nullptr);
    QString description() override;
    Type type() const override { return Type::Handshake; }

    Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
    Result doOnExit(AbstractStateBehavior *next) override;
    Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
    void doOnMachineState(MachineState state) override;

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
