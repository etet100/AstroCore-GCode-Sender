#include "commandscanner.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>

CommandScanner::CommandScanner(QObject* parent)
    : QObject(parent)
{}

CommandScanner::CommandType CommandScanner::classify(const QString& commandLine)
{
    const QString cmd = GcodePreprocessorUtils::removeComment(commandLine)
                            .toUpper()
                            .simplified();

    if (cmd.isEmpty()) {
        return CommandType::None;
    }

    // Commands that may change the active work coordinate offset.
    //   G10 Lx Px   — set coordinate system origin
    //   G92 / G92.x — set / cancel / suspend / restore temporary offset
    //   G54–G59     — select active coordinate system
    //   $RST=#      — GRBL EEPROM reset (clears stored offsets)
    static const QRegularExpression workOffsetRe(
        "G10(?!\\d)"
        "|G92(?!\\d)"
        "|G5[4-9]"
        "|\\$RST=#"
    );
    if (workOffsetRe.match(cmd).hasMatch()) {
        return CommandType::WorkOffset;
    }

    // Homing cycle and move-to-home commands.
    //   $H / $Hx  — home all / single axis
    //   G28 / G28.1, G30 / G30.1 — move to stored home position
    static const QRegularExpression homingRe(
        "\\$H[XYZABC]?"
        "|G28(?!\\d)"
        "|G30(?!\\d)"
    );
    if (homingRe.match(cmd).hasMatch()) {
        return CommandType::Homing;
    }

    // Pause commands.
    //   M0 / M00 — compulsory stop, M1 / M01 — optional stop, M25 — grblHAL pause
    static const QRegularExpression pauseRe(
        "M0{1,2}(?!\\d)"
        "|M0?1(?!\\d)"
        "|M25(?!\\d)"
    );
    if (pauseRe.match(cmd).hasMatch()) {
        return CommandType::Pause;
    }

    // Tool change: M6 / M06
    static const QRegularExpression toolChangeRe("M0*6(?!\\d)");
    if (toolChangeRe.match(cmd).hasMatch()) {
        return CommandType::ToolChange;
    }

    return CommandType::None;
}

void CommandScanner::scan(const QString& commandLine)
{
    switch (classify(commandLine)) {
        case CommandType::WorkOffset: emit workOffsetCommandDetected(); break;
        case CommandType::Homing:     emit homingCommandDetected();     break;
        case CommandType::Pause:      emit pauseCommandDetected();      break;
        case CommandType::ToolChange: emit toolChangeCommandDetected(); break;
        case CommandType::None:       break;
    }
}
