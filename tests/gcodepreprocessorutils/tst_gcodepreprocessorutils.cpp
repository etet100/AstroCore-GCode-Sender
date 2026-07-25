// Unit tests for GcodePreprocessorUtils — the static, dependency-free G-Code
// string and geometry helpers that every parser and converter builds on.
//
// A few tests deliberately pin down surprising-but-shipping behavior (nested
// comments, minus signs after a digit, degree-mode truncation). Those are
// marked as such so a future change does not "fix" them by accident.

#include <QtTest>
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include "core/gcode/gcodeitem.h"

namespace {

QStringList toList(const std::vector<std::string>& args)
{
    QStringList out;
    for (const std::string& a : args) {
        out << QString::fromStdString(a);
    }

    return out;
}

bool sameVec(const QVector3D& a, const QVector3D& b, float eps = 1e-3f)
{
    return (a - b).length() < eps;
}

} // namespace

class TstGcodePreprocessorUtils : public QObject
{
    Q_OBJECT

private slots:
    // parseLine / comment splitting
    void parseLineEmptyLines();
    void parseLineCommentOnly();
    void parseLineSplitsCommandAndComments();
    void parseLineUnclosedParen();

    // splitCommand
    void splitCommandTokenizes();
    void splitCommandIgnoresSpaces();
    void splitCommandDropsMinusAfterDigit();
    void splitCommandTruncatesOverlongToken();

    // coordinate / code extraction
    void parseCoordReturnsNaNWhenAbsent();
    void parseCodesKeepsDecimalSubtype();
    void parseGCodesDropsDecimalSubtype();

    // point updates
    void updatePointAbsolute();
    void updatePointIncremental();

    // text helpers
    void overrideSpeedScalesFeed();
    void removeCommentStripsAndUppercases();
    void removeCommentMishandlesNestedParens();
    void truncateDecimalsRoundsEveryNumber();
    void removeAllWhitespaceStripsAll();
    void generateG1FromPointsFormatting();

    // arc geometry
    void convertRToCenterQuarterCircle();
    void convertRToCenterInvalidRadiusYieldsNaN();
    void getAngleQuadrants();
    void getAngleDegenerateReturns270();
    void calculateSweepCases();
    void arcSegmentCountFollowsPrecision();
    void arcAutoRadiusUsesStartPoint();
    void arcExcludesStartPoint();
    void arcDegreeModeTruncates();
    void arcNaNCenterReturnsEmpty();
    void arcRoundTripsThroughPlanes();
};

void TstGcodePreprocessorUtils::parseLineEmptyLines()
{
    QCOMPARE(GcodePreprocessorUtils::parseLine(QStringLiteral("")).state, GCodeItem::EmptyLine);
    QCOMPARE(GcodePreprocessorUtils::parseLine(QStringLiteral("   \t ")).state, GCodeItem::EmptyLine);
    // Empty comment body: nothing left to run and nothing to show.
    QCOMPARE(GcodePreprocessorUtils::parseLine(QStringLiteral("()")).state, GCodeItem::EmptyLine);
}

void TstGcodePreprocessorUtils::parseLineCommentOnly()
{
    const GCodeItem semi = GcodePreprocessorUtils::parseLine(QStringLiteral("; spindle warm-up"));
    QCOMPARE(semi.state, GCodeItem::Comment);
    QCOMPARE(semi.group, GCodeItemGroup::Comment);
    QCOMPARE(semi.comment, QString("spindle warm-up"));

    const GCodeItem paren = GcodePreprocessorUtils::parseLine(QStringLiteral("(spindle warm-up)"));
    QCOMPARE(paren.state, GCodeItem::Comment);
    QCOMPARE(paren.group, GCodeItemGroup::Comment);
    QCOMPARE(paren.comment, QString("spindle warm-up"));
}

