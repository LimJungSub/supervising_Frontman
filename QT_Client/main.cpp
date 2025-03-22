#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "ServerInteraction.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/Init.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);


    ServerInteraction serverInteractionHandler;
    engine.rootContext()->setContextProperty("ServerInteraction", &serverInteractionHandler);



    engine.load(url);






    return app.exec();
}
