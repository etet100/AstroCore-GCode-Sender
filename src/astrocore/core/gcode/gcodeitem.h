// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef GCODEITEM_H
#define GCODEITEM_H

#include <QString>
#include <cstdint>
#include <vector>
#include <string>

enum class StreamerStartResult
{
    Success = 0,
    UnacceptableCommunicatorState = 1,
    UnacceptableConnectionState = 2,
};

enum class GCodeItemGroup : uint8_t
{
    Movement = 0,
    RapidMovement = 1,
    ArcMovement = 2,
    Dwell = 3,
    Spindle = 4,
    Coolant = 5,
    ToolChange = 6,
    CoordinateSystemSelection = 7,
    UnitsSelection = 8,
    FeedRateMode = 9,
    PlaneSelection = 10,
    CutterCompensation = 11,
    ReturnToReferencePoint = 12,
    Miscellaneous = 13,
    Comment = 14,
    Unknown = 15
};

// Field order chosen for packing on 64-bit: wide members first, then
// smaller ints, then single-byte enums/bool at the tail.
struct GCodeItem
{
    enum States : uint8_t { InQueue = 0, EmptyLine, Sent, Processed, Error, Skipped, Aborted, Comment };

    QString line;                                 // full trimmed source line (keeps original case and comments)
    QString comment;                              // first extracted comment (for display only)
    std::vector<std::string> args;                // parsed argument tokens
    int lineNumber = 0;
    int commandNumber = 0;
    int16_t overlayId = 0;                        // 0 = main program, >0 = overlay id
    States state = InQueue;
    GCodeItemGroup group = GCodeItemGroup::Unknown;
    bool isMovement = false;

    // Computes the executable command text from `line`: comments stripped and
    // uppercased (matches the original parser semantics).
    QString command() const;

    bool isArc() const;

    bool isOverlay() const {
        return overlayId > 0;
    }
};

struct OverlayInfo {
    int id;
    QString name;
    int insertedAt;     // index in the program where first overlay item was inserted
    int count;          // number of commands in this overlay
};

enum class GCodeType { MainProgram, Macro };

#endif // GCODEITEM_H
