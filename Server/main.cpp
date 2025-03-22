#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "SerialHandle.h"
#include "BadWordHandle.h"
#include "RoomHandle.h"
#include <QFile>

int main(int argc, char *argv[])
{

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    NetworkHandle networkHandler;
    int tcpPort = 20000;
    SerialHandle serialHandler(&networkHandler, tcpPort);   //의존성 주입 (DI)
    BadWordHandle badWordHandler("/Users/imjeongseob/QT_Example/Frontman_stm32/badword.txt");

    engine.rootContext()->setContextProperty("badWordHandler", &badWordHandler);
    engine.rootContext()->setContextProperty("serialHandler", &serialHandler); // QML에서 사용 가능하도록 등록
    engine.rootContext()->setContextProperty("networkHandler", &networkHandler);
    engine.rootContext()->setContextProperty("roomHandler", &RoomHandle::instance()); //이렇게해도 싱글톤은 깨지지 않을 것이다.

    // QML 파일이 올바르게 로드되는지 확인하는 디버깅 메시지
    QString qmlPath = ":/main.qml";
    qDebug() << "Checking if QML file exists: " << qmlPath << " -> " << QFile::exists(qmlPath);

    engine.load(":/main.qml");

    // QML에서 chat room 창을 생성하는 함수를 호출하는 예시 - UI (QML)부분에 한해서 연결짓는데 사용한다. 2명 랜덤매칭 작업은 C++로 처리해야하므로 다른 슬롯을 사용한다.
    QObject::connect(&serialHandler, &SerialHandle::addRoom,
                     [&engine](int roomNum){
                         // 예: QML 내 함수 createChatRoom(roomNum)를 호출
                         QObject *root = engine.rootObjects().first();
                         QMetaObject::invokeMethod(root, "UI_createChatRoom",
                                                   Q_ARG(QVariant, roomNum));
                     });

    QObject::connect(&serialHandler, &SerialHandle::delRoom,
                     [&engine](int roomNum){
                         // 예: QML 내 함수 deleteChatRoom(roomNum)를 호출
                         QObject *root = engine.rootObjects().first();
                         QMetaObject::invokeMethod(root, "UI_deleteChatRoom",
                                                   Q_ARG(QVariant, roomNum));
                     });

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}




