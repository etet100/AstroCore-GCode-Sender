// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#include "simplifyviewtransform.h"

namespace {

int segmentType(const LineSegment& s)
{
    return s.isFastTraverse() + s.isZMovement() * 2;
}

}

SimplifyViewTransform::SimplifyViewTransform(double precision)
    : m_precision(precision)
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
            QVector3D start = input[i].getEnd() - input[i].getStart();
            QVector3D next;
            double length = start.length();
            bool straight = false;

            do {
                i++;
                if (i < input.count() - 1) {
                    next = input[i].getEnd() - input[i].getStart();
                    length += next.length();
                }
            } while ((length < m_precision || straight) && i < input.count()
                     && segmentType(input[i]) == segmentType(input[j]));
            i--;
        }

        float segmentLen = (input[i].getEnd() - input[j].getStart()).length();
        if (segmentLen > 0.0001) {
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
