#include "commandscanner.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>

CommandScanner::CommandScanner(QObject* parent)
    : QObject(parent)
{}

void CommandScanner::scan(const QString& commandLine)
{
    const QString cmd = GcodePreprocessorUtils::removeComment(commandLine)
                            .toUpper()
                            .simplified();

    if (cmd.isEmpty()) return;

    // Commands that may change the active work coordinate offset.
    //   G10 Lx Px   — set coordinate system origin
    //   G92 / G92.x — set / cancel / suspend / restore temporary offset
    //   G54–G59     — select active coordinate system
    //   $RST=#      — GRBL EEPROM reset (clears stored offsets)
    static const QRegularExpression workOffsetRe(
        "G10(?!\\d)"     // G10, not G100/G101/...
        "|G92(?!\\d)"    // G92 and all G92.x sub-codes
        "|G5[4-9]"       // G54 through G59
        "|\\$RST=#"
    );

    if (workOffsetRe.match(cmd).hasMatch()) {
        emit workOffsetCommandDetected();
    }

    // Homing cycle and move-to-home commands.
    //   $H          — home all axes (GRBL / grblHAL)
    //   $HX/Y/Z/... — home single axis (grblHAL)
    //   G28 / G28.1 — move to stored home position
    //   G30 / G30.1 — move to secondary stored home position
    static const QRegularExpression homingRe(
        "\\$H[XYZABC]?"   // $H and single-axis variants
        "|G28(?!\\d)"      // G28 / G28.1, not G280/G281/...
        "|G30(?!\\d)"      // G30 / G30.1, not G300/G301/...
    );

    if (homingRe.match(cmd).hasMatch()) {
        emit homingCommandDetected();
    }

    // Pause commands — stop program, wait for operator to resume.
    //   M0 / M00  — compulsory stop
    //   M1 / M01  — optional stop
    //   M25       — pause (grblHAL)
    static const QRegularExpression pauseRe(
        "M0{1,2}(?!\\d)"   // M0, M00
        "|M0?1(?!\\d)"     // M1, M01
        "|M25(?!\\d)"      // M25 (grblHAL)
    );

    if (pauseRe.match(cmd).hasMatch()) {
        emit pauseCommandDetected();
    }

    // Tool change.
    //   M6 / M06  — standard tool change command
    static const QRegularExpression toolChangeRe("M0*6(?!\\d)");

    if (toolChangeRe.match(cmd).hasMatch()) {
        emit toolChangeCommandDetected();
    }
}
