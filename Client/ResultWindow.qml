import QtQuick 2.15
import QtQuick.Controls 2.15

Window {
    id: resultWindow
    modality: Qt.ApplicationModal
    visible: false
    width: 400
    height: 300
    title: "Game Result"

    // 결과 텍스트와 색상을 외부에서 ChatWindow.qml에서 설정할 수 있도록 property로 선언했다.
    property string resultText: ""
    property color resultColor: "black"

    Column {
        anchors.centerIn: parent
        spacing: 20

        // 결과 메시지를 중앙에 크게, 볼드체로 표시하였다.
        Text {
            id: resultLabel
            text: resultText
            color: resultColor
            font.bold: true
            font.pixelSize: 32
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }

        // 종료 버튼 (중앙에 배치)
        Button {
            id: closeButton
            text: "종료"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: resultWindow.close()
        }
    }
}
