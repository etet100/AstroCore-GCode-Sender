// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcodeitem.h"

QString GCodeItem::command() const
{
    if (line.isEmpty()) {
        return QString();
    }

    // Truncate at ';' (rest of line is a comment), then remove every '(...)'
    // block. Per NIST RS-274 G-code allows inline parenthesised comments,
    // e.g. "G1 (ostroznie) X10 (feed) Y20" => "G1 X10 Y20".
    const int semiPos = line.indexOf(';');
    QString cmd = (semiPos >= 0) ? line.left(semiPos) : line;

    int open;
    while ((open = cmd.indexOf('(')) >= 0) {
        const int close = cmd.indexOf(')', open);
        if (close < 0) {
            cmd.truncate(open);
            break;
        }
        cmd.remove(open, close - open + 1);
    }

    return cmd.trimmed().toUpper();
}

bool GCodeItem::isArc() const
{
    return group == GCodeItemGroup::ArcMovement;
}
