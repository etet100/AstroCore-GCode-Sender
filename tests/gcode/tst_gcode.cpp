// Unit tests for the GCode program model (core/gcode/gcode.h).
// Covers the pure logic: item parsing, execution cursor, response storage,
// overlays, structural edits and the coalesced view-update signals.

#include <QtTest>
#include "core/gcode/gcode.h"

namespace {

GCodeItem makeItem(const QString& line,
                   GCodeItemGroup group = GCodeItemGroup::Movement,
                   int16_t overlayId = 0)
{
    GCodeItem item;
    item.line = line;
    item.group = group;
    item.overlayId = overlayId;

    return item;
}

} // namespace

class TstGCode : public QObject
{
    Q_OBJECT

private slots:
    void init();

    // GCodeItem value logic
    void command_data();
    void command();
    void isArc();

    // Container basics
    void appendAssignsLineNumberAndCount();
    void clearResetsState();

    // Execution cursor
    void cursorAdvanceAndQueries();
    void resetRestoresStatesAndIndex();

    // Command state transitions
    void setCommandSentMarksSent();
    void setCommandResponseStoresStateAndResponse();
    void abortedAndSkipped();

    // Response storage rules
    void responseInfersOkFromProcessed();
    void responseStoresOnlyNonOk();

    // Navigation helpers bounds
    void lookAheadBehindAndGetLineBounds();

    // Structural edits keep mainCount and drop responses
    void insertRemoveEraseTrackMainCount();
    void structuralEditClearsResponses();
    void deleteLinesEmitsStructureChanged();
    void linesAsText();

    // Overlays
    void insertOverlayDoesNotAffectMainCount();
    void resetOverlaysRemovesOverlayItems();

    // Signal coalescing (uses the test seam)
    void coalescedLinesUpdatedFlushOnce();
    void lastSentCommandChangedOnFlush();
    void nameChangedOnlyWhenDifferent();
};

void TstGCode::init()
{
    // Nothing global; each test builds its own GCode. Auto-flush is disabled
    // per-instance inside the tests that inspect signals.
}

void TstGCode::command_data()
{
    QTest::addColumn<QString>("line");
    QTest::addColumn<QString>("expected");

    QTest::newRow("empty") << QString() << QString();
    QTest::newRow("uppercases") << "g1 x10" << "G1 X10";
    QTest::newRow("semicolon comment") << "G1 X10 ; go slow" << "G1 X10";
    QTest::newRow("paren comment") << "(setup) G0 X0" << "G0 X0";
    QTest::newRow("unterminated paren") << "G1 (oops X10" << "G1";
}

void TstGCode::command()
{
    QFETCH(QString, line);
    QFETCH(QString, expected);

    GCodeItem item = makeItem(line);

    QCOMPARE(item.command(), expected);
}

void TstGCode::isArc()
{
    QVERIFY(makeItem("G2 X1 Y1 I1", GCodeItemGroup::ArcMovement).isArc());
    QVERIFY(!makeItem("G1 X1", GCodeItemGroup::Movement).isArc());
}

void TstGCode::appendAssignsLineNumberAndCount()
{
    GCode g;
    g.setAutoFlushEnabled(false);

    QVERIFY(g.empty());
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");

    QCOMPARE(g.count(), 2);
    QVERIFY(!g.empty());
    // operator<< sets lineNumber to the 1-based position.
    QCOMPARE(g[0].lineNumber, 1);
    QCOMPARE(g[1].lineNumber, 2);
    QCOMPARE(g.mainCount(), 2);
}

void TstGCode::clearResetsState()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");

    g.clear();

    QVERIFY(g.empty());
    QCOMPARE(g.count(), 0);
    QCOMPARE(g.mainCount(), 0);
}

void TstGCode::cursorAdvanceAndQueries()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");
    g.reset();

    QCOMPARE(g.commandIndex(), 0);
    QVERIFY(g.hasMoreCommands());
    QVERIFY(!g.isLastCommand());
    QCOMPARE(g.lastCommandIndex(), 1);

    g.advanceCommandIndex();
    QCOMPARE(g.commandIndex(), 1);
    QVERIFY(g.isLastCommand());
    QVERIFY(g.hasMoreCommands());
    QVERIFY(!g.noMoreCommands());

    g.advanceCommandIndex();
    QCOMPARE(g.commandIndex(), 2);
    QVERIFY(!g.hasMoreCommands());
    QVERIFY(g.noMoreCommands());
}

