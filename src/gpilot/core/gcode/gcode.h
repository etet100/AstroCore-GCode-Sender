// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef GCODE_H
#define GCODE_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QHash>
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
    int insertedAt;     // index in m_data where first overlay item was inserted
    int count;          // number of commands in this overlay
};

enum class GCodeType { MainProgram, Macro };

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
        void setCommandAborted(int commandIndex);
        void setCommandSkipped();
        GCodeItem& operator [] (int index) { return m_data[index]; }
        GCodeItem& at(int index) { return m_data[index]; }
        GCodeItem& current() { return m_data[m_commandIndex]; }
        int count() { return m_data.count(); }
        bool empty() { return m_data.isEmpty(); }
        void clear() { m_data.clear(); m_responses.clear(); m_mainCount = 0; }
        void insertLines(int, QString text);
        void deleteLines(int from, int to);
        QString linesAsText(int from, int to);
        void replace(int from, int to, GCode& gcode);
        GCode& operator << (const GCodeItem& item);
        GCode& operator << (GCodeItem&& item);
        GCode& operator << (const GCode& source);
        void reserve(int size) { m_data.reserve(size); }
        // Structural edits invalidate index-based response keys, so we drop
        // the response map on any such change.
        void insert(int index, const GCodeItem& item) {
            if (item.overlayId == 0) m_mainCount++;
            m_data.insert(index, item);
            m_responses.clear();
        }
        void removeAt(int index) {
            if (m_data[index].overlayId == 0) m_mainCount--;
            m_data.removeAt(index);
            m_responses.clear();
        }
        void erase(int begin, int end) {
            for (int i = begin; i < end; i++) {
                if (m_data[i].overlayId == 0) m_mainCount--;
            }
            m_data.erase(m_data.begin() + begin, m_data.begin() + end);
            m_responses.clear();
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

        // Overlay support
        int insertOverlay(const QString& name, const QList<GCodeItem>& commands);
        const OverlayInfo* overlayInfo(int overlayId) const;
        void resetOverlays(int fromIndex = 0);
        bool isOverlayItem(int index) const;
        int mainCount() const;

        QString name() const { return m_name; }
        GCodeType type() const { return m_type; }
        void setName(const QString &name) { m_name = name; }
        void setType(GCodeType type) { m_type = type; }

        // Response storage. Kept in a sparse hash indexed by position to avoid
        // paying a QString header per item. "ok" responses are not stored;
        // they are inferred from the Processed state. Only error/custom
        // responses occupy memory.
        QString response(int index) const;
        void setResponse(int index, const QString& response);
        void clearResponses();

    private:
        QString m_name;
        GCodeType m_type = GCodeType::MainProgram;
        int m_commandIndex;
        int m_processedCommandIndex;
        bool m_iterationStarted = false;
        QList<GCodeItem> m_data;
        QHash<int, QString> m_responses;
        int m_mainCount = 0;
        QMap<int, OverlayInfo> m_overlays;
        int m_nextOverlayId = 0;
        int m_linesUpdatedFrom = INT_MAX;
        int m_linesUpdatedTo = INT_MIN;
        int m_lastSentCommand = INT_MAX;
        QTimer m_linesUpdatedTimer;
        QString m_contentHash;

        void addUpdatedRange(int index1, int index2 = -1);
        // Calculates and updates the checksum/hash of the GCode data
        // Main layer only, overlays are not included in the hash calculation
        void updateHash();
        QString calculateHash() const;
        bool isModified() const;

    signals:
        void progressChanged(int progress);
        void linesUpdated(int from, int to);
        void lastSentCommandChanged(int commandIndex);
        void loaded();
        // Emitted when execution enters or leaves an overlay (overlayId 0 = main program)
        void activeOverlayChanged(int overlayId);

    private slots:
        void onLinesUpdatedTimer();
};

#endif // GCODE_H
