// This file is a part of "Candle" application.
// This file was originally ported from "GcodeViewParse.java" class
// of "Universal GcodeSender" application written by Will Winder
// (https://github.com/winder/Universal-G-Code-Sender)

// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef GCODEVIEWPARSER_H
#define GCODEVIEWPARSER_H

#include <QObject>
#include <QVector3D>
#include <QVector2D>
#include "linesegment.h"
#include "gcodeparser.h"
#include "utils/utils.h"

class GCode;

class GCodeViewParser
{
    public:
        explicit GCodeViewParser();
        ~GCodeViewParser();

        QVector3D& getMinimumExtremes();
        QVector3D& getMaximumExtremes();
        double getMinLength() const;
        QSize getResolution() const;
        // QList<LineSegment>& toObjRedux(QList<QString> gcode, double arcPrecision, bool arcDegreeMode);
        QList<LineSegment>& getLineSegmentList();
        QList<LineSegment>& getLinesFromParser(GcodeParser *gp, double arcPrecision, bool arcDegreeMode);
        QList<LineSegment>& getLinesFromGCode(GCode &gcode, double arcPrecision, bool arcDegreeMode);

        QList<LineSegment>& getLines();
        QList<QList<int>>& getLinesIndexes();
        QList<LineSegment>& getSimplifiedLines(double simplifyPrecision);

        void reset();

    private:
        bool absoluteMode;
        bool absoluteIJK;

        // Parsed object
        QVector3D m_min, m_max;
        double m_minLength;
        QList<LineSegment> m_lines;
        QList<QList<int>> m_lineIndexes;
        QList<LineSegment> m_simplifiedLines;
        bool m_simplifiedLinesReady = false;
        double m_lastSimplifyPrecision = 0.0;

        // Parsing state.
        // QVector3D m_lastPoint;
        // int m_currentLine; // for assigning line numbers to segments.

        // Debug
        bool m_debug;
        void testExtremes(QVector3D p3d);
        void testExtremes(double x, double y, double z);
        void testLength(const QVector3D& start, const QVector3D& end);
        int getSegmentType(const LineSegment& segment) const;
};

#endif // GCODEVIEWPARSER_H
