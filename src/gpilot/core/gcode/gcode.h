// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef GCODE_H
#define GCODE_H

#include <QObject>
#include "core/gcode/parser/gcodeparser.h"
#include <QTimer>

enum class StreamerStartResult
{
    Success = 0,
    UnacceptableCommunicatorState = 1,
    UnacceptableConnectionState = 2,
};

enum class GCodeItemGroup
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

struct GCodeItem
{
    enum States { InQueue = 0, Sent, Processed, Error, Skipped, Comment };

    QString command;
    QString comment;
    QString response;
    int lineNumber;
    States state = InQueue;
    QStringList args;
    GCodeItemGroup group = GCodeItemGroup::Unknown;
    PointSegment* ps = nullptr;

    bool isArc() const {
        return command.startsWith('G') && (command == "G2" || command == "G3");
    }
};

class GCode : public QObject
{
    Q_OBJECT

    public:
        explicit GCode(QObject *parent = nullptr);
        void reset(int commandIndex = 0);
        void resetProcessed(int commandIndex = 0);
        int commandIndex() { return m_commandIndex; }
        QString command();
        int processedCommandIndex() { return m_processedCommandIndex; }
        void advanceCommandIndex();
        bool isLastCommand();
        bool noMoreCommands();
        bool hasMoreCommands();
        int lastCommandIndex();
        bool isLastCommandProcessed();
        void setCommandSent();
        void setCommandResponse(int commandIndex, bool success, QString response);
        void setCommandSkipped();
        GCodeItem& operator [] (int index) { return m_data[index]; }
        int count() { return m_data.count(); }
        bool empty() { return m_data.isEmpty(); }
        void clear() { m_data.clear(); }
        GCode& operator << (const GCodeItem& item) {
            m_data.append(item);

            return *this;
        }
        GCode& operator << (const GCode& source) {
            for (const GCodeItem& item : source.m_data) {
                m_data.append(item);
            }
            return *this;
        }
        void reserve(int size) { m_data.reserve(size); }
        void insert(int index, const GCodeItem& item) {
            m_data.insert(index, item);
        }
        void removeAt(int index) {
            m_data.removeAt(index);
        }
        void erase(int begin, int end) {
            m_data.erase(m_data.begin() + begin, m_data.begin() + end);
        }
        QList<GCodeItem>::iterator begin() { return m_data.begin(); }
        QList<GCodeItem>::iterator end() { return m_data.end(); }

    private:
        int m_commandIndex;
        int m_processedCommandIndex;
        GcodeParser m_parser;
        QList<GCodeItem> m_data;
        int m_linesUpdatedFrom = INT_MAX;
        int m_linesUpdatedTo = INT_MIN;
        QTimer m_linesUpdatedTimer;
        QString m_contentHash;

        void addUpdatedRange(int commandIndex);
        // Calculates and updates the checksum/hash of the GCode data
        void updateHash();
        QString calculateHash() const;
        bool isModified() const;

    signals:
        void progressChanged(int progress);
        void linesUpdated(int fromLine, int toLine);
        void finished();
        void paused();
        void error();

    private slots:
        void onLinesUpdatedTimer();
};

#endif // GCODE_H
