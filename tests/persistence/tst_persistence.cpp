// Unit tests for core/config/persistence.
//
// Two angles:
//  1. Inject a hand-crafted config file (JSON / INI / XML) and assert what each
//     provider reads back — this exercises the real format parsing, missing-key
//     fallbacks and (for XML) the type-attribute matching.
//  2. Round-trip: write with a persister, read with the matching provider —
//     covers the writing side and list types that are awkward to hand-craft.

#include <QtTest>
#include <QDir>
#include <QFile>
#include <QUuid>
#include "core/config/persistence/ini/iniprovider.h"
#include "core/config/persistence/ini/inipersister.h"
#include "core/config/persistence/json/jsonprovider.h"
#include "core/config/persistence/json/jsonpersister.h"
#include "core/config/persistence/xml/xmlprovider.h"
#include "core/config/persistence/xml/xmlpersister.h"

class TstPersistence : public QObject
{
    Q_OBJECT

    QString m_dir;

    QString writeFixture(const QString& name, const QByteArray& content)
    {
        const QString path = m_dir + "/" + name;
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly)) {
            return QString();
        }
        f.write(content);
        f.close();

        return path;
    }

    QString tempPath(const QString& name) { return m_dir + "/" + name; }

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Injected-fixture reads
    void jsonReadsInjectedFile();
    void iniReadsInjectedFile();
    void xmlReadsInjectedFile();
    void xmlTypeMismatchAndMissingReturnDefault();

    // Write -> read round-trips
    void iniRoundTrip();
    void xmlRoundTrip();
};

void TstPersistence::initTestCase()
{
    // Unique subdir under the system temp dir so parallel runs don't collide.
    m_dir = QDir::temp().filePath("astrocore_persist_" + QUuid::createUuid().toString(QUuid::Id128));
    QVERIFY(QDir().mkpath(m_dir));
}

void TstPersistence::cleanupTestCase()
{
    QDir(m_dir).removeRecursively();
}

void TstPersistence::jsonReadsInjectedFile()
{
    const QByteArray content = R"({
        "machine": { "feed": 1200, "name": "cnc", "dark": true },
        "parser":  { "angle": 5.5 },
        "jog":     { "steps": ["0.1", "1.0"] }
    })";
    const QString path = writeFixture("in.json", content);
    QVERIFY(!path.isEmpty());

    JsonProvider p(nullptr, path);
    QVERIFY(p.open());

    QCOMPARE(p.getInt("machine", "feed", 0), 1200);
    QCOMPARE(p.getString("machine", "name", QString()), QString("cnc"));
    QCOMPARE(p.getBool("machine", "dark", false), true);
    QCOMPARE(p.getDouble("parser", "angle", 0.0), 5.5);
    QCOMPARE(p.getStringList("jog", "steps", {}), QStringList({ "0.1", "1.0" }));
    QCOMPARE(p.getInt("machine", "missing", -1), -1);   // missing key -> default
    QCOMPARE(p.getInt("nogroup", "x", -2), -2);          // missing group -> default
}

void TstPersistence::iniReadsInjectedFile()
{
    const QByteArray content =
        "[machine]\n"
        "feed=1200\n"
        "name=cnc\n"
        "dark=true\n"
        "\n"
        "[parser]\n"
        "angle=5.5\n";
    const QString path = writeFixture("in.ini", content);
    QVERIFY(!path.isEmpty());

    IniProvider p(nullptr, path);
    QVERIFY(p.open());

    QCOMPARE(p.getInt("machine", "feed", 0), 1200);
    QCOMPARE(p.getString("machine", "name", QString()), QString("cnc"));
    QCOMPARE(p.getBool("machine", "dark", false), true);
    QCOMPARE(p.getDouble("parser", "angle", 0.0), 5.5);
    QCOMPARE(p.getInt("machine", "missing", -1), -1);
    p.close();
}

void TstPersistence::xmlReadsInjectedFile()
{
    const QByteArray content = R"(<?xml version="1.0"?>
<config>
  <group name="machine">
    <entry key="feed" type="int">1200</entry>
    <entry key="name" type="string">cnc</entry>
    <entry key="dark" type="bool">true</entry>
    <entry key="steps" type="stringlist">0.1,1.0</entry>
  </group>
  <group name="parser">
    <entry key="angle" type="double">5.5</entry>
  </group>
</config>)";
    const QString path = writeFixture("in.xml", content);
    QVERIFY(!path.isEmpty());

    XmlProvider p(nullptr, path);
    QVERIFY(p.open());

    QCOMPARE(p.getInt("machine", "feed", 0), 1200);
    QCOMPARE(p.getString("machine", "name", QString()), QString("cnc"));
    QCOMPARE(p.getBool("machine", "dark", false), true);
    QCOMPARE(p.getDouble("parser", "angle", 0.0), 5.5);
    QCOMPARE(p.getStringList("machine", "steps", {}), QStringList({ "0.1", "1.0" }));
}

void TstPersistence::xmlTypeMismatchAndMissingReturnDefault()
{
    const QByteArray content = R"(<?xml version="1.0"?>
<config>
  <group name="machine">
    <entry key="name" type="string">cnc</entry>
  </group>
</config>)";
    const QString path = writeFixture("mismatch.xml", content);
    QVERIFY(!path.isEmpty());

    XmlProvider p(nullptr, path);
    QVERIFY(p.open());

    // 'name' exists but is typed "string"; asking for an int must fall back.
    QCOMPARE(p.getInt("machine", "name", -1), -1);
    // Missing key / group also fall back.
    QCOMPARE(p.getString("machine", "missing", QString("def")), QString("def"));
    QCOMPARE(p.getDouble("nogroup", "x", 3.14), 3.14);
}

void TstPersistence::iniRoundTrip()
{
    const QString path = tempPath("rt.ini");
    QFile::remove(path);

    {
        IniPersister w(nullptr, path);
        QVERIFY(w.open());
        w.setInt("machine", "feed", 800);
        w.setBool("ui", "dark", true);
        w.setStringList("jog", "steps", { "0.1", "1.0", "10.0" });
        w.close();
    }
    QVERIFY(QFile::exists(path));

    IniProvider r(nullptr, path);
    QVERIFY(r.open());
    QCOMPARE(r.getInt("machine", "feed", 0), 800);
    QCOMPARE(r.getBool("ui", "dark", false), true);
    QCOMPARE(r.getStringList("jog", "steps", {}), QStringList({ "0.1", "1.0", "10.0" }));
    r.close();
}

void TstPersistence::xmlRoundTrip()
{
    const QString path = tempPath("rt.xml");
    QFile::remove(path);

    {
        XmlPersister w(nullptr, path);
        QVERIFY(w.open());
        w.setInt("machine", "feed", 800);
        w.setString("machine", "name", "router");
        w.setStringList("jog", "steps", { "0.1", "1.0", "10.0" });
        w.close();
    }
    QVERIFY(QFile::exists(path));

    XmlProvider r(nullptr, path);
    QVERIFY(r.open());
    QCOMPARE(r.getInt("machine", "feed", 0), 800);
    QCOMPARE(r.getString("machine", "name", QString()), QString("router"));
    QCOMPARE(r.getStringList("jog", "steps", {}), QStringList({ "0.1", "1.0", "10.0" }));
    r.close();
}

QTEST_GUILESS_MAIN(TstPersistence)
#include "tst_persistence.moc"
