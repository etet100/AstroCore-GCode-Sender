// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef DEVICECONTEXT_H
#define DEVICECONTEXT_H

#include <optional>
#include "physicalmachineconfiguration.h"
#include "modalstateparser.h"
#include "machinestatus.h"

enum class MachineType {
    Unknown,
    Grbl,
    GrblHAL,
    GCarvin,
    uCNC,
    FluidNC,
};

// Holds everything known about the connected device: firmware type,
// physical hardware configuration (from $$), modal parser state (from $G),
// and the last received status report (updated every status cycle).
// Populated during the session; reset on disconnect.
class DeviceContext
{
    public:
        MachineType machineType() const { return m_machineType; }
        bool hasPhysicalConfig() const { return m_physicalConfig.has_value(); }
        PhysicalMachineConfiguration& physicalConfig() { return *m_physicalConfig; }
        const PhysicalMachineConfiguration& physicalConfig() const { return *m_physicalConfig; }
        bool hasModalState() const { return m_modalState.has_value(); }
        const ModalState& modalState() const { return *m_modalState; }

        MachineState machineState() const {
            return m_lastStatusReport.has_value() ? m_lastStatusReport->state
                                                  : MachineState::Unknown;
        }

        bool hasLastStatusReport() const { return m_lastStatusReport.has_value(); }
        const MachineStatusReport& lastStatusReport() const { return *m_lastStatusReport; }

        void setMachineType(MachineType type) { m_machineType = type; }
        void setPhysicalConfig(PhysicalMachineConfiguration config) { m_physicalConfig = std::move(config); }
        void setModalState(ModalState state) { m_modalState = std::move(state); }
        void setLastStatusReport(MachineStatusReport report) { m_lastStatusReport = std::move(report); }

        void reset() {
            m_machineType = MachineType::Unknown;
            m_physicalConfig.reset();
            m_modalState.reset();
            m_lastStatusReport.reset();
        }

    private:
        MachineType m_machineType = MachineType::Unknown;
        std::optional<PhysicalMachineConfiguration> m_physicalConfig;
        std::optional<ModalState> m_modalState;
        std::optional<MachineStatusReport> m_lastStatusReport;
};

#endif // DEVICECONTEXT_H
