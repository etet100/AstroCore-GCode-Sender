// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "applyheightmap.h"
#include <QVector3D>

enum class Plane {
    XY,
    XZ,
    YZ
};

struct State {
    bool absolute = true; // G90
    Plane plane = Plane::XY;
    QVector3D currentOffset = {0.0, 0.0, 0.0}; // G92 offset
    double unitsFactor = 1.0; // G20/G21
    QPointF currentPosMM = {0.0, 0.0};
    double currentZMM = 0.0;
};

// This converter applies a heightmap to the G-code commands. The process is as follows.
// Scan each command and adjust Z values based on the heightmap data. Some lines have to be split if heightmap
// changes significantly along the line. Arcs are converted to lines first. Heightmap points are interpolated
// bilinearly.
// List of move related commands: G0, G1, G2, G3
// And more commands to take into account:
// G90 Absolute mode - X/Y/Z coordinates are absolute.
// G91 Incremental mode - X/Y/Z coordinates are relative to the current position.
// G17 XY plane - For arcs G2/G3 - uses X and Y axes.
// G18 XZ plane - For arcs G2/G3 - uses X and Z axes.
// G19 YZ plane - For arcs G2/G3 - uses Y and Z axes.
// G20 Units: inches - All coordinates * 25.4 mm.
// G21 Units: mm - GRBL standard - no conversions.
// G92 Set offset of current position - Creates an additional offset that is added to each coordinate.
//     The offset remains active until the end of the program or until another G92 command is encountered.

ApplyHeightmap::ApplyHeightmap(GCode &data, Heightmap &heightmap)
    : Converter(data)
    , m_heightmap(heightmap)
    , m_interpolator(heightmap)
{
    State state;

    // for (GCodeItem &cmd : m_data) {
    // }
}
