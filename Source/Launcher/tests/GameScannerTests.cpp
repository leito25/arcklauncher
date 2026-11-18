#include "GameScanner.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <cstdio>

class GameScannerTests : public QObject
{
    Q_OBJECT

private slots:
    void discoversAndLaunchesGame();
    void reportsInvalidMetadataAndMissingExecutable();
    void reportsDuplicateGameIds();
    void reportsLaunchFailure();
    void reportsGameErrorDetails();
};

static QString createGame(const QTemporaryDir &root, const QByteArray &metadata)
{
    QDir directory(root.path());
    if (!directory.mkdir("sample"))
        return {};
    const QString gameDirectory = directory.filePath("sample");
    QFile file(QDir(gameDirectory).filePath("meta.json"));
    if (!file.open(QIODevice::WriteOnly) || file.write(metadata) != metadata.size())
        return {};
    return gameDirectory;
}

void GameScannerTests::discoversAndLaunchesGame()
{
    QTemporaryDir root;
    QVERIFY(root.isValid());
    const QString executable = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    const QByteArray metadata = QString("{\"id\":\"circle-sample\",\"title\":\"Circle Sample\",\"executable\":\"%1\"}")
                                    .arg(executable).toUtf8();
    const QString directory = createGame(root, metadata);
    QVERIFY(!directory.isEmpty());
    QVERIFY(QFile::copy(QCoreApplication::applicationFilePath(), QDir(directory).filePath(executable)));

    GameScanner scanner(root.path());
    QCOMPARE(scanner.rowCount(), 1);
    QCOMPARE(scanner.data(scanner.index(0), Qt::UserRole + 1).toString(), QString("Circle Sample"));
    QCOMPARE(scanner.data(scanner.index(0), Qt::UserRole + 2).toString(), QString("circle-sample"));
    QVERIFY(scanner.errorMessage().isEmpty());

    QSignalSpy stateChanges(&scanner, &GameScanner::runningChanged);
    qputenv("ARCADE_TEST_CHILD", "1");
    scanner.launch(0);
    QTRY_VERIFY_WITH_TIMEOUT(scanner.running(), 5000);
    qunsetenv("ARCADE_TEST_CHILD");
    QTRY_VERIFY_WITH_TIMEOUT(!scanner.running(), 5000);
    QVERIFY(stateChanges.count() >= 2);
    QVERIFY(scanner.errorMessage().isEmpty());
}

void GameScannerTests::reportsInvalidMetadataAndMissingExecutable()
{
    QTemporaryDir root;
    QVERIFY(root.isValid());
    const QString directory = createGame(root, "{\"title\":\"Bad\", \"executable\":\"../game\"}");
    QVERIFY(!directory.isEmpty());
    GameScanner scanner(root.path());
    QCOMPARE(scanner.rowCount(), 0);
    QVERIFY(scanner.errorMessage().contains("Invalid game metadata"));

    QFile metadata(QDir(directory).filePath("meta.json"));
    QVERIFY(metadata.open(QIODevice::WriteOnly | QIODevice::Truncate));
    metadata.write("{\"id\":\"Bad ID\", \"title\":\"Bad\", \"executable\":\"missing.exe\"}");
    metadata.close();
    scanner.refresh();
    QCOMPARE(scanner.rowCount(), 0);
    QVERIFY(scanner.errorMessage().contains("Invalid game metadata"));

    QVERIFY(metadata.open(QIODevice::WriteOnly | QIODevice::Truncate));
    metadata.write("{\"id\":\"sample\", \"title\":\"Missing\", \"executable\":\"missing.exe\"}");
    metadata.close();
    scanner.refresh();
    QCOMPARE(scanner.rowCount(), 0);
    QVERIFY(scanner.errorMessage().contains("Game executable not found"));
}

void GameScannerTests::reportsDuplicateGameIds()
{
    QTemporaryDir root;
    QVERIFY(root.isValid());
    const QString executable = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    for (const QString &name : {QStringLiteral("first"), QStringLiteral("second")}) {
        QDir directory(root.path());
        QVERIFY(directory.mkdir(name));
        const QString gameDirectory = directory.filePath(name);
        QFile metadata(QDir(gameDirectory).filePath("meta.json"));
        QVERIFY(metadata.open(QIODevice::WriteOnly));
        const QByteArray contents =
            QString("{\"id\":\"duplicate\",\"title\":\"%1\",\"executable\":\"%2\"}")
                .arg(name, executable).toUtf8();
        QCOMPARE(metadata.write(contents), qint64(contents.size()));
        QVERIFY(QFile::copy(QCoreApplication::applicationFilePath(),
                            QDir(gameDirectory).filePath(executable)));
    }

    GameScanner scanner(root.path());
    QCOMPARE(scanner.rowCount(), 1);
    QVERIFY(scanner.errorMessage().contains("Duplicate game ID"));
}

void GameScannerTests::reportsLaunchFailure()
{
    QTemporaryDir root;
    QVERIFY(root.isValid());
    const QString executable = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    const QString directory = createGame(root, QString("{\"id\":\"sample\",\"title\":\"Sample\",\"executable\":\"%1\"}")
                                                   .arg(executable).toUtf8());
    QVERIFY(!directory.isEmpty());
    const QString path = QDir(directory).filePath(executable);
    QVERIFY(QFile::copy(QCoreApplication::applicationFilePath(), path));
    GameScanner scanner(root.path());
    QCOMPARE(scanner.rowCount(), 1);
    QVERIFY(QFile::remove(path));
    scanner.launch(0);
    QTRY_VERIFY_WITH_TIMEOUT(scanner.errorMessage().contains("Could not start"), 5000);
    QVERIFY(!scanner.running());
}

void GameScannerTests::reportsGameErrorDetails()
{
    QTemporaryDir root;
    QVERIFY(root.isValid());
    const QString executable = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    const QString directory = createGame(root, QString("{\"id\":\"sample\",\"title\":\"Sample\",\"executable\":\"%1\"}")
                                                   .arg(executable).toUtf8());
    QVERIFY(!directory.isEmpty());
    QVERIFY(QFile::copy(QCoreApplication::applicationFilePath(), QDir(directory).filePath(executable)));
    GameScanner scanner(root.path());
    qputenv("ARCADE_TEST_CHILD", "error");
    scanner.launch(0);
    QTRY_VERIFY_WITH_TIMEOUT(scanner.errorMessage().contains("profile save failed"), 5000);
    qunsetenv("ARCADE_TEST_CHILD");
    QVERIFY(scanner.errorMessage().contains("code 1"));
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if (qgetenv("ARCADE_TEST_CHILD") == "error") {
        std::fputs("profile save failed\n", stderr);
        return 1;
    }
    if (qEnvironmentVariableIsSet("ARCADE_TEST_CHILD")) {
        QTimer::singleShot(500, &app, &QCoreApplication::quit);
        return app.exec();
    }
    GameScannerTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#include "GameScannerTests.moc"
