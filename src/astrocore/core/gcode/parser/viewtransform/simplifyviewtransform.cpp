// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#include "simplifyviewtransform.h"

#include <QPair>
#include <QtGlobal>
#include <QVector>

namespace {

constexpr double PointEpsilon = 1e-7;
constexpr double SegmentEpsilon = 0.0001;

QVector3D pointAt(const QList<LineSegment>& input, int runStart, int pointIndex)
{
    if (pointIndex == 0) {
        return input[runStart].getStart();
    }

    return input[runStart + pointIndex - 1].getEnd();
}

bool sameDouble(double a, double b)
{
    return qAbs(a - b) <= 1e-9;
}

bool canMerge(const LineSegment& a, const LineSegment& b)
{
    return (a.getEnd() - b.getStart()).lengthSquared() <= SegmentEpsilon * SegmentEpsilon
           && !a.isArc()
           && !b.isArc()
           && a.isFastTraverse() == b.isFastTraverse()
           && a.isZMovement() == b.isZMovement()
           && a.isMetric() == b.isMetric()
           && a.isAbsolute() == b.isAbsolute()
           && a.isHightlight() == b.isHightlight()
           && a.getToolhead() == b.getToolhead()
           && sameDouble(a.getSpeed(), b.getSpeed())
           && sameDouble(a.getSpindleSpeed(), b.getSpindleSpeed())
           && sameDouble(a.getDwell(), b.getDwell());
}

double perpendicularDistanceSquared(const QVector3D& p, const QVector3D& lineStart, const QVector3D& lineEnd)
{
    QVector3D chord = lineEnd - lineStart;
    double chordLenSquared = chord.lengthSquared();
    if (chordLenSquared < PointEpsilon * PointEpsilon) {
        return (p - lineStart).lengthSquared();
    }

    double projection = QVector3D::dotProduct(p - lineStart, chord) / chordLenSquared;
    if (projection <= 0.0) {
        return (p - lineStart).lengthSquared();
    }

    if (projection >= 1.0) {
        return (p - lineEnd).lengthSquared();
    }

    QVector3D projected = lineStart + chord * projection;

    return (p - projected).lengthSquared();
}

LineSegment makeSegmentLike(const LineSegment& source,
                            const QVector3D& start,
                            const QVector3D& end,
                            int vertexIndex)
{
    LineSegment segment(start, end, source.getLineNumber());
    segment.setToolHead(source.getToolhead());
    segment.setSpeed(source.getSpeed());
    segment.setIsZMovement(source.isZMovement());
    segment.setIsArc(source.isArc());
    segment.setIsClockwise(source.isClockwise());
    segment.setPlane(source.plane());
    segment.setIsFastTraverse(source.isFastTraverse());
    segment.setDrawn(source.drawn());
    segment.setIsMetric(source.isMetric());
    segment.setIsAbsolute(source.isAbsolute());
    segment.setIsHightlight(source.isHightlight());
    segment.setSpindleSpeed(source.getSpindleSpeed());
    segment.setDwell(source.getDwell());
    segment.setVertexIndex(vertexIndex);

    return segment;
}

void appendSegmentLike(const LineSegment& source,
                       const QVector3D& start,
                       const QVector3D& end,
                       QList<LineSegment>& result,
                       int& vertexCount)
{
    if ((end - start).length() <= SegmentEpsilon) {
        return;
    }

    result.append(makeSegmentLike(source, start, end, vertexCount));
    vertexCount++;
}

void appendSimplifiedRun(const QList<LineSegment>& input,
                         int runStart,
                         int runEnd,
                         double tolerance,
                         QList<LineSegment>& result,
                         int& vertexCount)
{
    int segmentCount = runEnd - runStart;
    if (segmentCount <= 0) {
        return;
    }

    if (segmentCount == 1) {
        appendSegmentLike(input[runStart],
                          input[runStart].getStart(),
                          input[runStart].getEnd(),
                          result,
                          vertexCount);

        return;
    }

    QVector<char> keep(segmentCount + 1, false);
    keep[0] = true;
    keep[segmentCount] = true;

    QVector<QPair<int, int>> stack;
    stack.reserve(64);
    stack.append(QPair<int, int>(0, segmentCount));

    double toleranceSquared = tolerance * tolerance;
    while (!stack.isEmpty()) {
        QPair<int, int> range = stack.takeLast();
        int first = range.first;
        int last = range.second;
        if (last <= first + 1) {
            continue;
        }

        QVector3D lineStart = pointAt(input, runStart, first);
        QVector3D lineEnd = pointAt(input, runStart, last);
        double maxDistanceSquared = -1.0;
        int maxIndex = -1;

        for (int i = first + 1; i < last; i++) {
            double distanceSquared = perpendicularDistanceSquared(pointAt(input, runStart, i), lineStart, lineEnd);
            if (distanceSquared > maxDistanceSquared) {
                maxDistanceSquared = distanceSquared;
                maxIndex = i;
            }
        }

        if (maxDistanceSquared > toleranceSquared && maxIndex > first && maxIndex < last) {
            keep[maxIndex] = true;
            stack.append(QPair<int, int>(first, maxIndex));
            stack.append(QPair<int, int>(maxIndex, last));
        }
    }

    int previousPoint = 0;
    for (int i = 1; i <= segmentCount; i++) {
        if (!keep[i]) {
            continue;
        }

        appendSegmentLike(input[runStart + previousPoint],
                          pointAt(input, runStart, previousPoint),
                          pointAt(input, runStart, i),
                          result,
                          vertexCount);
        previousPoint = i;
    }
}

}

SimplifyViewTransform::SimplifyViewTransform(double precision, double collinearTolerance)
    : m_precision(precision)
    , m_collinearTolerance(collinearTolerance)
{
}

QList<LineSegment> SimplifyViewTransform::apply(const QList<LineSegment>& input) const
{
    QList<LineSegment> result;

    if (input.isEmpty()) {
        return result;
    }

    int vertexCount = 0;
    double tolerance = m_precision > 0.0 ? m_precision : m_collinearTolerance;

    for (int i = 0; i < input.count();) {
        if (input[i].isArc()) {
            appendSegmentLike(input[i],
                              input[i].getStart(),
                              input[i].getEnd(),
                              result,
                              vertexCount);
            i++;

            continue;
        }

        int runStart = i;
        i++;

        while (i < input.count() && canMerge(input[i - 1], input[i])) {
            i++;
        }

        appendSimplifiedRun(input, runStart, i, tolerance, result, vertexCount);
    }

    return result;
}
