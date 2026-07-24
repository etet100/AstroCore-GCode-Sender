// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef GCODE_H
#define GCODE_H

#include <QObject>
#include <QTimer>
#include <climits>
#include "gcodeitem.h"
#include "gcodeprogram.h"
#include "gcodecursor.h"

// QObject facade over the plain GCodeProgram (data) and GCodeCursor (execution
// state). It owns both, keeps the public API stable for existing callers, and
// is the only layer that emits view-update signals. The update notifications
// are coalesced and flushed by an internal timer (see flushPendingUpdates).
class GCode : public QObject
{
    Q_OBJECT

    public:
        explicit GCode(QObject *parent = nullptr);
        void reset(int commandIndex = 0);
        void resetProcessed(int commandIndex = 0);
        int commandIndex() { return m_cursor.commandIndex(); }
        QString command();
        int processedCommandIndex() { return m_cursor.processedCommandIndex(); }
        void advanceCommandIndex();
        bool isLastCommand() { return m_cursor.isLastCommand(); }
        bool noMoreCommands() { return m_cursor.noMoreCommands(); }
        bool hasMoreCommands() { return m_cursor.hasMoreCommands(); }
        int lastCommandIndex() { return m_cursor.lastCommandIndex(); }
        bool isLastCommandProcessed() { return m_cursor.isLastCommandProcessed(); }
        void setCommandSent();
        void setCommandResponse(int commandIndex, bool success, QString response);
        void setCommandAborted(int commandIndex);
        void setCommandSkipped();
        GCodeItem& operator [] (int index) { return m_program[index]; }
        GCodeItem& at(int index) { return m_program.at(index); }
        GCodeItem& current() { return m_program[m_cursor.commandIndex()]; }
        int count() { return m_program.count(); }
        bool empty() { return m_program.empty(); }
        void clear() { m_program.clear(); }
        void insertLines(int, QString text);
        void deleteLines(int from, int to);
        QString linesAsText(int from, int to) { return m_program.linesAsText(from, to); }
        void replace(int from, int to, GCode& gcode);
        GCode& operator << (const GCodeItem& item);
        GCode& operator << (GCodeItem&& item);
        GCode& operator << (const GCode& source);
        void reserve(int size) { m_program.reserve(size); }
        void insert(int index, const GCodeItem& item) { m_program.insert(index, item); }
        void removeAt(int index) { m_program.removeAt(index); }
        void erase(int begin, int end) { m_program.erase(begin, end); }
        QList<GCodeItem>::iterator begin() { return m_program.begin(); }
        QList<GCodeItem>::iterator end() { return m_program.end(); }

        GCodeItem* lookAhead(int fromIndex, int offset) { return m_program.lookAhead(fromIndex, offset); }
        GCodeItem* lookBehind(int fromIndex, int offset) { return m_program.lookBehind(fromIndex, offset); }
        GCodeItem* getLine(int index) { return m_program.getLine(index); }

        // Overlay support
        int insertOverlay(const QString& name, const QList<GCodeItem>& commands);
        const OverlayInfo* overlayInfo(int overlayId) const { return m_program.overlayInfo(overlayId); }
        void resetOverlays(int fromIndex = 0);
        bool isOverlayItem(int index) const { return m_program.isOverlayItem(index); }
        int mainCount() const { return m_program.mainCount(); }

        QString name() const { return m_name; }
        GCodeType type() const { return m_type; }
        void setName(const QString &name) {
            if (m_name == name) return;
            m_name = name;
            emit nameChanged(m_name);
        }
        void setType(GCodeType type) { m_type = type; }

        QString response(int index) const { return m_program.response(index); }
        void setResponse(int index, const QString& response) { m_program.setResponse(index, response); }
        void clearResponses() { m_program.clearResponses(); }

        // Access to the underlying pure components (mainly for tests and callers
        // that want the container/cursor without the signal layer).
        GCodeProgram& program() { return m_program; }
        GCodeCursor& cursor() { return m_cursor; }

        // View-update notifications are coalesced and normally flushed by an
        // internal timer. These expose the flush so callers (and tests) can
        // deliver them synchronously without waiting for the timer.
        void flushPendingUpdates();
        void setAutoFlushEnabled(bool enabled);

    private:
        GCodeProgram m_program;
        GCodeCursor m_cursor { m_program };
        QString m_name;
        GCodeType m_type = GCodeType::MainProgram;

        // Coalesced view-update state (signal layer only).
        int m_linesUpdatedFrom = INT_MAX;
        int m_linesUpdatedTo = INT_MIN;
        int m_lastSentCommand = INT_MAX;
        QTimer m_linesUpdatedTimer;

        void addUpdatedRange(int index1, int index2 = -1);

    signals:
        void progressChanged(int progress);
        void linesUpdated(int from, int to);
        // Emitted when the number of lines changes (delete, insert, replace with
        // a different line count). Unlike linesUpdated (data only), this requires
        // views to reset because row indices and the row count are no longer valid.
        void structureChanged();
        void lastSentCommandChanged(int commandIndex);
        void loaded();
        // Emitted when execution enters or leaves an overlay (overlayId 0 = main program)
        void activeOverlayChanged(int overlayId);
        void nameChanged(const QString &name);

    private slots:
        void onLinesUpdatedTimer();
};

#endif // GCODE_H