void TstGCode::resetRestoresStatesAndIndex()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("(only comment)", GCodeItemGroup::Comment);
    g.reset();

    // Move cursor and mutate a state, then reset back.
    g.advanceCommandIndex();
    g.setCommandResponse(0, true, "ok");
    QCOMPARE(g[0].state, GCodeItem::Processed);

    g.reset();

    QCOMPARE(g.commandIndex(), 0);
    QCOMPARE(g.processedCommandIndex(), 0);
    QCOMPARE(g[0].state, GCodeItem::InQueue);
    // Comment lines are restored to the Comment state, not InQueue.
    QCOMPARE(g[1].state, GCodeItem::Comment);
}

void TstGCode::setCommandSentMarksSent()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g.reset();

    g.setCommandSent();

    QCOMPARE(g.current().state, GCodeItem::Sent);
}

void TstGCode::setCommandResponseStoresStateAndResponse()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");
    g.reset();

    g.setCommandResponse(1, false, "error:9");

    QCOMPARE(g[1].state, GCodeItem::Error);
    QCOMPARE(g.response(1), QString("error:9"));
    QCOMPARE(g.processedCommandIndex(), 1);
}

void TstGCode::abortedAndSkipped()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");
    g.reset();

    g.setCommandSkipped(); // acts on current (index 0)
    QCOMPARE(g[0].state, GCodeItem::Skipped);

    g.setCommandAborted(1);
    QCOMPARE(g[1].state, GCodeItem::Aborted);
}

void TstGCode::responseInfersOkFromProcessed()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g.reset();

    // No stored response, but a Processed line reports the implicit "ok".
    g.setCommandResponse(0, true, "ok");

    QCOMPARE(g.response(0), QString("ok"));
    // Nothing before processing.
    GCode empty;
    empty.setAutoFlushEnabled(false);
    empty << makeItem("G0 X0");
    QCOMPARE(empty.response(0), QString());
}

void TstGCode::responseStoresOnlyNonOk()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g.reset();

    g.setResponse(0, "ok");        // must not be stored
    QVERIFY(g.response(0).isEmpty());

    g.setResponse(0, "error:2");   // stored verbatim
    QCOMPARE(g.response(0), QString("error:2"));

    g.clearResponses();
    QVERIFY(g.response(0).isEmpty());
}

void TstGCode::lookAheadBehindAndGetLineBounds()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");
    g << makeItem("G1 X20");

    QCOMPARE(g.lookAhead(0, 1), &g[1]);
    QCOMPARE(g.lookAhead(0, 2), &g[2]);
    QVERIFY(g.lookAhead(0, 3) == nullptr);   // out of range
    QVERIFY(g.lookAhead(0, 0) == nullptr);   // offset must be >= 1

    QCOMPARE(g.lookBehind(2, 1), &g[1]);
    QVERIFY(g.lookBehind(0, 1) == nullptr);

    QCOMPARE(g.getLine(1), &g[1]);
    QVERIFY(g.getLine(-1) == nullptr);
    QVERIFY(g.getLine(3) == nullptr);
}

void TstGCode::insertRemoveEraseTrackMainCount()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");
    QCOMPARE(g.mainCount(), 2);

    g.insert(1, makeItem("G1 X5"));
    QCOMPARE(g.mainCount(), 3);
    QCOMPARE(g.count(), 3);

    g.removeAt(0);
    QCOMPARE(g.mainCount(), 2);

    g.erase(0, g.count()); // erase all remaining
    QCOMPARE(g.mainCount(), 0);
    QVERIFY(g.empty());
}

void TstGCode::structuralEditClearsResponses()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");
    g.reset();
    g.setResponse(1, "error:5");
    QCOMPARE(g.response(1), QString("error:5"));

    // insert() drops the response map (index keys are invalidated).
    g.insert(0, makeItem("G1 X1"));

    QVERIFY(g.response(2).isEmpty());
}

