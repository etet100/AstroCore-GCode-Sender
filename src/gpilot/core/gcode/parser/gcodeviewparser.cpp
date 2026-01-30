// This file is a part of "Candle" application.
// This file was originally ported from "GcodeViewParse.java" class
// of "Universal GcodeSender" application written by Will Winder
// (https://github.com/winder/Universal-G-Code-Sender)

// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include <QDebug>
#include "gcodeviewparser.h"
#include "core/gcode/gcode.h"

GCodeViewParser::GCodeViewParser()
{
    reset();
}

GCodeViewParser::~GCodeViewParser()
{
    //foreach (LineSegment *ls, m_lines) delete ls;
}

QVector3D &GCodeViewParser::getMinimumExtremes()
{
    return m_min;
}

QVector3D &GCodeViewParser::getMaximumExtremes()
{
    return m_max;
}

void GCodeViewParser::testExtremes(QVector3D p3d)
{
    testExtremes(p3d.x(), p3d.y(), p3d.z());
}

void GCodeViewParser::testExtremes(double x, double y, double z)
{
    m_min.setX(Utils::nMin(m_min.x(), x));
    m_min.setY(Utils::nMin(m_min.y(), y));
    m_min.setZ(Utils::nMin(m_min.z(), z));

    m_max.setX(Utils::nMax(m_max.x(), x));
    m_max.setY(Utils::nMax(m_max.y(), y));
    m_max.setZ(Utils::nMax(m_max.z(), z));
}

void GCodeViewParser::testLength(const QVector3D &start, const QVector3D &end)
{
    double length = (start - end).length();
    if (!qIsNaN(length) && length != 0) m_minLength = qIsNaN(m_minLength) ? length : qMin<double>(m_minLength, length);
}

// QList<LineSegment> &GCodeViewParser::toObjRedux(QList<QString> gcode, double arcPrecision, bool arcDegreeMode)
// {
//     GcodeParser parser;

//     foreach (QString s, gcode) {
//         parser.addCommand(s);
//     }

//     return getLinesFromParser(&parser, arcPrecision, arcDegreeMode);
// }

QList<LineSegment> &GCodeViewParser::getLineSegmentList()
{
    return m_lines;
}

void GCodeViewParser::reset()
{
    //foreach (LineSegment &ls, m_lines) delete ls;
    m_lines.clear();
    m_lineIndexes.clear();
    m_simplifiedLines.clear();
    m_simplifiedLinesReady = false;
    m_lastSimplifyPrecision = 0.0;
    // m_currentLine = 0;
    m_min = QVector3D(qQNaN(), qQNaN(), qQNaN());
    m_max = QVector3D(qQNaN(), qQNaN(), qQNaN());
    m_minLength = qQNaN();

    // Reset parser state to initial values
    absoluteMode = true;
    absoluteIJK = false;
    m_debug = true;
}

double GCodeViewParser::getMinLength() const
{
    return m_minLength;
}

QSize GCodeViewParser::getResolution() const
{
    return QSize(((m_max.x() - m_min.x()) / m_minLength) + 1, ((m_max.y() - m_min.y()) / m_minLength) + 1);
}