void TstGcodePreprocessorUtils::parseLineSplitsCommandAndComments()
{
    const GCodeItem item = GcodePreprocessorUtils::parseLine(
        QStringLiteral("  g1 (slow) X10 (feed) Y20 ; done  "));

    // `line` keeps the original case, only outer whitespace is trimmed.
    QCOMPARE(item.line, QString("g1 (slow) X10 (feed) Y20 ; done"));
    // Every comment body is collected, in order, space separated.
    QCOMPARE(item.comment, QString("slow feed done"));
    QCOMPARE(toList(item.args), QStringList({ "G1", "X10", "Y20" }));
    QCOMPARE(item.state, GCodeItem::InQueue);
    // parseLine does not classify commands — the loader does that later.
    QCOMPARE(item.group, GCodeItemGroup::Unknown);
    QVERIFY(!item.isMovement);
}

void TstGcodePreprocessorUtils::parseLineUnclosedParen()
{
    const GCodeItem item = GcodePreprocessorUtils::parseLine(QStringLiteral("G1 X10 (oops"));

    QCOMPARE(item.comment, QString("oops"));
    QCOMPARE(toList(item.args), QStringList({ "G1", "X10" }));
}

void TstGcodePreprocessorUtils::splitCommandTokenizes()
{
    QCOMPARE(toList(GcodePreprocessorUtils::splitCommand(QStringLiteral("G1X10Y-5.5Z0"))),
             QStringList({ "G1", "X10", "Y-5.5", "Z0" }));
    // A minus directly after a letter starts a negative token, as in arc offsets.
    QCOMPARE(toList(GcodePreprocessorUtils::splitCommand(QStringLiteral("I-1J-2"))),
             QStringList({ "I-1", "J-2" }));
}

void TstGcodePreprocessorUtils::splitCommandIgnoresSpaces()
{
    QCOMPARE(toList(GcodePreprocessorUtils::splitCommand(QStringLiteral("G1 X10  Y20"))),
             QStringList({ "G1", "X10", "Y20" }));
}

void TstGcodePreprocessorUtils::splitCommandDropsMinusAfterDigit()
{
    // Known quirk: a minus following a digit (malformed G-Code) is silently
    // dropped instead of starting a new token.
    QCOMPARE(toList(GcodePreprocessorUtils::splitCommand(QStringLiteral("X1-2"))),
             QStringList({ "X1", "2" }));
}

void TstGcodePreprocessorUtils::splitCommandTruncatesOverlongToken()
{
    // The tokenizer uses a fixed 32-byte buffer and truncates instead of
    // overflowing it.
    const QString longToken = "X" + QString("1").repeated(40);
    const auto args = GcodePreprocessorUtils::splitCommand(longToken);

    QCOMPARE(args.size(), size_t(1));
    QCOMPARE(int(args[0].size()), 32);
}

void TstGcodePreprocessorUtils::parseCoordReturnsNaNWhenAbsent()
{
    const auto args = GcodePreprocessorUtils::splitCommand(QStringLiteral("G1 X10 Y-5 F100"));

    QCOMPARE(GcodePreprocessorUtils::parseCoord(args, 'X'), 10.0);
    QCOMPARE(GcodePreprocessorUtils::parseCoord(args, 'Y'), -5.0);
    QCOMPARE(GcodePreprocessorUtils::parseCoord(args, 'F'), 100.0);
    // Lower-case selectors are upper-cased before matching.
    QCOMPARE(GcodePreprocessorUtils::parseCoord(args, 'x'), 10.0);
    QVERIFY(qIsNaN(GcodePreprocessorUtils::parseCoord(args, 'Z')));
}

void TstGcodePreprocessorUtils::parseCodesKeepsDecimalSubtype()
{
    // parseCodes parses as float, so probe subtypes survive.
    const auto args = GcodePreprocessorUtils::splitCommand(QStringLiteral("G38.2 Z-10"));
    const QList<float> codes = GcodePreprocessorUtils::parseCodes(args, 'G');

    QCOMPARE(codes.count(), 1);
    QCOMPARE(codes.first(), 38.2f);
}