void TstGCode::deleteLinesEmitsStructureChanged()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");
    g << makeItem("G1 X20");

    QSignalSpy spy(&g, &GCode::structureChanged);
    g.deleteLines(0, 1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(g.count(), 1);
    QCOMPARE(g.mainCount(), 1);
    QCOMPARE(g[0].line, QString("G1 X20"));
}

void TstGCode::linesAsText()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");
    g << makeItem("G1 X20");

    QCOMPARE(g.linesAsText(0, 1), QString("G0 X0\nG1 X10"));
    // 'to' beyond the end is clamped.
    QCOMPARE(g.linesAsText(1, 99), QString("G1 X10\nG1 X20"));
}

void TstGCode::insertOverlayDoesNotAffectMainCount()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");
    g.reset(); // cursor at 0, iteration not started -> overlay inserts at front

    QList<GCodeItem> overlay { makeItem("M3"), makeItem("M5") };
    int id = g.insertOverlay("spindle", overlay);

    QVERIFY(id > 0);
    QCOMPARE(g.count(), 4);
    QCOMPARE(g.mainCount(), 2);          // overlay items are not "main"
    QVERIFY(g.isOverlayItem(0));
    QVERIFY(!g.isOverlayItem(2));

    const OverlayInfo* info = g.overlayInfo(id);
    QVERIFY(info != nullptr);
    QCOMPARE(info->count, 2);
    QCOMPARE(info->insertedAt, 0);

    QCOMPARE(g.insertOverlay("empty", {}), 0); // empty commands rejected
}

void TstGCode::resetOverlaysRemovesOverlayItems()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");
    g.reset();
    g.insertOverlay("spindle", { makeItem("M3"), makeItem("M5") });
    QCOMPARE(g.count(), 4);

    g.resetOverlays(0);

    QCOMPARE(g.count(), 2);
    QCOMPARE(g.mainCount(), 2);
    QVERIFY(g.overlayInfo(1) == nullptr);
    QVERIFY(!g.isOverlayItem(0));
}

void TstGCode::coalescedLinesUpdatedFlushOnce()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");
    g << makeItem("G1 X20");
    g.reset();
    // reset() queues a full-file update range (0..count-1); flush it so the
    // spy below only sees the operations under test.
    g.flushPendingUpdates();

    // Watch after the appends/reset so their emissions don't count.
    QSignalSpy spy(&g, &GCode::linesUpdated);

    g.setCommandSent();       // range {0}
    g.advanceCommandIndex();  // range widens to {0,1}
    g.setCommandResponse(0, true, "ok");

    QCOMPARE(spy.count(), 0);  // nothing delivered until flush
    g.flushPendingUpdates();
    QCOMPARE(spy.count(), 1);  // one coalesced emission

    const QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toInt(), 0);
    QCOMPARE(args.at(1).toInt(), 1);

    // Second flush with no pending work emits nothing.
    g.flushPendingUpdates();
    QCOMPARE(spy.count(), 0);
}

void TstGCode::lastSentCommandChangedOnFlush()
{
    GCode g;
    g.setAutoFlushEnabled(false);
    g << makeItem("G0 X0");
    g << makeItem("G1 X10");
    g.reset();

    QSignalSpy spy(&g, &GCode::lastSentCommandChanged);
    g.advanceCommandIndex();
    g.setCommandSent(); // marks index 1 as last sent

    QCOMPARE(spy.count(), 0);
    g.flushPendingUpdates();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 1);
}

void TstGCode::nameChangedOnlyWhenDifferent()
{
    GCode g;
    g.setAutoFlushEnabled(false);

    QSignalSpy spy(&g, &GCode::nameChanged);
    g.setName("prog.nc");
    g.setName("prog.nc"); // same value -> no signal

    QCOMPARE(spy.count(), 1);
    QCOMPARE(g.name(), QString("prog.nc"));
}

QTEST_GUILESS_MAIN(TstGCode)
#include "tst_gcode.moc"
