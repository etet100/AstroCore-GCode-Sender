// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef GCODECURSOR_H
#define GCODECURSOR_H

#include "gcodeprogram.h"

// Execution cursor over a GCodeProgram: the send/processed positions and the
// queries used while streaming a program. Holds no Qt signals, so it can be
// unit tested against an in-memory program without an event loop.
class GCodeCursor
{
    public:
        explicit GCodeCursor(GCodeProgram& program) : m_program(program) {}

        void reset(int commandIndex = 0) {
            m_commandIndex = commandIndex;
            m_processedCommandIndex = commandIndex;
            m_iterationStarted = false;
        }

        void resetProcessed(int commandIndex = 0) { m_processedCommandIndex = commandIndex; }

        int commandIndex() const { return m_commandIndex; }
        int processedCommandIndex() const { return m_processedCommandIndex; }
        bool iterationStarted() const { return m_iterationStarted; }

        void setProcessedIndex(int index) { m_processedCommandIndex = index; }

        // Marks iteration started and steps to the next command.
        void advance() {
            m_iterationStarted = true;
            m_commandIndex++;
        }

        // Overlay id at a given position (0 = main program / out of range).
        int overlayIdAt(int index) const {
            return (index >= 0 && index < m_program.count()) ? m_program[index].overlayId : 0;
        }

        bool isLastCommand() const { return m_commandIndex == m_program.count() - 1; }
        bool noMoreCommands() const { return m_commandIndex > m_program.count() - 1; }
        bool hasMoreCommands() const { return !m_program.empty() && m_commandIndex <= m_program.count() - 1; }
        int lastCommandIndex() const { return m_program.count() - 1; }
        bool isLastCommandProcessed() const { return m_processedCommandIndex == m_program.count() - 1; }

    private:
        GCodeProgram& m_program;
        int m_commandIndex = 0;
        int m_processedCommandIndex = 0;
        bool m_iterationStarted = false;
};

#endif // GCODECURSOR_H
