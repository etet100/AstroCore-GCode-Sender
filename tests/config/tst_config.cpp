// Unit tests for core/config:
//   - derived logic in config modules (finalFeedZ, spindleSpeedRatio,
//     arcApproximationValue) + the changed() signal
//   - AbstractConfigurationModule::MinMax comparison operators
//   - JSON persister -> provider round-trip through a temp file
//
// Config modules are plain QObjects (constructor takes parent + defaults, no
// singleton registration), so they can be built and driven directly. Members
// are not initialised from the defaults map by the constructor (that happens on
// load()), so tests set the inputs explicitly via setters or setProperty().

#include <QtTest>
#include <QDir>
#include <QFile>
#include "core/config/module/abstractconfigurationmodule.h"
#include "core/config/module/configurationjogging.h"
#include "core/config/module/configurationmachine.h"
#include "core/config/module/configurationparser.h"
#include "core/config/persistence/json/jsonpersister.h"
#include "core/config/persistence/json/jsonprovider.h"

using MinMax = AbstractConfigurationModule::MinMax;

class TstConfig : public QObject
{
    Q_OBJECT

private slots:
    void joggingFinalFeedZ();
    void joggingEmitsChanged();
    void machineSpindleSpeedRatio();
    void parserArcApproximationValue();
    void minMaxOperators();
    void jsonPersisterProviderRoundTrip();
};

void TstConfig::joggingFinalFeedZ()
{
    ConfigurationJogging c;
    c.setFeed(100);
    c.setFeedZ(200);

    c.setSeparateFeedZ(false);
    QCOMPARE(c.finalFeedZ(), 100);   // separate Z off -> uses feed

    c.setSeparateFeedZ(true);
    QCOMPARE(c.finalFeedZ(), 200);   // separate Z on -> uses feedZ
}

void TstConfig::joggingEmitsChanged()
{
    ConfigurationJogging c;
    QSignalSpy spy(&c, &AbstractConfigurationModule::changed);

    c.setFeed(500);
    c.setSeparateFeedZ(true);

    QCOMPARE(spy.count(), 2);
}

void TstConfig::machineSpindleSpeedRatio()
{
    ConfigurationMachine c;

    c.setProperty("spindleSpeedRange", QVariant::fromValue(MinMax{0, 1000}));
    QCOMPARE(c.spindleSpeedRange().max, 1000);
    QCOMPARE(c.spindleSpeedRatio(), 10.0);   // (1000 - 0) / 100.0

    // Ratio is a true floating-point division (max-min)/100.0, so it does not
    // truncate: 2550/100.0 == 25.5.
    c.setProperty("spindleSpeedRange", QVariant::fromValue(MinMax{0, 2550}));
    QCOMPARE(c.spindleSpeedRatio(), 25.5);
}

void TstConfig::parserArcApproximationValue()
{
    ConfigurationParser c;
    c.setProperty("arcApproximationLength", 0.5);
    c.setProperty("arcApproximationAngle", 7.0);

    c.setProperty("arcApproximationMode", QVariant::fromValue(ConfigurationParser::ByLength));
    QCOMPARE(c.arcApproximationValue(), 0.5);

    c.setProperty("arcApproximationMode", QVariant::fromValue(ConfigurationParser::ByAngle));
    QCOMPARE(c.arcApproximationValue(), 7.0);
}

void TstConfig::minMaxOperators()
{
    QVERIFY(MinMax({1, 2}) == MinMax({1, 2}));
    QVERIFY(MinMax({1, 2}) != MinMax({1, 3}));
    QVERIFY(MinMax({1, 2}) != MinMax({9, 2}));
    QVERIFY(!(MinMax({1, 2}) != MinMax({1, 2})));
}

void TstConfig::jsonPersisterProviderRoundTrip()
{
    const QString path = QDir::temp().filePath("astrocore_tst_config.json");
    QFile::remove(path);

    {
        JsonPersister p(nullptr, path);
        QVERIFY(p.open());
        p.setInt("machine", "feed", 1200);
        p.setString("machine", "name", "cnc");
        p.setDouble("parser", "angle", 5.5);
        p.setBool("ui", "dark", true);
        p.setStringList("jog", "steps", { "0.1", "1.0" });
        p.close();
    }

    QVERIFY(QFile::exists(path));

    {
        JsonProvider prov(nullptr, path);
        QVERIFY(prov.open());
        QCOMPARE(prov.getInt("machine", "feed", 0), 1200);
        QCOMPARE(prov.getString("machine", "name", QString()), QString("cnc"));
        QCOMPARE(prov.getDouble("parser", "angle", 0.0), 5.5);
        QCOMPARE(prov.getBool("ui", "dark", false), true);
        QCOMPARE(prov.getStringList("jog", "steps", {}), QStringList({ "0.1", "1.0" }));
        // Missing key falls back to the supplied default.
        QCOMPARE(prov.getInt("machine", "missing", -1), -1);
    }

    QFile::remove(path);
}

QTEST_GUILESS_MAIN(TstConfig)
#include "tst_config.moc"
