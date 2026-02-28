// Unit test for ShakingGCode converter
// Verifies basic functionality and edge cases

#include "core/gcode/converter/shakinggcode.h"
#include "core/gcode/gcode.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QTest>
#include <QDebug>

class TestShakingGCode : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testBasicSegmentation();
    void testShortLineNoSegmentation();
    void testNonMovementPassthrough();
    void testRandomOffsetRange();
    void testFeedRateVariation();
    void testReproducibleWithSeed();
    void testEmptyGCode();
    void testProgressSignals();

private:
    GCode* createSimpleGCode(const QString &line);
};

void TestShakingGCode::initTestCase()
{
    qDebug() << "Starting ShakingGCode tests";
}

void TestShakingGCode::cleanupTestCase()
{
    qDebug() << "ShakingGCode tests complete";
}

GCode* TestShakingGCode::createSimpleGCode(const QString &line)
{
    GCode* gcode = new GCode();
    GCodeItem item;
    item.line = line;
    item.args = GcodePreprocessorUtils::splitCommand(line);
    item.command = item.args.isEmpty() ? "" : item.args.first();
    item.isMovement = (item.command == "G0" || item.command == "G1");
    *gcode << item;
    return gcode;
}

void TestShakingGCode::testBasicSegmentation()
{
    qDebug() << "Test: Basic segmentation";

    // Create G-code with 50mm line
    GCode* original = createSimpleGCode("G1 X50.0 Y0.0 Z0.0 F1000");

    // Segment every 5mm should create ~10 segments
    ShakingGCode shaker(5.0, 0.0);  // No offset for predictability
    shaker.setFeedRateVariation(0.0);  // No feed variation
    shaker.setGCode(original);

    GCode* result = shaker.convertAll();

    QVERIFY(result != nullptr);
    QVERIFY(result->count() > 1);  // Should be segmented
    QVERIFY(result->count() >= 10);  // At least 10 segments for 50mm / 5mm

    qDebug() << "  Original: 1 line, Result:" << result->count() << "lines";

    delete original;
    delete result;
}

void TestShakingGCode::testShortLineNoSegmentation()
{
    qDebug() << "Test: Short line no segmentation";

    // Create G-code with 3mm line (shorter than segment length)
    GCode* original = createSimpleGCode("G1 X3.0 Y0.0 Z0.0 F1000");

    ShakingGCode shaker(5.0, 0.0);
    shaker.setFeedRateVariation(0.0);
    shaker.setGCode(original);

    GCode* result = shaker.convertAll();

    QVERIFY(result != nullptr);
    QCOMPARE(result->count(), 1);  // Should stay as 1 line

    qDebug() << "  Short line preserved as single line";

    delete original;
    delete result;
}

void TestShakingGCode::testNonMovementPassthrough()
{
    qDebug() << "Test: Non-movement commands pass through";

    GCode* original = new GCode();

    GCodeItem comment;
    comment.line = "; This is a comment";
    comment.isMovement = false;
    *original << comment;

    GCodeItem mCommand;
    mCommand.line = "M3 S1000";
    mCommand.command = "M3";
    mCommand.isMovement = false;
    *original << mCommand;

    ShakingGCode shaker(5.0, 1.0);
    shaker.setGCode(original);

    GCode* result = shaker.convertAll();

    QCOMPARE(result->count(), 2);  // Both should pass through unchanged
    QCOMPARE(result->at(0).line, QString("; This is a comment"));
    QCOMPARE(result->at(1).line, QString("M3 S1000"));

    qDebug() << "  Non-movement commands preserved";

    delete original;
    delete result;
}

void TestShakingGCode::testRandomOffsetRange()
{
    qDebug() << "Test: Random offset within range";

    GCode* original = createSimpleGCode("G1 X10.0 Y10.0 Z0.0 F1000");

    double maxOffset = 1.0;
    ShakingGCode shaker(2.0, maxOffset);
    shaker.setSeed(12345);  // Fixed seed
    shaker.setFeedRateVariation(0.0);
    shaker.setGCode(original);

    GCode* result = shaker.convertAll();

    // Parse result coordinates and check they're within offset range
    for (int i = 0; i < result->count(); i++) {
        QString line = result->at(i).line;

        // Extract X coordinate
        QRegularExpression xRegex("X([+-]?[0-9.]+)");
        QRegularExpressionMatch match = xRegex.match(line);
        if (match.hasMatch()) {
            double x = match.captured(1).toDouble();
            // X should be roughly on path (0 to 10) ± maxOffset
            QVERIFY(x >= -maxOffset);
            QVERIFY(x <= 10.0 + maxOffset);
        }
    }

    qDebug() << "  All offsets within ±" << maxOffset << "mm range";

    delete original;
    delete result;
}

