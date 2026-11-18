#include "ProfileStats.h"

#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

class ProfileStatsTests : public QObject
{
    Q_OBJECT

private slots:
    void sumsGamePlaytime();
    void reportsInvalidProfiles();
    void refreshesAfterProfileReplacement();
    void watchesForProfileCreatedLater();
};

static bool writeProfile(const QString &path, const QByteArray &contents)
{
    QSaveFile file(path);
    return file.open(QIODevice::WriteOnly) &&
           file.write(contents) == contents.size() &&
           file.commit();
}

void ProfileStatsTests::sumsGamePlaytime()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = QDir(directory.path()).filePath("profile.json");
    QVERIFY(writeProfile(path,
        R"({"schemaVersion":1,"games":{"one":{"playtimeSeconds":3661},"two":{"playtimeSeconds":59}}})"));

    ProfileStats stats(path);
    QCOMPARE(stats.formattedPlaytime(), QString("1h 2m"));
    QCOMPARE(stats.gamePlaytimes().value("one").toString(), QString("1h 1m"));
    QCOMPARE(stats.gamePlaytimes().value("two").toString(), QString("0m"));
    QVERIFY(stats.errorMessage().isEmpty());
}

void ProfileStatsTests::reportsInvalidProfiles()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = QDir(directory.path()).filePath("profile.json");
    QVERIFY(writeProfile(path, R"({"schemaVersion":2,"games":{}})"));

    ProfileStats stats(path);
    QVERIFY(stats.errorMessage().contains("Invalid or unsupported"));

    QVERIFY(writeProfile(path, R"({"schemaVersion":1,"games":{"one":{"playtimeSeconds":-1}}})"));
    stats.refresh();
    QVERIFY(stats.errorMessage().contains("Invalid game playtime"));
}

void ProfileStatsTests::refreshesAfterProfileReplacement()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = QDir(directory.path()).filePath("profile.json");
    QVERIFY(writeProfile(path, R"({"schemaVersion":1,"games":{"one":{"playtimeSeconds":60}}})"));

    ProfileStats stats(path);
    QCOMPARE(stats.formattedPlaytime(), QString("1m"));
    QSignalSpy statsChanged(&stats, &ProfileStats::statsChanged);
    QVERIFY(writeProfile(path, R"({"schemaVersion":1,"games":{"one":{"playtimeSeconds":7200}}})"));
    QTRY_COMPARE_WITH_TIMEOUT(stats.formattedPlaytime(), QString("2h 0m"), 5000);
    QCOMPARE(stats.gamePlaytimes().value("one").toString(), QString("2h 0m"));
    QVERIFY(statsChanged.count() > 0);
    QVERIFY(stats.errorMessage().isEmpty());
}

void ProfileStatsTests::watchesForProfileCreatedLater()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString profileDirectory = QDir(directory.path()).filePath("ArcadePortfolio");
    const QString path = QDir(profileDirectory).filePath("profile.json");

    ProfileStats stats(path);
    QVERIFY(stats.errorMessage().isEmpty());
    QVERIFY(QDir().mkpath(profileDirectory));
    QVERIFY(writeProfile(path, R"({"schemaVersion":1,"games":{"one":{"playtimeSeconds":600}}})"));
    QTRY_COMPARE_WITH_TIMEOUT(stats.formattedPlaytime(), QString("10m"), 5000);
}

QTEST_GUILESS_MAIN(ProfileStatsTests)
#include "ProfileStatsTests.moc"
