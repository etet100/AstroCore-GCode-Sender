// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef CB1D3DF8_6540_4A18_88B0_F995A2100C20
#define CB1D3DF8_6540_4A18_88B0_F995A2100C20
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATETOOLCHANGE_H
#define STATETOOLCHANGE_H

#include "state.h"

class StateToolChange : public State
{
    public:
        // Tool change type - program vs. manual
        enum class ToolChangeSource {
            Program,   // Tool change triggered by G-code program (M6)
            Manual     // Tool change triggered manually by user
        };

        explicit StateToolChange(State *previous, int toolNumber = 0,
                               ToolChangeSource source = ToolChangeSource::Program,
                               QObject *parent = nullptr);
        QString name() override { return "Tool Change: T" + QString::number(m_toolNumber); }
        bool isJoggingAllowed() override { return true; } // Allow jogging during tool change
        bool isHomingAllowed() override { return false; }
        void onEntry(Communicator *communicator, State *previous = nullptr) override;
        void onExit() override;
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QStringList response) override;

    private:
        int m_toolNumber;
        bool m_toolChangeConfirmed;
        ToolChangeSource m_source; // Source of tool change
};

#endif // STATETOOLCHANGE_H


#endif /* CB1D3DF8_6540_4A18_88B0_F995A2100C20 */
