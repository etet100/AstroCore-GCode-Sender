// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "arcstolines.h"
#include "core/gcode/gcode.h"
#include "core/gcode/parser/gcodeparser.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>

static const double MIN_ARC_SEGMENT_LENGTH = 0.1;

ArcsToLines::ArcsToLines(double arcPrecision, bool arcDegreeMode)
    : AbstractConverter()
    , m_arcPrecision(arcPrecision)
    , m_arcDegreeMode(arcDegreeMode)
{
}

bool ArcsToLines::convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser)
{
    if (!item.isArc() || !parser) {
        return false;
    }

    QVector3D startPoint = *parser->getCurrentPoint();

    // addCommand() adds the arc into the parser's pushed state.
    // All data must be saved before popState() deletes ps.
    PointSegment *ps = parser->addCommand(item);
    if (!ps || !ps->isArc()) {
        return false;
    }

    QVector3D endPoint  = *ps->point();
    QVector3D center    = *ps->center();
    double    radius    = ps->getRadius();
    bool      clockwise = ps->isClockwise();
    PointSegment::planes plane = ps->plane();

    QList<QVector3D> arcPoints = GcodePreprocessorUtils::generatePointsAlongArcBDring(
        plane, startPoint, endPoint, center, clockwise, radius,
        MIN_ARC_SEGMENT_LENGTH, m_arcPrecision, m_arcDegreeMode
    );

    // generatePointsAlongArcBDring may or may not include startPoint as first element.
    // Build a deduped list starting from startPoint.
    QList<QVector3D> points;
    points.append(startPoint);
    for (const QVector3D &p : arcPoints) {
        if ((p - points.last()).lengthSquared() > 1e-12) {
            points.append(p);
        }
    }

    if (points.size() < 2) {
        return false;
    }

    item.line       = buildG1Line(points[0], points[1], item, true);
    item.args       = GcodePreprocessorUtils::splitCommand(item.line);
    item.group      = GCodeItemGroup::Movement;
    item.isMovement = true;

    // Insert remaining segments. Each insert shifts subsequent items right,
    // so inserting at currentIndex+1, +2, ... keeps the correct order.
    for (int i = 2; i < points.size(); ++i) {
        GCodeItem seg;
        seg.line          = buildG1Line(points[i - 1], points[i], item, false);
        seg.args          = GcodePreprocessorUtils::splitCommand(seg.line);
        seg.isMovement    = true;
        seg.state         = GCodeItem::InQueue;
        seg.group         = GCodeItemGroup::Movement;
        seg.commandNumber = item.commandNumber;

        gcode->insert(currentIndex + i - 1, seg);
    }

    return true;
}

QString ArcsToLines::buildG1Line(const QVector3D &start, const QVector3D &end,
                                  const GCodeItem &source, bool includeF)
{
    QString line = GcodePreprocessorUtils::generateG1FromPoints(start, end, true, 3);

    if (includeF) {
        static QRegularExpression feedRegex(
            "F([0-9.]+)", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = feedRegex.match(source.line);
        if (match.hasMatch()) {
            line += match.captured(0);
        }
    }

    return line;
}
