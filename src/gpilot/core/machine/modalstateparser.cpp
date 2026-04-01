// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "modalstateparser.h"
#include <QStringList>

std::optional<ModalState> ModalStateParser::parse(const QString &line)
{
    if (!line.startsWith('[') || !line.endsWith(']')) {
        return std::nullopt;
    }

    const QString inner = line.mid(1, line.length() - 2);
    const QStringList tokens = inner.split(' ', Qt::SkipEmptyParts);

    ModalState state;

    for (const QString &token : tokens) {
        if (token.startsWith('G')) {
            const QString upper = token.toUpper();

            if (upper == "G54" || upper == "G55" || upper == "G56" ||
                upper == "G57" || upper == "G58" || upper == "G59")
            {
                state.coordinateSystem = upper;
            } else if (upper == "G17" || upper == "G18" || upper == "G19") {
                state.workPlane = upper;
            } else if (upper == "G20" || upper == "G21") {
                state.units = upper;
            } else if (upper == "G90" || upper == "G91") {
                state.motionMode = upper;
            } else if (upper == "G93" || upper == "G94" || upper == "G95") {
                state.feedMode = upper;
            }
        } else if (token.startsWith('M')) {
            const QString upper = token.toUpper();

            if (upper == "M3" || upper == "M4" || upper == "M5") {
                state.spindleMode = upper;
            }
        } else if (token.startsWith('F')) {
            bool ok = false;
            int value = token.mid(1).toInt(&ok);
            if (ok) {
                state.feedRate = value;
            }
        } else if (token.startsWith('S')) {
            bool ok = false;
            int value = token.mid(1).toInt(&ok);
            if (ok) {
                state.spindleSpeed = value;
            }
        }
    }

    return state;
}
