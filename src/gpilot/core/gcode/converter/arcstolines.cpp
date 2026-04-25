// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "arcstolines.h"
#include "core/gcode/parser/gcodeparser.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>

static const double MIN_ARC_SEGMENT_LENGTH = 0.1;

QString ArcsToLines::parameterSchema()
{
    return QStringLiteral(R"JSON({
  "title": "Arcs to lines",
  "description": "Converts G2/G3 arc movements into chains of G1 linear segments. Useful for controllers with limited or buggy arc support.",
  "image": ":/images/converters/arcstolines.svg",
  "fields": [
    {
      "name": "arcPrecision",
      "label": "Arc precision",
      "type": "float",
      "min": 0.001,
      "max": 10.0,
      "default": 0.1,
      "description": "Max chord deviation in mm, or segment length in degrees when 'Degree mode' is enabled."
    },
    {
      "name": "arcDegreeMode",
      "label": "Degree mode",
      "type": "bool",
      "default": false,
      "description": "When on, 'Arc precision' is interpreted as degrees per segment instead of chord deviation in mm."
    }
  ]
})JSON");
}

ArcsToLines::ArcsToLines(double arcPrecision, bool arcDegreeMode)
    : m_parser(new GcodeParser())
    , m_arcPrecision(arcPrecision)
    , m_arcDegreeMode(arcDegreeMode)
{
}

ArcsToLines::~ArcsToLines()
{
    delete m_parser;
}

void ArcsToLines::reset()
{
    m_parser->reset();
}

QList<GCodeItem> ArcsToLines::push(const GCodeItem &input)
{
    if (!input.isArc()) {
        m_parser->addCommand(input);

        return { input };
    }

    QVector3D startPoint = *m_parser->getCurrentPoint();

    // addCommand() commits the arc to the parser. All point segment data must
    // be read before any subsequent call invalidates the returned pointer.
    PointSegment *ps = m_parser->addCommand(input);
    if (!ps || !ps->isArc()) {
        return { input };
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
        return { input };
    }

    QList<GCodeItem> out;
    out.reserve(points.size() - 1);

    GCodeItem first = input;
    first.line       = buildG1Line(points[0], points[1], input, true);
    first.args       = GcodePreprocessorUtils::splitCommand(first.line);
    first.group      = GCodeItemGroup::Movement;
    first.isMovement = true;
    out << first;

    for (int i = 2; i < points.size(); ++i) {
        GCodeItem seg;
        seg.line          = buildG1Line(points[i - 1], points[i], input, false);
        seg.args          = GcodePreprocessorUtils::splitCommand(seg.line);
        seg.isMovement    = true;
        seg.state         = GCodeItem::InQueue;
        seg.group         = GCodeItemGroup::Movement;
        seg.commandNumber = input.commandNumber;
        seg.overlayId     = input.overlayId;
        out << seg;
    }

    return out;
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
