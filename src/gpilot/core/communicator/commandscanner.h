#pragma once

#include <QObject>

// Scans outgoing G-code commands and emits signals for significant patterns.
// Call scan() from Communicator::sendCommand() before the command is enqueued.
class CommandScanner : public QObject
{
    Q_OBJECT

public:
    explicit CommandScanner(QObject* parent = nullptr);

    // Inspect one command line. Strips comments internally.
    void scan(const QString& commandLine);

signals:
    // G10, G92 / G92.x, G54-G59, $RST=# — anything that may shift the work offset.
    void workOffsetCommandDetected();

    // $H / $Hx (grblHAL single-axis), G28 / G28.1, G30 / G30.1 — homing or move-to-home.
    void homingCommandDetected();

    // M0, M00, M1, M01, M25 — program pause / optional stop.
    void pauseCommandDetected();

    // M6, M06 — tool change.
    void toolChangeCommandDetected();
};
