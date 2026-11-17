#include "GameScanner.h"

#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    GameScanner games(QDir(QCoreApplication::applicationDirPath()).filePath("games"));
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("games"), &games);
    const QUrl mainQml(QStringLiteral("qrc:/qt/qml/ArcadeLauncher/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                     [mainQml](QObject *object, const QUrl &url) {
                         if (!object && url == mainQml)
                             QCoreApplication::exit(1);
                     }, Qt::QueuedConnection);
    engine.load(mainQml);
    return app.exec();
}
