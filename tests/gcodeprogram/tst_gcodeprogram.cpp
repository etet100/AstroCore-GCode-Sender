// Unit tests for GCodeProgram — the pure data model (no QObject, no signals,
// no timer). Everything is asserted synchronously on plain state.

#include <QtTest>
#include "core/gcode/gcodeprogram.h"

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

class TstGCodeProgram : public QObject
{
    Q_OBJECT

private slots:
    void appendReturnsIndexAndTracksMainCount();
    void appendProgramAppendsAll();
    void insertRemoveEraseTrackMainCount();
    void structuralEditClearsResponses();
    void deleteLinesTracksMainCount();
    void replaceReportsSameLineCount();
    void linesAsTextClamps();
    void resetItemStatesKeepsComments();
    void overlayInsertAndReset();
    void responseInferenceRules();
    void navigationBounds();
    void hashTracksMainProgramOnly();
};

void TstGCodeProgram::appendReturnsIndexAndTracksMainCount()
{
    GCodeProgram p;

    QCOMPARE(p.append(makeItem("G0 X0")), 0);
    QCOMPARE(p.append(makeItem("G1 X10")), 1);
    QCOMPARE(p.count(), 2);
    QCOMPARE(p.mainCount(), 2);
    // append sets a 1-based lineNumber.
    QCOMPARE(p[0].lineNumber, 1);
    QCOMPARE(p[1].lineNumber, 2);
}

void TstGCodeProgram::appendProgramAppendsAll()
{
    GCodeProgram src;
    src.append(makeItem("G0 X0"));
    src.append(makeItem("G1 X10"));

    GCodeProgram dst;
    dst.append(makeItem("G90"));
    dst.appendProgram(src);

    QCOMPARE(dst.count(), 3);
    QCOMPARE(dst.mainCount(), 3);
    QCOMPARE(dst[2].line, QString("G1 X10"));
}

void TstGCodeProgram::insertRemoveEraseTrackMainCount()
{
    GCodeProgram p;
    p.append(makeItem("G0 X0"));
    p.append(makeItem("G1 X10"));

    p.insert(1, makeItem("G1 X5"));
    QCOMPARE(p.mainCount(), 3);
    QCOMPARE(p.count(), 3);
    QCOMPARE(p[1].line, QString("G1 X5"));

    p.removeAt(0);
    QCOMPARE(p.mainCount(), 2);

    p.erase(0, p.count());
    QCOMPARE(p.mainCount(), 0);
    QVERIFY(p.empty());
}

void TstGCodeProgram::structuralEditClearsResponses()
{
    GCodeProgram p;
    p.append(makeItem("G0 X0"));
    p.append(makeItem("G1 X10"));
    p.setResponse(1, "error:5");
    QCOMPARE(p.response(1), QString("error:5"));

    p.insert(0, makeItem("G1 X1")); // structural edit drops the response map
    QVERIFY(p.response(2).isEmpty());
}

void TstGCodeProgram::deleteLinesTracksMainCount()
{
    GCodeProgram p;
    p.append(makeItem("G0 X0"));
    p.append(makeItem("G1 X10"));
    p.append(makeItem("G1 X20"));

    p.deleteLines(0, 1);

    QCOMPARE(p.count(), 1);
    QCOMPARE(p.mainCount(), 1);
    QCOMPARE(p[0].line, QString("G1 X20"));
}

void TstGCodeProgram::replaceReportsSameLineCount()
{
    GCodeProgram p;
    p.append(makeItem("A"));
    p.append(makeItem("B"));
    p.append(makeItem("C"));

    // Same number of lines back -> true.
    QVERIFY(p.replace(1, 1, { makeItem("B2") }));
    QCOMPARE(p[1].line, QString("B2"));

    // Fewer/more lines -> false.
    QVERIFY(!p.replace(0, 1, { makeItem("X"), makeItem("Y"), makeItem("Z") }));
    QCOMPARE(p.count(), 4);
    QCOMPARE(p.mainCount(), 4);
}

