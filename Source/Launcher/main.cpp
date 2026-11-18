#include "GameScanner.h"
#include "ProfileStats.h"

#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStandardPaths>
#include <QUrl>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("ArcadePortfolio"));
    app.setQuitOnLastWindowClosed(false);
    GameScanner games(QDir(QCoreApplication::applicationDirPath()).filePath("games"));
    const QString profileDirectory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    const QString profilePath = QDir(profileDirectory).filePath("profile.json");
    ProfileStats profileStats(profilePath);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("games"), &games);
    engine.rootContext()->setContextProperty(QStringLiteral("profileStats"), &profileStats);
    const QUrl mainQml(QStringLiteral("qrc:/qt/qml/ArcadeLauncher/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                     [mainQml](QObject *object, const QUrl &url) {
                         if (!object && url == mainQml)
                             QCoreApplication::exit(1);
                     }, Qt::QueuedConnection);
    engine.load(mainQml);
    return app.exec();
}
