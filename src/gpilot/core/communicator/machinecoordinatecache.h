// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#pragma once

#include <QMap>
#include <QVector3D>
#include <QString>

// Stores work coordinate offsets received from GRBL ($# response).
// Covers G54-G59, G28, G30, G92, TLO, PRB, active CS, and active tool.
// Machine pos and work offset are owned by PositionTracker directly.
class MachineCoordinateCache
{
public:
    MachineCoordinateCache();

    double G54x() const;
    double G54y() const;
    double G54z() const;

    double G55x() const;
    double G55y() const;
    double G55z() const;

    double G56x() const;
    double G56y() const;
    double G56z() const;

    double G57x() const;
    double G57y() const;
    double G57z() const;

    double G58x() const;
    double G58y() const;
    double G58z() const;

    double G59x() const;
    double G59y() const;
    double G59z() const;

    double G28x() const;
    double G28y() const;
    double G28z() const;

    double G30x() const;
    double G30y() const;
    double G30z() const;

    double G92x() const;
    double G92y() const;
    double G92z() const;

    double TLOx() const;
    double TLOy() const;
    double TLOz() const;

    double PRBx() const;
    double PRBy() const;
    double PRBz() const;

    // Returns coordinates of the given system by name (e.g. "M", "G54").
    QVector3D coords(const QString &name) const;

    // Returns whether the last PROBE/HOME command was succesful
    bool succesful(const QString &name) const;

    // Returns x/y/z for a named coordinate system.
    double x(const QString &name) const;
    double y(const QString &name) const;
    double z(const QString &name) const;

    // Returns x/y/z for the currently active coordinate system.
    double x() const;
    double y() const;
    double z() const;

    QString CS() const;
    int tool() const;

    void setCoords(const QString &name, QVector3D coords);
    void setSuccesful(const QString &name, bool succesful);
    void setCS(const QString &cs);
    void setTool(int tool);

private:
    QMap<QString, QVector3D> m_coords;
    QMap<QString, bool> m_succesful;
    QString m_cs;
    int m_tool;
};
