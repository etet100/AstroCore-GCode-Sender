// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef GCODE_H
#define GCODE_H

#include <QObject>
#include "core/gcode/parser/gcodeparser.h"

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
    enum States { InQueue = 0, Sent, Processed, Skipped, Comment };

    QString command;
    QString comment;
    QString response;
    int lineNumber;
    States state = InQueue;
    QStringList args;
    GCodeItemGroup group = GCodeItemGroup::Unknown;

    bool isArc() const {
        return command.startsWith('G') && (command == "G2" || command == "G3");
    }
};

class GCode : public QObject , public QList<GCodeItem>
{
    Q_OBJECT

    public:
        explicit GCode( QObject *parent = nullptr);
        //void setData(QList<GCodeItem> data);
        void reset(int commandIndex = 0);
        void resetProcessed(int commandIndex = 0);
        int commandIndex() { return m_commandIndex; }
        QString command();
        int processedCommandIndex() { return m_processedCommandIndex; }
        void advanceCommandIndex();
        StreamerStartResult start();
        void stop();
        void pause();
        bool isLastCommand();
        bool noMoreCommands();
        bool hasMoreCommands();
        int lastCommandIndex();
        bool isLastCommandProcessed();
        void commandSent();

    private:
        int m_commandIndex;
        int m_processedCommandIndex;
        GcodeParser m_parser;

    signals:
        void progressChanged(int progress);
        void finished();
        void paused();
        void error();
};

#endif // GCODE_H
