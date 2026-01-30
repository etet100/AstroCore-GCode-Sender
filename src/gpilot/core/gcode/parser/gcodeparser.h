// This file is a part of "Candle" application.
// This file was originally ported from "GcodeParser.java" class
// of "Universal GcodeSender" application written by Will Winder
// (https://github.com/winder/Universal-G-Code-Sender)

// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef GCODEPARSER_H
#define GCODEPARSER_H

#include <QObject>
#include <QVector3D>
#include <cmath>
#include "pointsegment.h"
#include "gcodepreprocessorutils.h"

struct GCodeItem;

struct GcodeParserState
{
    bool isMetric;
    bool inAbsoluteMode;
    bool inAbsoluteIJKMode;
    float lastGcodeCommand;
    QVector3D currentPoint;
    int commandNumber;
    PointSegment::planes currentPlane;
    double lastSpeed;
    double lastSpindleSpeed;
    int pointsCount = 0;
    float activeCannedCycle = -1; // G80-G89, -1 means no active cycle
    double cannedR = 0.0;  // R - retract plane
    double cannedZ = 0.0;  // Z - hole depth
    double cannedQ = 0.0;  // Q - peck increment (for G83)
    double cannedP = 0.0;  // P - dwell time at bottom (for G82)
};

class GcodeParser : public QObject
{
    Q_OBJECT
public:
    explicit GcodeParser(QObject *parent = 0);
    ~GcodeParser();

    bool getConvertArcsToLines();
    void setConvertArcsToLines(bool convertArcsToLines);
    bool getRemoveAllWhitespace();
    void setRemoveAllWhitespace(bool removeAllWhitespace);
    double getSmallArcSegmentLength();
    void setSmallArcSegmentLength(double smallArcSegmentLength);
    double getSmallArcThreshold();
    void setSmallArcThreshold(double smallArcThreshold);
    double getSpeedOverride();
    void setSpeedOverride(double speedOverride);
    int getTruncateDecimalLength();
    void setTruncateDecimalLength(int truncateDecimalLength);
    // Theoretically the initial point is unknown, can we assume (0,0,0)?
    void reset(const QVector3D &initialPoint = QVector3D(0, 0, 0));
    PointSegment *addCommand(QString command);
    PointSegment *addCommand(const QStringList &args);
    PointSegment *addCommand(const GCodeItem &gcodeItem);
    QVector3D* getCurrentPoint();
    QList<PointSegment *> expandArc();
    // QStringList preprocessCommands(QStringList commands);
    // QStringList preprocessCommand(QString command);
    // QStringList convertArcsToLines(QString command);
    QList<PointSegment *> getPointSegmentList();
    double getTraverseSpeed() const;
    void setTraverseSpeed(double traverseSpeed);
    int getCommandNumber() const;

    // Save and restore parser state using a stack, can be used to rollback after line modification
    // (e.g. in converters)
    void pushState();
    void popState();

    // Legacy methods for backward compatibility
    GcodeParserState saveState() const;
    void restoreState(const GcodeParserState &state);

private:
    GcodeParserState m_state;
    QList<GcodeParserState> m_stateStack;

    // Settings
    double m_speedOverride;
    int m_truncateDecimalLength;
    bool m_removeAllWhitespace;
    bool m_convertArcsToLines;
    double m_smallArcThreshold;
    double m_smallArcSegmentLength;
    double m_traverseSpeed;

    // The gcode.
    QList<PointSegment*> m_points;

    PointSegment *processCommand(const QStringList &args);
    void handleMCode(float code, const QStringList &args);
    PointSegment *handleGCode(float code, const QStringList &args);
    PointSegment *addLinearPointSegment(const QVector3D &nextPoint, bool fastTraverse);
    PointSegment *addArcPointSegment(const QVector3D &nextPoint, bool clockwise, const QStringList &args);
    void setLastGcodeCommand(float num);
    void expandCannedCycle(const QVector3D &position);
};

#endif // GCODEPARSER_H
