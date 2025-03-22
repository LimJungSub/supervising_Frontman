#ifndef SERVER_INTERACTION_H
#define SERVER_INTERACTION_H

#include <QTcpSocket>
#include <QObject>
#include <QTcpServer>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class ServerInteraction : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentClientNumber READ getClientNumber NOTIFY connectedClientNumberChange)

private:
    QTcpSocket *clientSocket;
    int connectedUser_number = NULL;  // 현재 서버에 연결된 클라이언트 수 저장
    bool isMeWinner = false;

public:
    ServerInteraction(QObject *parent = NULL) {}

    Q_INVOKABLE bool getServerConnectionResult(QString serverAddress, int serverPort){
        clientSocket = new QTcpSocket(this);
        clientSocket->connectToHost(serverAddress, serverPort);

        connect(clientSocket, SIGNAL(connected()), this, SLOT(connectionSuccess()));
        connect(clientSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(connectionFailed(QAbstractSocket::SocketError)));
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(readData()));
    }


    int getClientNumber(){
        return connectedUser_number;
    }

signals:
    void connectionComplete();
    void sendDataToQml(QString msg);
    void connectionFail();
    void waitingForMatch();
    void connectedClientNumberChange();
    void openChatWindow(int roomNum);
    void opponentMessageReceived(QString msg);
    void gameResultReceived(QString msg);

public slots:
    void connectionSuccess(){
        qDebug() << "[Connection Success] Connected to server.";
        emit waitingForMatch();
    }

    //서버로부터 데이터를 받았을 때 처리하는 함수
    void readData() {
        QByteArray receivedData = clientSocket->readAll();

        // 여러 개의 JSON 메시지가 '\n'으로 구분되어 있다면 분할하여 개별 처리
        QList<QByteArray> jsonMessages = receivedData.split('\n');

        for (const QByteArray &jsonMsg : jsonMessages) {
            if (jsonMsg.trimmed().isEmpty()) continue;  // 빈 메시지 무시

            QJsonParseError jsonError;
            QJsonDocument doc = QJsonDocument::fromJson(jsonMsg, &jsonError);

            if (jsonError.error == QJsonParseError::NoError && doc.isObject()) {
                qDebug() << "[ServerInteraction] JSON 메시지 파싱 성공:" << doc.toJson(QJsonDocument::Compact);
                QJsonObject obj = doc.object();
                if (obj.contains("connected_users")) {
                    int newCount = obj["connected_users"].toInt();

                    if (connectedUser_number != newCount) {
                        connectedUser_number = newCount;
                        emit connectedClientNumberChange();  // QML UI 업데이트 (서버에 접속한 클라이언트의 수가 바뀜)
                    }
                }
                else if (obj.contains("matched")) {
                    int roomNum = obj["matched"].toInt();
                    qDebug() << "[ServerInteraction] Matched room number received:" << roomNum;
                    emit openChatWindow(roomNum); // QML UI 업데이트 (서버로부터 매칭받기 성공)
                }
                else if (obj.contains("fromOpponent")) {
                    // 상대방(어떤 클라이언트로부터 받은 메시지)를 채팅창에 기록하기 위한 시그널
                    QString oppMsg = obj["fromOpponent"].toString();
                    qDebug() << "[ServerInteraction] Received opponent message:" << oppMsg;
                    emit opponentMessageReceived(oppMsg);
                }
                if (obj.contains("gameResult")) {
                    // 게임 결과 메시지(강조되어야 함)를 채팅창에 기록하기 위한 시그널
                    QString gameResultMsg = obj["gameResult"].toString();
                    qDebug() << "[ServerInteraction] Received game result message:" << gameResultMsg;
                    if(gameResultMsg.startsWith("승리")){
                        isMeWinner = true;
                    }
                    emit gameResultReceived(gameResultMsg);
                }
            }

            // 전부 json 데이터로 처리하도록 변경하였다
            // else {
            //     // JSON이 아니면 일반 문자열 메시지로 처리
            //     QString dataString = QString::fromUtf8(jsonMsg).trimmed();
            //     if (!dataString.isEmpty()) {
            //         qDebug() << "[ServerInteraction] 일반 메시지: " << dataString;
            //         emit sendDataToQml(dataString);
            //     }
            // }
        }
    }

    void sendData(const QString &message) {
        // TCP 소켓이 존재하고 연결 상태인지 확인
        if (clientSocket && clientSocket->state() == QAbstractSocket::ConnectedState) {
            QByteArray data = message.toUtf8();
            clientSocket->write(data + "\n");
            qDebug() << "[ServerInteraction] Sent data:" << data;
        } else {
            qDebug() << "[ServerInteraction] Cannot send data. Socket not connected.";
        }
    }

    void onSendEndMessageToServer_STM32(int roomNum){
        //승리자와 패배자 중 승리자만 Disconnect 메시지를 보내게한다. 승리자,패배자 둘 다 보내게 하면 안된다.
        //전역변수 isMeWinner 사용, 서버측으로부터 JSON으로 gameResult 받으면 파싱하여 승리자한테만 isMeWinner을 True로 기록하자.
        if(isMeWinner){
            QByteArray str = "[Disconnect]:" + QByteArray::number(roomNum);
            clientSocket->write(str);
            qDebug() << "[onSendEndMessageToServer_STM32] Sent end message:" << str;
        }
    }

    void connectionFailed(QAbstractSocket::SocketError socketError){
        qDebug() << "[Connection Fail] errorOccured. [Error Content] is " << socketError;
        emit connectionFail();
    }
};

#endif // SERVER_INTERACTION_H
