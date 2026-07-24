// Unit tests for GCodeCursor — the execution cursor over a GCodeProgram.
// Header-only, no QObject/signals, so all queries are asserted directly.

#include <QtTest>
#include "core/gcode/gcodecursor.h"
#include "core/gcode/gcodeprogram.h"

namespace {

GCodeItem makeItem(const QString& line, int16_t overlayId = 0)
{
    GCodeItem item;
    item.line = line;
    item.overlayId = overlayId;

    return item;
}

} // namespace

class TstGCodeCursor : public QObject
{
    Q_OBJECT

private slots:
    void advanceStepsAndSetsIterationStarted();
    void positionQueriesAgainstCount();
    void emptyProgramQueries();
    void processedIndexHelpers();
    void overlayIdAtBounds();
};

void TstGCodeCursor::advanceStepsAndSetsIterationStarted()
{
    GCodeProgram p;
    p.append(makeItem("A"));
    p.append(makeItem("B"));
    GCodeCursor c(p);
    c.reset();

    QCOMPARE(c.commandIndex(), 0);
    QVERIFY(!c.iterationStarted());

    c.advance();
    QCOMPARE(c.commandIndex(), 1);
    QVERIFY(c.iterationStarted());
}

void TstGCodeCursor::positionQueriesAgainstCount()
{
    GCodeProgram p;
    p.append(makeItem("A"));
    p.append(makeItem("B"));
    GCodeCursor c(p);
    c.reset();

    QVERIFY(c.hasMoreCommands());
    QVERIFY(!c.isLastCommand());
    QCOMPARE(c.lastCommandIndex(), 1);

    c.advance();
    QVERIFY(c.isLastCommand());
    QVERIFY(c.hasMoreCommands());
    QVERIFY(!c.noMoreCommands());

    c.advance();
    QVERIFY(!c.hasMoreCommands());
    QVERIFY(c.noMoreCommands());
}

void TstGCodeCursor::emptyProgramQueries()
{
    GCodeProgram p;
    GCodeCursor c(p);
    c.reset();

    QVERIFY(!c.hasMoreCommands());
    QVERIFY(c.noMoreCommands());
    QVERIFY(!c.isLastCommand());
    QCOMPARE(c.lastCommandIndex(), -1);
}

void TstGCodeCursor::processedIndexHelpers()
{
    GCodeProgram p;
    p.append(makeItem("A"));
    p.append(makeItem("B"));
    GCodeCursor c(p);
    c.reset();

    QCOMPARE(c.processedCommandIndex(), 0);
    c.setProcessedIndex(1);
    QCOMPARE(c.processedCommandIndex(), 1);
    QVERIFY(c.isLastCommandProcessed());

    c.resetProcessed(0);
    QCOMPARE(c.processedCommandIndex(), 0);
    QVERIFY(!c.isLastCommandProcessed());
}

void TstGCodeCursor::overlayIdAtBounds()
{
    GCodeProgram p;
    p.append(makeItem("A"));
    p.insertOverlay("o", { makeItem("M3") }, 1); // overlay item at index 1
    GCodeCursor c(p);

    QCOMPARE(c.overlayIdAt(0), 0);   // main
    QVERIFY(c.overlayIdAt(1) > 0);   // overlay
    QCOMPARE(c.overlayIdAt(-1), 0);  // out of range -> 0
    QCOMPARE(c.overlayIdAt(99), 0);  // out of range -> 0
}

QTEST_APPLESS_MAIN(TstGCodeCursor)
#include "tst_gcodecursor.moc"