void TstGcodePreprocessorUtils::parseGCodesDropsDecimalSubtype()
{
    // Known limitation: the regex-based variant returns ints, so "G38.2"
    // collapses to 38. Leading zeros are stripped.
    QCOMPARE(GcodePreprocessorUtils::parseGCodes("G38.2"), QList<int>({ 38 }));
    QCOMPARE(GcodePreprocessorUtils::parseGCodes("G01 G00"), QList<int>({ 1, 0 }));
    QCOMPARE(GcodePreprocessorUtils::parseMCodes("M03"), QList<int>({ 3 }));
}

void TstGcodePreprocessorUtils::updatePointAbsolute()
{
    const QVector3D initial(1, 2, 3);
    const QVector3D result =
        GcodePreprocessorUtils::updatePointWithCommand("X10 Z-1", initial, true);

    // Axes absent from the command keep their previous value.
    QVERIFY(sameVec(result, QVector3D(10, 2, -1)));
}

void TstGcodePreprocessorUtils::updatePointIncremental()
{
    const QVector3D initial(1, 2, 3);
    const QVector3D result =
        GcodePreprocessorUtils::updatePointWithCommand("X10 Z-1", initial, false);

    QVERIFY(sameVec(result, QVector3D(11, 2, 2)));
}

void TstGcodePreprocessorUtils::overrideSpeedScalesFeed()
{
    double original = 0.0;
    const QString result =
        GcodePreprocessorUtils::overrideSpeed("G1 X10 F100", 50, &original);

    QCOMPARE(result, QString("G1 X10 F50"));
    QCOMPARE(original, 100.0);
    // No F word: the line is returned untouched.
    QCOMPARE(GcodePreprocessorUtils::overrideSpeed("G0 X10", 50), QString("G0 X10"));
}

void TstGcodePreprocessorUtils::removeCommentStripsAndUppercases()
{
    QCOMPARE(GcodePreprocessorUtils::removeComment(QStringLiteral("g1 x10 (note) ; tail")),
             QString("G1 X10"));
    QCOMPARE(GcodePreprocessorUtils::removeComment(QStringLiteral("G1 X10 (unclosed")),
             QString("G1 X10"));
}

void TstGcodePreprocessorUtils::removeCommentMishandlesNestedParens()
{
    // Known limitation: the scan stops at the first ')', so a nested comment
    // leaves the tail behind. Documented rather than relied upon.
    QCOMPARE(GcodePreprocessorUtils::removeComment(QStringLiteral("G1 (outer (inner) x) Y2")),
             QString("G1  X) Y2"));
}

void TstGcodePreprocessorUtils::truncateDecimalsRoundsEveryNumber()
{
    QCOMPARE(GcodePreprocessorUtils::truncateDecimals(2, "G1 X10.5555 Y2.1234"),
             QString("G1 X10.56 Y2.12"));
    QCOMPARE(GcodePreprocessorUtils::truncateDecimals(0, "X1.6"), QString("X2"));
}

void TstGcodePreprocessorUtils::removeAllWhitespaceStripsAll()
{
    QCOMPARE(GcodePreprocessorUtils::removeAllWhitespace("G1 X10\tY20"),
             QString("G1X10Y20"));
}

void TstGcodePreprocessorUtils::generateG1FromPointsFormatting()
{
    // Output has no spaces and always carries all three axes.
    QCOMPARE(GcodePreprocessorUtils::generateG1FromPoints(
                 QVector3D(0, 0, 0), QVector3D(10, 5, -1), true, 3),
             QString("G1X10.000Y5.000Z-1.000"));
    // Incremental mode emits the delta from start.
    QCOMPARE(GcodePreprocessorUtils::generateG1FromPoints(
                 QVector3D(1, 2, 3), QVector3D(10, 5, -1), false, 3),
             QString("G1X9.000Y3.000Z-4.000"));
}

