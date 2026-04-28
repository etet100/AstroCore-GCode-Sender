// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#include "simplifyviewtransform.h"

namespace {

int segmentType(const LineSegment& s)
{
    return s.isFastTraverse() + s.isZMovement() * 2;
}

bool areCollinear(const QVector3D& refDir, const LineSegment& s, float tolerance)
{
    QVector3D dir = s.getEnd() - s.getStart();
    float len = dir.length();
    if (len < 1e-7f) {
        return true;
    }

    QVector3D cross = QVector3D::crossProduct(refDir, dir / len);

    return cross.lengthSquared() < tolerance * tolerance;
}

float perpendicularDistance(const QVector3D& p, const QVector3D& lineStart, const QVector3D& lineEnd)
{
    QVector3D chord = lineEnd - lineStart;
    float chordLen = chord.length();
    if (chordLen < 1e-7f) {
        return (p - lineStart).length();
    }

    return QVector3D::crossProduct(p - lineStart, chord / chordLen).length();
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
    for (int i = 0; i < input.count(); i++) {
        int j = i;

        if (i < input.count() - 1) {
            QVector3D refVec = input[j].getEnd() - input[j].getStart();
            float refLen = refVec.length();
            QVector3D refDir = (refLen > 1e-7f) ? (refVec / refLen) : QVector3D();
            bool hasRefDir = refLen > 1e-7f && !input[j].isArc();

            while (i < input.count() - 1
                   && segmentType(input[i + 1]) == segmentType(input[j])) {
                if (input[i + 1].isArc() || !hasRefDir) {
                    break;
                }

                bool collinear = areCollinear(refDir, input[i + 1], (float)m_collinearTolerance);
                bool withinDeviation = m_precision > 0.0
                                       && perpendicularDistance(
                                              input[i].getEnd(),
                                              input[j].getStart(),
                                              input[i + 1].getEnd()
                                          ) <= (float)m_precision;

                if (!collinear && !withinDeviation) {
                    break;
                }

                i++;
            }
        }

        float segmentLen = (input[i].getEnd() - input[j].getStart()).length();
        if (segmentLen > 0.0001f) {
            LineSegment simplified(input[j].getStart(), input[i].getEnd(), input[j].getLineNumber());
            simplified.setIsArc(input[j].isArc());
            simplified.setIsClockwise(input[j].isClockwise());
            simplified.setPlane(input[j].plane());
            simplified.setIsFastTraverse(input[j].isFastTraverse());
            simplified.setIsZMovement(input[j].isZMovement());
            simplified.setIsMetric(input[j].isMetric());
            simplified.setIsAbsolute(input[j].isAbsolute());
            simplified.setSpeed(input[j].getSpeed());
            simplified.setSpindleSpeed(input[j].getSpindleSpeed());
            simplified.setDwell(input[j].getDwell());
            simplified.setVertexIndex(vertexCount);

            result.append(simplified);
            vertexCount++;
        }
    }

    return result;
}
