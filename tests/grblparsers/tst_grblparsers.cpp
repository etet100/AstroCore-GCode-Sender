// Unit tests for the pure GRBL text parsers:
//   - StatusReportProcessor  (<Run|MPos:...|FS:...>)
//   - ProbeResponseParser     ([PRB:x,y,z:contact])
//   - ModalStateParser        ([GC:G54 G17 ...])
// All three are static, dependency-free string-in / struct-out functions.

#include <QtTest>
#include "core/communicator/statusreportprocessor.h"
#include "core/communicator/proberesponseparser.h"
#include "core/machine/modalstateparser.h"

class TstGrblParsers : public QObject
{
    Q_OBJECT

private slots:
    // StatusReportProcessor
    void statusRejectsMalformed();
    void statusParsesFullLine();
    void statusParsesPinsAndOverrides();

    // ProbeResponseParser
    void probeParsesContact();
    void probeReturnsNulloptWhenNoPrb();

    // ModalStateParser
    void modalParsesTokens();
    void modalRejectsWrongPrefix();
};

void TstGrblParsers::statusRejectsMalformed()
{
    QVERIFY(!StatusReportProcessor::parse("Run|MPos:0,0,0").has_value());   // no brackets
    QVERIFY(!StatusReportProcessor::parse("").has_value());
}

void TstGrblParsers::statusParsesFullLine()
{
    const auto r = StatusReportProcessor::parse(
        "<Run|MPos:-10.780,-9.740,3.000|Bf:15,128|FS:673,1000|WCO:1.000,2.000,3.000>");

    QVERIFY(r.has_value());
    QCOMPARE(r->state, MachineState::Run);

    QVERIFY(r->hasMachinePos);
    QCOMPARE(r->machinePos.x(), -10.780f);
    QCOMPARE(r->machinePos.y(), -9.740f);
    QCOMPARE(r->machinePos.z(), 3.000f);

    QVERIFY(r->hasFeedSpindleSpeed);
    QCOMPARE(r->feedRate, 673);
    QCOMPARE(r->spindleSpeed, 1000);

    QVERIFY(r->hasBufferStatus);
    QCOMPARE(r->bufferAvailable, 15);
    QCOMPARE(r->bufferSize, 128);

    QVERIFY(r->hasWorkOffset);
    QCOMPARE(r->workOffset.z(), 3.000f);
}

void TstGrblParsers::statusParsesPinsAndOverrides()
{
    const auto r = StatusReportProcessor::parse(
        "<Idle|WPos:0.000,0.000,0.000|Ov:110,100,95|Pn:XYP|A:SF>");

    QVERIFY(r.has_value());
    QCOMPARE(r->state, MachineState::Idle);

    QVERIFY(r->hasWorkPos);

    QVERIFY(r->hasOverrides);
    QCOMPARE(r->feedOverride, 110);
    QCOMPARE(r->rapidOverride, 100);
    QCOMPARE(r->spindleOverride, 95);

    QVERIFY(r->hasPinStates);
    QVERIFY(r->pinStates.limitX);
    QVERIFY(r->pinStates.limitY);
    QVERIFY(r->pinStates.probe);
    QVERIFY(!r->pinStates.limitZ);

    QVERIFY(r->hasAccessoryState);
    QVERIFY(r->spindleEnabled);
    QVERIFY(r->spindleCW);       // 'S' present
    QVERIFY(r->floodEnabled);    // 'F' present
    QVERIFY(!r->mistEnabled);
}

void TstGrblParsers::probeParsesContact()
{
    const auto r = ProbeResponseParser::parse(
        { "ok", "[PRB:0.000,0.000,-8.530:1]", "ok" });

    QVERIFY(r.has_value());
    QCOMPARE(r->position.z(), -8.530f);
    QVERIFY(r->contacted);
}

void TstGrblParsers::probeReturnsNulloptWhenNoPrb()
{
    QVERIFY(!ProbeResponseParser::parse({ "ok", "error:9" }).has_value());
    // contact flag 0 -> not contacted
    const auto r = ProbeResponseParser::parse({ "[PRB:1.0,2.0,3.0:0]" });
    QVERIFY(r.has_value());
    QVERIFY(!r->contacted);
}

void TstGrblParsers::modalParsesTokens()
{
    const auto r = ModalStateParser::parse(
        "[GC:G54 G17 G21 G90 G94 M5 T0 F100 S1000]");

    QVERIFY(r.has_value());
    QCOMPARE(r->coordinateSystem, QString("G54"));
    QCOMPARE(r->workPlane, QString("G17"));
    QCOMPARE(r->units, QString("G21"));
    QCOMPARE(r->motionMode, QString("G90"));
    QCOMPARE(r->feedMode, QString("G94"));
    QCOMPARE(r->spindleMode, QString("M5"));
    QCOMPARE(r->feedRate, 100);
    QCOMPARE(r->spindleSpeed, 1000);
}

void TstGrblParsers::modalRejectsWrongPrefix()
{
    QVERIFY(!ModalStateParser::parse("[G54 G17 G21]").has_value()); // missing "GC:" prefix
    QVERIFY(!ModalStateParser::parse("GC:G54]").has_value());
}

QTEST_GUILESS_MAIN(TstGrblParsers)
#include "tst_grblparsers.moc"
