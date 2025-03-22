import QtQuick 2.15
import QtQuick.Controls 2.15

Window {
    id: chatWindow
    visible: true
    width: 600
    height: 400
    title: "Chat Room: " + roomNum

    // WaitingWindow의 openChatRoom에서 세팅됨
    property int roomNum

    // QML 시그널 선언 (메시지를 보내기 위한 시그널)
    signal sendMessageSignal(string message)
    signal sendEndMessageToServer(int roomNum);

    // 채팅 메시지 모델 정의
    ListModel {
        id: chatModel
    }

    // 1번: 유저들의 실시간 채팅 기록 영역
    Rectangle {
        width: parent.width * 2 / 3
        height: parent.height
        color: "#f0f0f0"

        Column {
            id: column
            anchors.fill: parent
            spacing: 5

            // 채팅 기록을 표시하는 ListView
            ListView {
                id: chatListView
                width: parent.width
                height: parent.height - 50
                spacing: 5
                model: chatModel   // 채팅 메시지 모델 (아래에서 정의)
                clip: true

                // delegate: 메시지 타입에 따라 스타일을 다르게 설정
                delegate: Item {
                    width: chatListView.width
                    // 내부 Column의 암시적 높이에 따라 delegate 높이를 결정
                    height: contentColumn.implicitHeight + 10

                    Rectangle {
                        id: messageRect
                        width: parent.width * 0.75
                        // "나:" 메시지는 배경색과 정렬을 다르게 처리
                        color: model.sender !== undefined && model.sender === "나" ? "#DCF8C6" : "#FFFFFF"
                        radius: 10
                        border.color: "lightgray"
                        anchors.horizontalCenter: parent.horizontalCenter
                        // "나:" 메시지는 오른쪽 정렬, 상대 메시지는 왼쪽 정렬
                        anchors.right: (model.sender !== undefined && model.sender === "나") ? parent.right : undefined
                        anchors.left: (model.sender !== undefined && model.sender !== "나") ? parent.left : undefined
                        anchors.margins: 5

                        Column {
                            id: contentColumn
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 4

                            // 발신자 표시
                            Text {
                                text: model.sender !== undefined ? model.sender : "나"
                                font.pixelSize: 14
                                font.bold: true
                                color: "gray"
                            }

                            // 메시지 내용 표시
                            Text {
                                text: model.message
                                font.pixelSize: 16
                                font.bold: model.sender === "시스템" ? true : false
                                color: model.sender === "시스템" ? "red" : "black"
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
                // 새로운 메시지가 추가되면 ListView의 끝으로 자동 스크롤
                onCountChanged: chatListView.positionViewAtEnd()
            }

            // 채팅 입력 창과 전송 버튼
            Row {
                width: parent.width
                height: 50
                spacing: 5

                // 내부적으로 정의되어 있는 시그널 Keys.onReturnPressed를 사용하여 엔터키 입력 처리
                TextField {
                    id: chatInput
                    width: parent.width - 100
                    placeholderText: "Type your message..."
                    Keys.onReturnPressed: {
                        if (chatInput.text !== "") {
                            console.log("[ChatWindow] return pressed / contents: " + chatInput.text);
                            // QML 시그널 발생: sendMessageSignal 호출
                            sendMessageSignal(chatInput.text);
                            // 보낸 메시지는 "나:" 접두어 없이, 모델에 sender와 message 프로퍼티 포함하여 추가
                            chatModel.append({ "sender": "나", "message": chatInput.text });
                            chatInput.clear();
                        } else {
                            warningWindow.visible = true;
                        }
                    }
                }

                Button {
                    text: "Send"
                    width: 80
                    onClicked: {
                        if (chatInput.text !== "") {
                            console.log("[ChatWindow] Send button clicked / contents: " + chatInput.text);
                            sendMessageSignal(chatInput.text);
                            chatModel.append({ "sender": "나", "message": chatInput.text });
                            chatInput.clear();
                        } else {
                            warningWindow.visible = true;
                        }
                    }
                }
            }
        }

        // 경고창으로 사용할 새 창 (모달로 설정)
        Window {
            id: warningWindow
            width: 300
            height: 150
            title: "경고"
            visible: false
            modality: Qt.ApplicationModal   // 모달 설정

            Rectangle {
                width: parent.width
                height: parent.height

                Column {
                    anchors.centerIn: parent
                    spacing: 20

                    Text {
                        font.bold: true
                        text: "메시지를 입력해주세요."
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    Button {
                        onClicked: warningWindow.close()
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

        // QML Connections: 서버 측(등록된 C++ 객체 ServerInteraction)의 시그널들을 받아 처리
        Connections {
            // QML 시그널(sendMessageSignal)을 C++ 슬롯(sendData)으로 연결
            target: chatWindow
            function onSendMessageSignal(message) {
                // context property로 등록된 C++ 객체의 이름은 "ServerInteraction"이어야 함
                ServerInteraction.sendData(message);
            }
        }

        Connections {
            // 서버 측(ServerInteraction)의 opponentMessageReceived 시그널 연결
            target: ServerInteraction
            function onOpponentMessageReceived(msg) {
                // 상대방이 보낸 메시지를 채팅 모델에 추가 (일반 메시지 타입)
                chatModel.append({ "sender": "상대", "message": msg });
            }
        }

        Connections {
            // 서버 측(ServerInteraction)의 gameResultReceived 시그널 연결
            target: ServerInteraction
            function onGameResultReceived(msg) {

                // 게임 결과 메시지를 채팅 모델에 추가; UI적으로 강조되어야 함 (예: 폰트 크기, 색상 등은 delegate에서 처리)
                chatModel.append({ "sender": "시스템", "message": msg});
                resultTimer.start();

                // 게임종료 시 서버로 roomNum을 보내기 위해, 클라이언트의 QML에서 시그널발생 -> 클라이언트의 C++ 슬롯함수를 호출
                sendEndMessageToServer(roomNum);    //시그널 발생
            }
        }

        Connections {
            target: chatWindow
            function onSendEndMessageToServer(roomNum) {
                // myHandler는 main.cpp에서 등록한 C++ 객체
                ServerInteraction.onSendEndMessageToServer_STM32(roomNum);
            }
        }

        // 3초 후에 결과 창을 띄우는 Timer
        Timer {
            id: resultTimer
            interval: 3000; repeat: false
            onTriggered: {
                    // 결과 창을 생성하기 전에 모델에 최소한 하나 이상의 메시지가 있는지 확인
                    if (chatModel.count > 0) {
                        //note: ListModel.count : ListModel에 저장된 항목의 총 개수를 반환한다.
                        //유저가 마지막에 승리라는 메시지를 받았었으면 resultText를 승리로 기록, 아니라면 패배로 기록
                        var resultMsg = chatModel.get(chatModel.count - 1);
                        var resultText = "";
                        var resultColor = "";
                        if (resultMsg.message.indexOf("승리") !== -1) {
                            resultText = "승리";
                            resultColor = "blue";
                        }
                        else if (resultMsg.message.indexOf("패배") !== -1) {
                            resultText = "패배";
                            resultColor = "red";
                        }

                        var component = Qt.createComponent("qrc:/ResultWindow.qml");
                        if (component.status === Component.Ready) {
                            //note:  결과 창의 parent를 null로 지정하여 독립적인 창으로 생성했다 (chatWindow로 한다면, chatWindow가 닫혀 부모가 사라질 수 있다)
                            var resultWin = component.createObject(null, {
                                resultText: resultText,
                                resultColor: resultColor
                            });
                            if (resultWin !== null) {
                                resultWin.visible = true;
                            } else {
                                console.log("Error creating result window.");
                            }
                        } else {
                            console.log("Error loading ResultWindow.qml: " + component.errorString());
                        }
                    } else {
                        console.log("chatModel is empty, cannot create result window.");
                    }
                    // chatWindow를 닫습니다.
                    chatWindow.close();
                }
        }
    }
}