void TstGCodeProgram::linesAsTextClamps()
{
    GCodeProgram p;
    p.append(makeItem("G0 X0"));
    p.append(makeItem("G1 X10"));
    p.append(makeItem("G1 X20"));

    QCOMPARE(p.linesAsText(0, 1), QString("G0 X0\nG1 X10"));
    QCOMPARE(p.linesAsText(1, 99), QString("G1 X10\nG1 X20")); // 'to' clamped
}

void TstGCodeProgram::resetItemStatesKeepsComments()
{
    GCodeProgram p;
    p.append(makeItem("G0 X0"));
    p.append(makeItem("(comment)", GCodeItemGroup::Comment));
    p[0].state = GCodeItem::Processed;

    p.resetItemStates();

    QCOMPARE(p[0].state, GCodeItem::InQueue);
    QCOMPARE(p[1].state, GCodeItem::Comment);
}

void TstGCodeProgram::overlayInsertAndReset()
{
    GCodeProgram p;
    p.append(makeItem("G0 X0"));
    p.append(makeItem("G1 X10"));

    int id = p.insertOverlay("spindle", { makeItem("M3"), makeItem("M5") }, 0);

    QVERIFY(id > 0);
    QCOMPARE(p.count(), 4);
    QCOMPARE(p.mainCount(), 2);          // overlay items are not "main"
    QVERIFY(p.isOverlayItem(0));
    QVERIFY(!p.isOverlayItem(2));

    const OverlayInfo* info = p.overlayInfo(id);
    QVERIFY(info != nullptr);
    QCOMPARE(info->count, 2);
    QCOMPARE(info->insertedAt, 0);

    QVERIFY(p.resetOverlays(0));         // returns true: it changed data
    QCOMPARE(p.count(), 2);
    QCOMPARE(p.mainCount(), 2);
    QVERIFY(p.overlayInfo(id) == nullptr);
    QVERIFY(!p.resetOverlays(0));        // nothing left to remove -> false
}

void TstGCodeProgram::responseInferenceRules()
{
    GCodeProgram p;
    p.append(makeItem("G0 X0"));

    QVERIFY(p.response(0).isEmpty());    // not processed, no stored response

    p[0].state = GCodeItem::Processed;
    QCOMPARE(p.response(0), QString("ok"));  // implicit ok for processed

    p.setResponse(0, "ok");              // "ok" is never stored
    QCOMPARE(p.response(0), QString("ok"));  // still inferred from state

    p.setResponse(0, "error:2");
    QCOMPARE(p.response(0), QString("error:2"));

    p.clearResponses();
    QCOMPARE(p.response(0), QString("ok"));  // back to inference
}

void TstGCodeProgram::navigationBounds()
{
    GCodeProgram p;
    p.append(makeItem("A"));
    p.append(makeItem("B"));
    p.append(makeItem("C"));

    QCOMPARE(p.lookAhead(0, 2), &p[2]);
    QVERIFY(p.lookAhead(0, 3) == nullptr);
    QVERIFY(p.lookAhead(0, 0) == nullptr);
    QCOMPARE(p.lookBehind(2, 1), &p[1]);
    QVERIFY(p.lookBehind(0, 1) == nullptr);
    QCOMPARE(p.getLine(1), &p[1]);
    QVERIFY(p.getLine(-1) == nullptr);
    QVERIFY(p.getLine(3) == nullptr);
}

void TstGCodeProgram::hashTracksMainProgramOnly()
{
    GCodeProgram p;
    p.append(makeItem("G0 X0"));
    p.markAsNotModified();
    QVERIFY(!p.isModified());

    p.append(makeItem("G1 X10"));
    QVERIFY(p.isModified());

    p.markAsNotModified();
    QVERIFY(!p.isModified());

    // Overlay items are excluded from the hash, so adding one is not "modified".
    p.insertOverlay("o", { makeItem("M3") }, p.count());
    QVERIFY(!p.isModified());
}

QTEST_APPLESS_MAIN(TstGCodeProgram)
#include "tst_gcodeprogram.moc"