void TstGcodePreprocessorUtils::convertRToCenterQuarterCircle()
{
    // Quarter circle from (0,0) to (10,10) with R10: the centre sits at (0,10).
    const QVector3D center = GcodePreprocessorUtils::convertRToCenter(
        QVector3D(0, 0, 0), QVector3D(10, 10, 0), 10.0, false, false);

    QVERIFY(sameVec(center, QVector3D(0, 10, 0)));
}

void TstGcodePreprocessorUtils::convertRToCenterInvalidRadiusYieldsNaN()
{
    // R is too small to span the chord — the result is NaN, only logged.
    const QVector3D center = GcodePreprocessorUtils::convertRToCenter(
        QVector3D(0, 0, 0), QVector3D(10, 0, 0), 1.0, false, false);

    QVERIFY(qIsNaN(center.x()));
    QVERIFY(qIsNaN(center.y()));
}

void TstGcodePreprocessorUtils::getAngleQuadrants()
{
    const QVector3D o(0, 0, 0);

    QCOMPARE(GcodePreprocessorUtils::getAngle(o, QVector3D(1, 0, 0)), 0.0);
    QCOMPARE(GcodePreprocessorUtils::getAngle(o, QVector3D(0, 1, 0)), M_PI / 2.0);
    QCOMPARE(GcodePreprocessorUtils::getAngle(o, QVector3D(-1, 0, 0)), M_PI);
    QCOMPARE(GcodePreprocessorUtils::getAngle(o, QVector3D(0, -1, 0)), M_PI * 3.0 / 2.0);
}

void TstGcodePreprocessorUtils::getAngleDegenerateReturns270()
{
    // Start == centre has no defined angle; the code falls through to 270 deg.
    QCOMPARE(GcodePreprocessorUtils::getAngle(QVector3D(0, 0, 0), QVector3D(0, 0, 0)),
             M_PI * 3.0 / 2.0);
}

void TstGcodePreprocessorUtils::calculateSweepCases()
{
    // Equal angles mean a full circle, not a zero-length arc.
    QCOMPARE(GcodePreprocessorUtils::calculateSweep(0.0, 0.0, false), M_PI * 2);
    // Plain CCW quarter turn.
    QCOMPARE(GcodePreprocessorUtils::calculateSweep(0.0, M_PI / 2, false), M_PI / 2);
    // CCW wrapping past zero.
    QCOMPARE(GcodePreprocessorUtils::calculateSweep(M_PI * 3 / 2, M_PI / 2, false), M_PI);
    // A CW arc ending at 0 is remapped to 2*PI first.
    QCOMPARE(GcodePreprocessorUtils::calculateSweep(M_PI / 2, 0.0, true), M_PI / 2);
}

void TstGcodePreprocessorUtils::arcSegmentCountFollowsPrecision()
{
    // Quarter circle, radius 10 -> arc length ~15.708 mm.
    const QVector3D start(10, 0, 0);
    const QVector3D end(0, 10, 0);
    const QVector3D center(0, 0, 0);

    const auto coarse = GcodePreprocessorUtils::generatePointsAlongArcBDring(
        PointSegment::XY, start, end, center, false, 10.0, 0.1, 1.0, false);
    QCOMPARE(coarse.count(), 16);   // ceil(15.708 / 1.0)

    const auto fine = GcodePreprocessorUtils::generatePointsAlongArcBDring(
        PointSegment::XY, start, end, center, false, 10.0, 0.1, 0.5, false);
    QCOMPARE(fine.count(), 32);     // ceil(15.708 / 0.5)
}

