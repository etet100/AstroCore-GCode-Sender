// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef GCODE_H
#define GCODE_H

#include <QObject>
#include <QTimer>
#include <vector>
#include <string>

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
    enum States { InQueue = 0, EmptyLine, Sent, Processed, Error, Skipped, Comment };

    int lineNumber;
    QString line;
    QString command;
    QString comment;
    QString response;
    int commandNumber;
    States state = InQueue;
    std::vector<std::string> args;
    GCodeItemGroup group = GCodeItemGroup::Unknown;
    bool isMovement = false;

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
        GCodeItem& at(int index) { return m_data[index]; }
        int count() { return m_data.count(); }
        bool empty() { return m_data.isEmpty(); }
        void clear() { m_data.clear(); }
        void insertLines(int, QString text);
        void deleteLines(int from, int to);
        QString linesAsText(int from, int to);
        void replace(int from, int to, GCode& gcode);
        GCode& operator << (const GCodeItem& item);
        GCode& operator << (GCodeItem&& item);
        GCode& operator << (const GCode& source);
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

        // offset is number of lines ahead (1 = next line, 2 = line after next, etc.)
        GCodeItem* lookAhead(int fromIndex, int offset) {
            if (fromIndex + offset >= m_data.count() || offset < 1) {
                return nullptr;
            }

            return &m_data[fromIndex + offset];
        }

        // offset is number of lines behind (1 = next line, 2 = line after next, etc.)
        GCodeItem* lookBehind(int fromIndex, int offset) {
            if (fromIndex - offset < 0 || offset < 1) {
                return nullptr;
            }

            return &m_data[fromIndex - offset];
        }

        GCodeItem* getLine(int index) {
            if (index < 0 || index >= m_data.count()) {
                return nullptr;
            }

            return &m_data[index];
        }

    private:
        int m_commandIndex;
        int m_processedCommandIndex;
        QList<GCodeItem> m_data;
        int m_linesUpdatedFrom = INT_MAX;
        int m_linesUpdatedTo = INT_MIN;
        int m_lastSentCommand = INT_MAX;
        QTimer m_linesUpdatedTimer;
        QString m_contentHash;

        void addUpdatedRange(int commandIndex);
        // Calculates and updates the checksum/hash of the GCode data
        void updateHash();
        QString calculateHash() const;
        bool isModified() const;

    signals:
        void progressChanged(int progress);
        void linesUpdated(int from, int to);
        void lastSentCommandChanged(int commandIndex);
        void loaded();

    private slots:
        void onLinesUpdatedTimer();
};

#endif // GCODE_H
