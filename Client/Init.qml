import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs

Window {
    id: entranceWindow
    visible: true
    width: 400
    height: 300
    title: "Frontman Chat Client - Require Connection"

    Column {
        anchors.centerIn: parent
        spacing: 20

        TextField {
            id: serverAddressField
            color: "#050505"
            selectedTextColor: "#d8e21212"
            placeholderTextColor: "#984c4848"
            placeholderText: "서버 IP 주소"
        }

        TextField {
            id: serverPortField
            color: "#050505"
            selectedTextColor: "#d8e21212"
            placeholderTextColor: "#984c4848"
            placeholderText: "서버 포트 번호"
        }

        Button {
            Text{
                text: "입장"
                anchors.centerIn: parent
                color: "#050505"
                styleColor: "black"
            }
            font.hintingPreference: Font.PreferVerticalHinting
            font.weight: Font.Bold
            onClicked: {
                var serverAddress = serverAddressField.text;
                var serverPort = parseInt(serverPortField.text);

                console.log("Server Address:", serverAddress);
                console.log("Server Port:", serverPort);

                var ipPattern = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;

                if(ipPattern.test(serverAddress) && serverPort >=1 && serverPort <=65535){
                    console.log("[init.qml] connecting server, sending info to server");
                    connectToServer(serverAddress, parseInt(serverPort));
                }
                else{
                    console.log("[init.qml] Please enter valid server address.");
                    connErrorWin.visible = true;
                }
            }
        }
    }

    function connectToServer(serverAddress, serverPort) {
        console.log("[init.qml] Connecting to server at " + serverAddress + ":" + serverPort);
        ServerInteraction.getServerConnectionResult(serverAddress, serverPort);
    }

    Window {
        id: connErrorWin
        width: 300
        height: 150
        visible: false
        modality: Qt.ApplicationModal
        title: "경고"
        Rectangle {
            width: parent.width
            height: parent.height

            Column {
                anchors.centerIn: parent
                spacing: 20

                Text {
                    font.bold: true
                    text: "올바른 정보를 입력해주세요. 연결이 실패했습니다."
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                Button {
                    onClicked: connErrorWin.close()
                    contentItem: Text {
                        text: "확인"
                        color: "black"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    Text {
        id: _text
        x: 125
        y: 48
        width: 138
        height: 25
        text: "서버 입장을 위해, 다음을 입력하세요."
        font.pixelSize: 13
        leftPadding: 6
        topPadding: 0
        font.weight: Font.Bold
    }

    Connections{
        target: ServerInteraction
        onWaitingForMatch: {
            console.log("[init.qml] Waiting for random match...");
            var waitingComponent = Qt.createComponent("qrc:/WaitingWindow.qml");
            if (waitingComponent.status === Component.Ready) {
                var waitingWindow = waitingComponent.createObject(null);
                if (waitingWindow) {
                    waitingWindow.show();
                    Qt.callLater(() => entranceWindow.close());
                } else {
                    console.log("[init.qml] Failed to create waiting window");
                }
            } else {
                console.log("[init.qml] Component not ready: " + waitingComponent.errorString());
            }
        }
        onConnectionFail:{
            console.log("[init.qml] 연결이 실패했습니다.");
            connectionErrorDialog.open();
        }
    }

    MessageDialog{
        id: connectionErrorDialog
        title: "알림"
        text: "연결이 실패했습니다."
    }
}
