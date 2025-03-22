import QtQuick 2.15
import QtQuick.Controls 2.15

Window {
    id: waitingWindow
    visible: true
    width: 600
    height: 400
    title: "Waiting for Match"

    Rectangle {
        anchors.fill: parent
        color: "white"

        Column {
            anchors.centerIn: parent
            spacing: 20

            Text {
                text: "You are waiting for Matching new client"
                font.pixelSize: 24
                font.weight: Font.Black
                color: "black"
                horizontalAlignment: Text.AlignHCenter
            }

            // GIF/APNG 로딩 애니메이션
            AnimatedImage {
                id: loadingSpinner
                source: "qrc:/Loading_LINEAR_BLUE.gif"  // qrc 리소스 사용
                width: 50
                height: 50
                anchors.horizontalCenter: parent.horizontalCenter
                playing: true  // 자동 재생
            }

            // 서버에서 받은 현재 접속자 수 표시
            Text {
                //C++의 NULL은 QML(JS)에서 처리가 안되기때문에 이렇게 별도로 처리했다.
                text: "Current server connected user number: " + (ServerInteraction ? ServerInteraction.currentClientNumber : "N/A")
                font.pixelSize: 24
                font.weight: Font.Black
                color: "black"
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    // ServerInteraction의 openChatWindow 시그널을 받아 처리하는 Connections
    Connections {
        target: ServerInteraction
        //"onOpenChatWindow: { }"구조로 작성하면 오류내는거 알지?
        onOpenChatWindow: function(roomNum) {
            console.log("Received openChatWindow signal with room number:", roomNum);
            openChatRoom(roomNum);
        }
    }

    function openChatRoom(roomNum) {
            var component = Qt.createComponent("qrc:/ChatWindow.qml");
            if (component.status === Component.Ready) {
                // waitingWindow를 부모로 지정 (window 대신 waitingWindow 사용)
                var chatWindow = component.createObject(waitingWindow, { "roomNum": roomNum });
                if (chatWindow !== null) {
                    chatWindow.title = "Chat Room - " + roomNum;
                    waitingWindow.close();
                    chatWindow.show();
                } else {
                    console.log("Error creating chat window object.");
                }
            } else {
                console.log("Error creating chat window: " + component.errorString());
            }
        }
}