void TestShakingGCode::testFeedRateVariation()
{
    qDebug() << "Test: Feed rate variation";

    GCode* original = createSimpleGCode("G1 X20.0 Y0.0 Z0.0 F1000");

    ShakingGCode shaker(5.0, 0.0);
    shaker.setFeedRateVariation(0.2);  // ±20%
    shaker.setSeed(12345);
    shaker.setGCode(original);

    GCode* result = shaker.convertAll();

    // Check that feed rates vary
    QRegularExpression fRegex("F([0-9.]+)");
    QList<double> feedRates;

    for (int i = 0; i < result->count(); i++) {
        QRegularExpressionMatch match = fRegex.match(result->at(i).line);
        if (match.hasMatch()) {
            double f = match.captured(1).toDouble();
            feedRates.append(f);

            // Should be within ±20% of 1000 (800 to 1200)
            QVERIFY(f >= 800.0);
            QVERIFY(f <= 1200.0);
        }
    }

    // Feed rates should vary (not all the same)
    bool hasVariation = false;
    for (int i = 1; i < feedRates.size(); i++) {
        if (qAbs(feedRates[i] - feedRates[i-1]) > 1.0) {
            hasVariation = true;
            break;
        }
    }
    QVERIFY(hasVariation);

    qDebug() << "  Feed rates vary within ±20% range";

    delete original;
    delete result;
}

void TestShakingGCode::testReproducibleWithSeed()
{
    qDebug() << "Test: Reproducible with seed";

    GCode* original1 = createSimpleGCode("G1 X30.0 Y0.0 Z0.0 F1000");
    GCode* original2 = createSimpleGCode("G1 X30.0 Y0.0 Z0.0 F1000");

    // First run
    ShakingGCode shaker1(5.0, 1.0);
    shaker1.setSeed(42);
    shaker1.setGCode(original1);
    GCode* result1 = shaker1.convertAll();

    // Second run with same seed
    ShakingGCode shaker2(5.0, 1.0);
    shaker2.setSeed(42);
    shaker2.setGCode(original2);
    GCode* result2 = shaker2.convertAll();

    // Results should be identical
    QCOMPARE(result1->count(), result2->count());

    for (int i = 0; i < result1->count(); i++) {
        QCOMPARE(result1->at(i).line, result2->at(i).line);
    }

    qDebug() << "  Same seed produces identical results";

    delete original1;
    delete original2;
    delete result1;
    delete result2;
}

void TestShakingGCode::testEmptyGCode()
{
    qDebug() << "Test: Empty G-code";

    GCode* original = new GCode();

    ShakingGCode shaker;
    shaker.setGCode(original);

    GCode* result = shaker.convertAll();

    QVERIFY(result != nullptr);
    QCOMPARE(result->count(), 0);

    qDebug() << "  Empty G-code handled correctly";

    delete original;
    delete result;
}

void TestShakingGCode::testProgressSignals()
{
    qDebug() << "Test: Progress signals";

    GCode* original = new GCode();
    for (int i = 0; i < 100; i++) {
        GCodeItem item;
        item.line = QString("G1 X%1 Y0 Z0").arg(i);
        item.command = "G1";
        item.isMovement = true;
        item.args = GcodePreprocessorUtils::splitCommand(item.line);
        *original << item;
    }

    ShakingGCode shaker(5.0, 0.5);
    shaker.setGCode(original);

    int progressCount = 0;
    QObject::connect(&shaker, &ShakingGCode::progressChanged,
        [&progressCount](int current, int total) {
            Q_UNUSED(current);
            Q_UNUSED(total);
            progressCount++;
        });

    GCode* result = shaker.convertAll();

    QVERIFY(progressCount > 0);  // Should emit progress

    qDebug() << "  Progress signals emitted:" << progressCount << "times";

    delete original;
    delete result;
}

QTEST_MAIN(TestShakingGCode)
#include "test_shakinggcode.moc"
