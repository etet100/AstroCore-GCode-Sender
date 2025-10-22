#include "partmainstatebase.h"

void PartMainStateBase::initializeColorsAndCaptions()
{
    m_statusCaptions[MachineState::Unknown] = tr("Unknown");
    m_statusCaptions[MachineState::Idle] = tr("Idle");
    m_statusCaptions[MachineState::Alarm] = tr("Alarm");
    m_statusCaptions[MachineState::Run] = tr("Run");
    m_statusCaptions[MachineState::Home] = tr("Home");
    m_statusCaptions[MachineState::Hold0] = tr("Hold") + " (0)";
    m_statusCaptions[MachineState::Hold1] = tr("Hold") + " (1)";
    m_statusCaptions[MachineState::Queue] = tr("Queue");
    m_statusCaptions[MachineState::Check] = tr("Check");
    m_statusCaptions[MachineState::Door0] = tr("Door") + " (0)";
    m_statusCaptions[MachineState::Door1] = tr("Door") + " (1)";
    m_statusCaptions[MachineState::Door2] = tr("Door") + " (2)";
    m_statusCaptions[MachineState::Door3] = tr("Door") + " (3)";
    m_statusCaptions[MachineState::Jog] = tr("Jog");
    m_statusCaptions[MachineState::Sleep] = tr("Sleep");

    m_statusBackColors[MachineState::Unknown] = "red";
    m_statusBackColors[MachineState::Idle] = "palette(button)";
    m_statusBackColors[MachineState::Alarm] = "red";
    m_statusBackColors[MachineState::Run] = "lime";
    m_statusBackColors[MachineState::Home] = "lime";
    m_statusBackColors[MachineState::Hold0] = "yellow";
    m_statusBackColors[MachineState::Hold1] = "yellow";
    m_statusBackColors[MachineState::Queue] = "yellow";
    m_statusBackColors[MachineState::Check] = "palette(button)";
    m_statusBackColors[MachineState::Door0] = "red";
    m_statusBackColors[MachineState::Door1] = "red";
    m_statusBackColors[MachineState::Door2] = "red";
    m_statusBackColors[MachineState::Door3] = "red";
    m_statusBackColors[MachineState::Jog] = "lime";
    m_statusBackColors[MachineState::Sleep] = "blue";

    m_statusForeColors[MachineState::Unknown] = "white";
    m_statusForeColors[MachineState::Idle] = "palette(text)";
    m_statusForeColors[MachineState::Alarm] = "white";
    m_statusForeColors[MachineState::Run] = "black";
    m_statusForeColors[MachineState::Home] = "black";
    m_statusForeColors[MachineState::Hold0] = "black";
    m_statusForeColors[MachineState::Hold1] = "black";
    m_statusForeColors[MachineState::Queue] = "black";
    m_statusForeColors[MachineState::Check] = "palette(text)";
    m_statusForeColors[MachineState::Door0] = "white";
    m_statusForeColors[MachineState::Door1] = "white";
    m_statusForeColors[MachineState::Door2] = "white";
    m_statusForeColors[MachineState::Door3] = "white";
    m_statusForeColors[MachineState::Jog] = "black";
    m_statusForeColors[MachineState::Sleep] = "white";
}