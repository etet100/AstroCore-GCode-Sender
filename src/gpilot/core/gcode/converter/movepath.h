// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef MOVEPATH_H
#define MOVEPATH_H

#include "abstractconverter.h"

// Shifts all G90 (absolute) movement coordinates by the given X, Y, Z offsets.
// G91 (incremental) lines are left unchanged — adding a fixed offset to relative
// coordinates would break the path geometry.
// Tracks G90/G91 mode changes. Defaults to G90 at start (standard CNC default).
//
// Limitations:
//   - G92 (set work coordinate offset) lines are not modified. If the file uses
//     G92, the shift applies to movements within the active coordinate system.
//   - If the offset causes a coordinate to sit outside the machine travel, no
//     warning is produced — the caller is responsible for range checking.
class MovePath : public AbstractConverter
{
public:
    static QString parameterSchema();

    MovePath(double offsetX = 0.0, double offsetY = 0.0, double offsetZ = 0.0);

    bool convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser) override;
    void reset() override;

    void setOffset(double x, double y, double z);
    double offsetX() const { return m_offsetX; }
    double offsetY() const { return m_offsetY; }
    double offsetZ() const { return m_offsetZ; }

private:
    double m_offsetX;
    double m_offsetY;
    double m_offsetZ;
    bool   m_absoluteMode;

    QString applyOffset(const QString &cmd, char axis, double offset) const;
};

#endif // MOVEPATH_H
