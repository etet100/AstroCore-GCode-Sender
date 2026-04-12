// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "machinecoordinatecache.h"

MachineCoordinateCache::MachineCoordinateCache()
    : m_tool(0)
{
    m_coords["G54"] = QVector3D();
    m_coords["G55"] = QVector3D();
    m_coords["G56"] = QVector3D();
    m_coords["G57"] = QVector3D();
    m_coords["G58"] = QVector3D();
    m_coords["G59"] = QVector3D();
    m_coords["G28"] = QVector3D();
    m_coords["G30"] = QVector3D();
    m_coords["G92"] = QVector3D();
    m_coords["TLO"] = QVector3D();
    m_coords["PRB"] = QVector3D();

    m_succesful["PRB"] = false;
    m_succesful["HOME"] = false;
}

double MachineCoordinateCache::G54x() const { return m_coords["G54"].x(); }
double MachineCoordinateCache::G54y() const { return m_coords["G54"].y(); }
double MachineCoordinateCache::G54z() const { return m_coords["G54"].z(); }

double MachineCoordinateCache::G55x() const { return m_coords["G55"].x(); }
double MachineCoordinateCache::G55y() const { return m_coords["G55"].y(); }
double MachineCoordinateCache::G55z() const { return m_coords["G55"].z(); }

double MachineCoordinateCache::G56x() const { return m_coords["G56"].x(); }
double MachineCoordinateCache::G56y() const { return m_coords["G56"].y(); }
double MachineCoordinateCache::G56z() const { return m_coords["G56"].z(); }

double MachineCoordinateCache::G57x() const { return m_coords["G57"].x(); }
double MachineCoordinateCache::G57y() const { return m_coords["G57"].y(); }
double MachineCoordinateCache::G57z() const { return m_coords["G57"].z(); }

double MachineCoordinateCache::G58x() const { return m_coords["G58"].x(); }
double MachineCoordinateCache::G58y() const { return m_coords["G58"].y(); }
double MachineCoordinateCache::G58z() const { return m_coords["G58"].z(); }

double MachineCoordinateCache::G59x() const { return m_coords["G59"].x(); }
double MachineCoordinateCache::G59y() const { return m_coords["G59"].y(); }
double MachineCoordinateCache::G59z() const { return m_coords["G59"].z(); }

double MachineCoordinateCache::G28x() const { return m_coords["G28"].x(); }
double MachineCoordinateCache::G28y() const { return m_coords["G28"].y(); }
double MachineCoordinateCache::G28z() const { return m_coords["G28"].z(); }

double MachineCoordinateCache::G30x() const { return m_coords["G30"].x(); }
double MachineCoordinateCache::G30y() const { return m_coords["G30"].y(); }
double MachineCoordinateCache::G30z() const { return m_coords["G30"].z(); }

double MachineCoordinateCache::G92x() const { return m_coords["G92"].x(); }
double MachineCoordinateCache::G92y() const { return m_coords["G92"].y(); }
double MachineCoordinateCache::G92z() const { return m_coords["G92"].z(); }

double MachineCoordinateCache::TLOx() const { return m_coords["TLO"].x(); }
double MachineCoordinateCache::TLOy() const { return m_coords["TLO"].y(); }
double MachineCoordinateCache::TLOz() const { return m_coords["TLO"].z(); }

double MachineCoordinateCache::PRBx() const { return m_coords["PRB"].x(); }
double MachineCoordinateCache::PRBy() const { return m_coords["PRB"].y(); }
double MachineCoordinateCache::PRBz() const { return m_coords["PRB"].z(); }

QVector3D MachineCoordinateCache::coords(const QString &name) const
{
    return m_coords.value(name, QVector3D());
}

bool MachineCoordinateCache::succesful(const QString &name) const
{
    return m_succesful.value(name, false);
}

double MachineCoordinateCache::x(const QString &name) const { return coords(name).x(); }
double MachineCoordinateCache::y(const QString &name) const { return coords(name).y(); }
double MachineCoordinateCache::z(const QString &name) const { return coords(name).z(); }

double MachineCoordinateCache::x() const { return coords(m_cs).x(); }
double MachineCoordinateCache::y() const { return coords(m_cs).y(); }
double MachineCoordinateCache::z() const { return coords(m_cs).z(); }

QString MachineCoordinateCache::CS() const { return m_cs; }
int MachineCoordinateCache::tool() const { return m_tool; }

void MachineCoordinateCache::setCoords(const QString &name, QVector3D coords)
{
    m_coords[name] = coords;
}

void MachineCoordinateCache::setSuccesful(const QString &name, bool succesful)
{
    m_succesful[name] = succesful;
}

void MachineCoordinateCache::setCS(const QString &cs)
{
    m_cs = cs;
}

void MachineCoordinateCache::setTool(int tool)
{
    m_tool = tool;
}