void TstGcodePreprocessorUtils::arcAutoRadiusUsesStartPoint()
{
    // Passing R = 0 asks for the radius to be derived from the start point.
    // Regression guard: the derivation must use start.x/start.y, not a mix of
    // start.x and end.y, which inflated the radius whenever start.y != end.y.
    const QVector3D start(10, 0, 0);
    const QVector3D end(0, 10, 0);
    const QVector3D center(0, 0, 0);

    const auto points = GcodePreprocessorUtils::generatePointsAlongArcBDring(
        PointSegment::XY, start, end, center, false, 0.0, 0.1, 1.0, false);

    QCOMPARE(points.count(), 16);
    for (const QVector3D& p : points) {
        QVERIFY2(qAbs((p - center).length() - 10.0f) < 1e-3f,
                 qPrintable(QString("point %1,%2 is not on the r=10 arc")
                                .arg(p.x()).arg(p.y())));
    }
}

void TstGcodePreprocessorUtils::arcExcludesStartPoint()
{
    const QVector3D start(10, 0, 0);
    const QVector3D end(0, 10, 0);
    const QVector3D center(0, 0, 0);

    const auto points = GcodePreprocessorUtils::generatePointsAlongArcBDring(
        PointSegment::XY, start, end, center, false, 10.0, 0.1, 1.0, false);

    QVERIFY(!points.isEmpty());
    // Despite the doc comment, the start point is never emitted — callers must
    // prepend it themselves.
    QVERIFY(!sameVec(points.first(), start));
    QVERIFY(sameVec(points.last(), end));
}

void TstGcodePreprocessorUtils::arcDegreeModeTruncates()
{
    // Degree mode divides the sweep by the step and truncates (no ceil like
    // millimetre mode): 90 deg / 7 deg = 12.86 -> 12 segments.
    const auto points = GcodePreprocessorUtils::generatePointsAlongArcBDring(
        PointSegment::XY, QVector3D(10, 0, 0), QVector3D(0, 10, 0), QVector3D(0, 0, 0),
        false, 10.0, 0.1, 7.0, true);

    QCOMPARE(points.count(), 12);
}

void TstGcodePreprocessorUtils::arcNaNCenterReturnsEmpty()
{
    const auto points = GcodePreprocessorUtils::generatePointsAlongArcBDring(
        PointSegment::XY, QVector3D(10, 0, 0), QVector3D(0, 10, 0),
        QVector3D(qQNaN(), 0, 0), false, 10.0, 0.1, 1.0, false);

    QVERIFY(points.isEmpty());
}

void TstGcodePreprocessorUtils::arcRoundTripsThroughPlanes()
{
    // The generator rotates the arc into XY and back out again. For every plane
    // the last point must land exactly on `end` and all points stay on the
    // circle around `center`.
    struct Case {
        PointSegment::planes plane;
        QVector3D start;
        QVector3D end;
    };

    const QVector3D center(0, 0, 0);
    const QList<Case> cases = {
        { PointSegment::XY, QVector3D(10, 0, 0), QVector3D(0, 10, 0) },
        { PointSegment::ZX, QVector3D(10, 0, 0), QVector3D(0, 0, 10) },
        { PointSegment::YZ, QVector3D(0, 10, 0), QVector3D(0, 0, 10) },
    };

    for (const Case& c : cases) {
        const auto points = GcodePreprocessorUtils::generatePointsAlongArcBDring(
            c.plane, c.start, c.end, center, false, 10.0, 0.1, 1.0, false);

        QVERIFY(!points.isEmpty());
        QVERIFY2(sameVec(points.last(), c.end),
                 qPrintable(QString("plane %1 did not round-trip the end point")
                                .arg(int(c.plane))));
        for (const QVector3D& p : points) {
            QVERIFY2(qAbs((p - center).length() - 10.0f) < 1e-3f,
                     qPrintable(QString("plane %1 produced an off-circle point")
                                    .arg(int(c.plane))));
        }
    }
}

QTEST_GUILESS_MAIN(TstGcodePreprocessorUtils)
#include "tst_gcodepreprocessorutils.moc"
