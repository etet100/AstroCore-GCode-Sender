// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef DEVICECONTEXT_H
#define DEVICECONTEXT_H

#include <optional>
#include "physicalmachineconfiguration.h"
#include "modalstateparser.h"

enum class MachineType {
    Unknown,
    Grbl,
    GrblHAL,
    GCarvin,
    uCNC,
    FluidNC,
};

// Holds everything known about the connected device: its firmware type,
// physical hardware configuration (from $$), and current modal state (from $G).
// Populated during handshake; reset on disconnect.
class DeviceContext
{
    public:
        MachineType machineType() const { return m_machineType; }
        bool hasPhysicalConfig() const { return m_physicalConfig.has_value(); }
        PhysicalMachineConfiguration& physicalConfig() { return *m_physicalConfig; }
        const PhysicalMachineConfiguration& physicalConfig() const { return *m_physicalConfig; }
        bool hasModalState() const { return m_modalState.has_value(); }
        const ModalState& modalState() const { return *m_modalState; }

        void setMachineType(MachineType type) { m_machineType = type; }
        void setPhysicalConfig(PhysicalMachineConfiguration config) { m_physicalConfig = std::move(config); }
        void setModalState(ModalState state) { m_modalState = std::move(state); }

        void reset() {
            m_machineType = MachineType::Unknown;
            m_physicalConfig.reset();
            m_modalState.reset();
        }

    private:
        MachineType m_machineType = MachineType::Unknown;
        std::optional<PhysicalMachineConfiguration> m_physicalConfig;
        std::optional<ModalState> m_modalState;
};

#endif // DEVICECONTEXT_H