QList<LineSegment>& GCodeViewParser::getLinesFromParser(GcodeParser *parser, double arcPrecision, bool arcDegreeMode)
{
    assert(m_lines.empty());

    QList<PointSegment*> psl = parser->getPointSegmentList();
    // For a line segment list ALL arcs must be converted to lines.
    double minArcLength = 0.1;

    QVector3D *start = nullptr;
    QVector3D *end = nullptr;

    // Prepare segments indexes
    m_lineIndexes.resize(psl.count());

    int lineIndex = 0;
    foreach (PointSegment *segment, psl) {
        PointSegment *ps = segment;
        bool isMetric = ps->isMetric();
        ps->convertToMetric();

        end = ps->point();

        // start is null for the first iteration.
        if (start != nullptr) {
            // Expand arc for graphics.
            if (ps->isArc()) {
                QList<QVector3D> points =
                    GcodePreprocessorUtils::generatePointsAlongArcBDring(ps->plane(),
                    *start, *end, *ps->center(), ps->isClockwise(), ps->getRadius(), minArcLength, arcPrecision, arcDegreeMode);

                // Create line segments from points.
                if (!points.empty()) {
                    QVector3D arcStart = *start;
                    foreach (QVector3D arcNext, points) {
                        if (arcNext == arcStart) {
                            continue;
                        }
                        LineSegment ls(arcStart, arcNext, lineIndex);
                        ls.setIsArc(ps->isArc());
                        ls.setIsClockwise(ps->isClockwise());
                        ls.setPlane(ps->plane());
                        ls.setIsFastTraverse(ps->isFastTraverse());
                        ls.setIsZMovement(ps->isZMovement());
                        ls.setIsMetric(isMetric);
                        ls.setIsAbsolute(ps->isAbsolute());
                        ls.setSpeed(ps->getSpeed());
                        ls.setSpindleSpeed(ps->getSpindleSpeed());
                        ls.setDwell(ps->getDwell());

                        testExtremes(arcNext);

                        m_lines << ls;
                        m_lineIndexes[ps->getLineNumber()] << m_lines.count() - 1;

                        arcStart = arcNext;
                    }
                    lineIndex++;
                }
            // Line
            } else {
                LineSegment ls(*start, *end, lineIndex++);
                ls.setIsArc(ps->isArc());
                ls.setIsFastTraverse(ps->isFastTraverse());
                ls.setIsZMovement(ps->isZMovement());
                ls.setIsMetric(isMetric);
                ls.setIsAbsolute(ps->isAbsolute());
                ls.setSpeed(ps->getSpeed());
                ls.setSpindleSpeed(ps->getSpindleSpeed());
                ls.setDwell(ps->getDwell());

                testExtremes(*end);
                testLength(*start, *end);

                m_lines << ls;
                m_lineIndexes[ps->getLineNumber()] << m_lines.count() - 1;
            }
        }
        start = end;
    }

    return m_lines;
}

QList<LineSegment>& GCodeViewParser::getLines()
{
    return m_lines;
}

QList<QList<int>>& GCodeViewParser::getLinesIndexes()
{
    return m_lineIndexes;
}

int GCodeViewParser::getSegmentType(const LineSegment& segment) const
{
    return segment.isFastTraverse() + segment.isZMovement() * 2;
}

QList<LineSegment>& GCodeViewParser::getSimplifiedLines(double simplifyPrecision)
{
    // Check if we need to rebuild simplified lines
    if (m_simplifiedLinesReady && m_lastSimplifyPrecision == simplifyPrecision) {
        return m_simplifiedLines;
    }

    m_simplifiedLines.clear();
    m_lastSimplifyPrecision = simplifyPrecision;

    if (m_lines.isEmpty()) {
        m_simplifiedLinesReady = true;
        return m_simplifiedLines;
    }

    // Simplify geometry
    int vertexCount = 0;
    for (int i = 0; i < m_lines.count(); i++) {
        int j = i;

        if (i < m_lines.count() - 1) {
            QVector3D start = m_lines[i].getEnd() - m_lines[i].getStart();
            QVector3D next;
            double length = start.length();
            bool straight = false;

            do {
                i++;
                if (i < m_lines.count() - 1) {
                    next = m_lines[i].getEnd() - m_lines[i].getStart();
                    length += next.length();
                }
            } while ((length < simplifyPrecision || straight) && i < m_lines.count()
                     && getSegmentType(m_lines[i]) == getSegmentType(m_lines[j]));
            i--;
        }

        // Create simplified segment from j to i
        float segmentLen = (m_lines[i].getEnd() - m_lines[j].getStart()).length();
        if (segmentLen > 0.0001) {
            LineSegment simplifiedSegment(m_lines[j].getStart(), m_lines[i].getEnd(), m_lines[j].getLineNumber());
            simplifiedSegment.setIsArc(m_lines[j].isArc());
            simplifiedSegment.setIsClockwise(m_lines[j].isClockwise());
            simplifiedSegment.setPlane(m_lines[j].plane());
            simplifiedSegment.setIsFastTraverse(m_lines[j].isFastTraverse());
            simplifiedSegment.setIsZMovement(m_lines[j].isZMovement());
            simplifiedSegment.setIsMetric(m_lines[j].isMetric());
            simplifiedSegment.setIsAbsolute(m_lines[j].isAbsolute());
            simplifiedSegment.setSpeed(m_lines[j].getSpeed());
            simplifiedSegment.setSpindleSpeed(m_lines[j].getSpindleSpeed());
            simplifiedSegment.setDwell(m_lines[j].getDwell());
            simplifiedSegment.setVertexIndex(vertexCount);

            m_simplifiedLines.append(simplifiedSegment);
            vertexCount++;
        }
    }

    m_simplifiedLinesReady = true;
    return m_simplifiedLines;
}
