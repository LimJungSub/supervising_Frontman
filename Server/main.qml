import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

Window {
    id: window
    visible: true
    width: 640
    height: 480
    title: "Waiting Screen"

    // 기본 대기 화면
    Rectangle {
        id: waitingScreen
        anchors.fill: parent
        color: "white"

        Text {
            id: waitingText
            text: "Waiting Pushed Start Button..."
            color: "black"
            font.pixelSize: 24
            anchors.centerIn: parent
        }
    }

    // serialHandler 시그널 연결
    Connections {
        target: serialHandler  // setContextProperty로 등록한 C++ 객체
        onDataReceived: function(message) {
            if (message === "RUN_QT") {
                console.log("[init.qml] Qt program Triggered - QML");
                // FrontmanWindow.qml을 동적으로 생성
                var component = Qt.createComponent("qrc:/FrontmanWindow.qml");
                if (component.status === Component.Ready) {
                    var frontmanWindow = component.createObject(null);  // 부모 객체 없이 새 창 생성
                    if (frontmanWindow) {
                        console.log("[init.qml] FrontmanWindow 생성 성공, 창 띄우기");
                        frontmanWindow.show();  // 새 창을 화면에 표시
                        // 현재 대기 창을 닫음 (예: entranceWindow라는 ID의 창)
                        Qt.callLater(function() { window.close(); });
                    } else {
                        console.log("[init.qml] FrontmanWindow 생성 실패");
                    }
                } else {
                    console.log("[init.qml] FrontmanWindow 로드 실패: " + component.errorString);
                }
            } else if (message.startsWith("DEL")) {
                console.log("[init.qml] Room Deleted - QML");
                // 추가 작업: 필요한 경우 창 삭제 등 처리
            } else if (message.startsWith("ADD")) {
                console.log("[init.qml] Room Added - QML");
                // 추가 작업: 필요한 경우 창 생성 등 처리
            }
        }
    }
}
