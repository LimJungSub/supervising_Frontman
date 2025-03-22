import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


ApplicationWindow {

    id: window
    visible: true
    width: 800
    height: 600
    title: qsTr("Frontman")

    ColumnLayout {
        anchors.fill: parent

        // [1] 상단 (전체 높이의 1/3)
        Rectangle {
            Layout.preferredHeight: parent.height * 0.33
            Layout.fillWidth: true
            color: "lightgray"

            Text {
                text: qsTr("Frontman, the Supervisor")
                anchors.centerIn: parent
                font.pixelSize: 35
                font.weight: Font.Black  // 가장 두꺼운 폰트
            }
        }

        // [2] 하단 (나머지 2/3) - 좌우로 분할
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // [2-1] 왼쪽 영역 (6개의 상태 텍스트)
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#333333"

                Column {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        text: qsTr("게임을 진행중인 채팅방 수")
                        color: "white"
                        font.pixelSize: 23
                        font.weight: Font.Black  // 가장 두꺼운 폰트
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }
                    Text {
                        //note: qsTr(): first argument (sourceText) must be a string : QML의 String() 함수는 정수형(int)을 문자열(QString)로 변환해줌.
                        text: qsTr(String(roomHandler.roomCount))
                        color: "white"
                        font.pixelSize: 23
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }

                    //QML에서는 그냥 Q_PROPERTY로 등록한 info(QVariant)를 가져다쓰면 된다. 인덱스로 접근이 가능하다?
                    Text {
                        text: qsTr("참여 클라이언트 수")
                        color: "white"
                        font.pixelSize: 23
                        font.weight: Font.Black
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }
                    Text {
                        text: qsTr(String(networkHandler.info[0] !== undefined ? networkHandler.info[0] : "N/A"))
                        color: "white"
                        font.pixelSize: 23
                        font.weight: Font.Black
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }
                    Text {
                        text: qsTr("생존 클라이언트 수")
                        color: "white"
                        font.pixelSize: 23
                        font.weight: Font.Black
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }
                    Text {
                        text: qsTr(String(networkHandler.info[0] !== undefined ? networkHandler.info[1] : "N/A"))
                        color: "white"
                        font.pixelSize: 23
                        font.weight: Font.Black
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }
                    Text {
                        text: qsTr("탈락 클라이언트 수")
                        color: "white"
                        font.pixelSize: 23
                        font.weight: Font.Black
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }
                    Text {
                        text: qsTr(String(networkHandler.info[0] !== undefined ? networkHandler.info[2] : "N/A")) //Q_INVOKABLE이므로 직접접근하여 사용한다.
                        color: "white"
                        font.pixelSize: 23
                        font.weight: Font.Black
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }
                }
            }

            // [2-2] 오른쪽 영역 (비속어 관리 UI)
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#444444"

                Column {
                    anchors.centerIn: parent
                    spacing: 10

                    Text {
                        text: qsTr("비속어 관리")
                        color: "white"
                        font.pointSize: 25
                        font.weight: Font.Black  // 가장 두꺼운 폰트
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    // ListView에 ScrollBar 추가 (수직 스크롤바 항상 표시)
                    ListView {
                        id: badWordList
                        width: 300
                        height: 200
                        model: ListModel { id: badWordModel }
                        clip: true

                        // 수직 스크롤바 첨부
                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AlwaysOn
                        }

                        delegate: Item {
                            width: badWordList.width
                            height: 35
                            Row {
                                spacing: 10
                                Text {
                                    text: model.word
                                    font.pointSize: 14
                                    color: "white"
                                }
                                Button {
                                    text: qsTr("삭제")
                                    onClicked: {
                                        badWordHandler.removeBadWord(model.word);
                                        refreshBadWords();
                                    }
                                }
                            }
                        }
                    }

                    Row {
                        spacing: 10
                        TextField {
                            id: inputField
                            width: 180
                            height: 30
                            placeholderText: qsTr("비속어 입력")
                        }
                        Button {
                            text: qsTr("추가")
                            onClicked: {
                                if (inputField.text.trim().length > 0) {
                                    badWordHandler.addBadWord(inputField.text);
                                    refreshBadWords();
                                    inputField.text = "";
                                } else {
                                    console.log("비속어 입력 필드가 비어 있습니다.");
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        refreshBadWords();
    }

    function refreshBadWords() {
        if (!badWordModel) {
            console.log("badWordModel이 초기화되지 않았습니다.");
            return;
        }
        badWordModel.clear();
        var words = badWordHandler.getBadWords();
        for (var i = 0; i < words.length; i++) {
            badWordModel.append({ word: words[i] });
        }
        console.log("비속어 목록 갱신 완료. 총 개수:", words.length);
    }
}
