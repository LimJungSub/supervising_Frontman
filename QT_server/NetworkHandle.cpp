#include "NetworkHandle.h"
#include <SerialHandle.h>
#include <RoomHandle.h>
#include <BadWordHandle.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>

int NetworkHandle::totalClientCount = 0;
QMap<int,int> NetworkHandle::whoIsMyPartner; //QMap의 기본 생성자 호출

//생성자 호출은 SerialHandler의 생성자에서,
NetworkHandle::NetworkHandle(QObject *parent)
    : QObject{parent}
{
    server = new QTcpServer(this);

    // 랜덤매칭을 위한 난수 생성 초기화 (프로그램 시작 시 한 번만 호출해도되므로 여기서)
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 새 연결이 생기면 onNewConnection() 슬롯 호출
    connect(server, &QTcpServer::newConnection, this, &NetworkHandle::NewConnection);
    connect(this, &NetworkHandle::clientMapChanged, this, &NetworkHandle::broadcastUserCount);
}


void NetworkHandle::NewConnection(){
    qDebug() << "[NetworkHandle] New client connected, initializing new client... ";
    clientSocket = server->nextPendingConnection();

    if (!clientSocket) return;

    // 클라이언트의 채팅데이터 및 연결 종료 시 처리할 슬롯을 connect
    connect(clientSocket, &QTcpSocket::readyRead, this, &NetworkHandle::processMessage);
    connect(clientSocket, &QTcpSocket::disconnected, this, &NetworkHandle::clientDisconnected);


    int userID = generateUserID();  // 유저 ID 받음
    clientMap[userID] = clientSocket;
    clientStatus[userID] = false;  // 기본적으로 채팅 미참여 상태
    totalClientCount++;

    //emit clientMapChanged()는 clientMap이 바뀌는 경우에만 호출한다. Room관련된 것은 RoomHandle에서 처리
    if (clientMap.size() > 0) {
        emit clientMapChanged(); // -> c++ slot : broadcastUserCount (클라이언트의 QML 변경, 클라이언트가 남아있을때만 클라이언트의 UI를 바꿔주니 클라이언트가 한명이라도 있을때만 emit하는게 효과적)
    }
    changeInfo(); // -> qml slot : FrontmanWindow.qml의 UI를 변경
}


 //Onenote에 동작을 적었다.
int NetworkHandle::generateUserID() {
    static int lastID = 0;

    if (!availableIDs.isEmpty()) {
        // 이전에 삭제된 ID 재사용
        auto it = availableIDs.begin();
        int reusedID = *it;
        availableIDs.erase(it);
        return reusedID;
    }

    return ++lastID;  // 새로운 ID 할당 (lastId)가 증가하는 경우는 원소가 없었을때만이다.
}

//클라이언트로부터 받은 메시지가 EndMsg라면 roomNum까지 파싱하여 알아낸다. EndMsg가 아니라면 0을 리턴한다.
int NetworkHandle::isEndMsg(const QByteArray& data){
    if (data.startsWith("[Disconnect]:")) {
        QList<QByteArray> parts = data.split(':');
        if (parts.size() == 2) {
            bool ok = false;
            int roomNum = parts[1].toInt(&ok); //toInt를 썼다. 즉 파싱가능한경우에만 return roomNum;
            if (ok) {
                return roomNum;
            }
        }
    }
    return 0;
}

void NetworkHandle::processMessage() {
    QTcpSocket *senderSocket = qobject_cast<QTcpSocket*>(sender());  // 현재 시그널을 보낸 소켓을 찾는다.

    if (!senderSocket) return;

    QByteArray data = senderSocket->readAll();
    int roomNum;
    if((roomNum = isEndMsg(data))){
        //STM32로의 직접 UART 전송, return
        //QString을 사용한 후 toUtf8()로 QByteArray로 변환
        QString message = QString("%1\n").arg(roomNum);
        SerialHandle::writeData(message.toUtf8());
        return;
    }

    //서로 매칭된 클라이언트끼리 데이터를 전송해야한다. 짝을 잘 찾아 데이터를 전송해야한다. (Unicast느낌)
    // 보낸 클라이언트의 ID 찾기 (clientMap에서)
    int senderId = -1;
    for (auto it = clientMap.begin(); it != clientMap.end(); ++it) {
        if (it.value() == senderSocket) {
            senderId = it.key();
            break;
        }
    }

    qDebug() << "Data received from client:" << senderId << " Data:" << data;

    // 비속어 필터링 (Utf8로 변환하고해야함!!)
    QString message = QString::fromUtf8(data);
    bool isBadMsg = BadWordHandle::isContainBadWord(message);

    // 매칭된 상대방 클라이언트 ID 찾기
    if (!whoIsMyPartner.contains(senderId)) {
        qDebug() << "No matched partner for sender id:" << senderId;
        return;
    }
    int partnerId = whoIsMyPartner.value(senderId);

    // 상대방의 소켓 찾기
    if (!clientMap.contains(partnerId)) {
        qDebug() << "Matched partner id not found in clientMap.";
        return;
    }
    QTcpSocket *partnerSocket = clientMap.value(partnerId);

    // 메시지를 전송 (Unicast)
    QJsonObject receiverJSON;
    QJsonObject senderJSON;
    if (!isBadMsg) {
        // 비속어 미포함: 일반적인 데이터 전송
        receiverJSON.insert("fromOpponent", message);
        QJsonDocument docReceiver(receiverJSON);
        QByteArray jsonDataWinner = docReceiver.toJson(QJsonDocument::Compact) + "\n";
        qDebug() << "Forwarded legal message to partner (ID:" << partnerId << "):" << data;
        partnerSocket->write(jsonDataWinner);
    }
    else {
        qDebug() << "Game will be ended due to illegal message.";

        // 승리자(파트너)에게 JSON 메시지 전송
        receiverJSON.insert("fromOpponent", message);
        receiverJSON.insert("gameResult", "승리, 3초 후 창이 자동으로 닫힙니다.");
        QJsonDocument docWinner(receiverJSON);
        QByteArray jsonDataWinner = docWinner.toJson(QJsonDocument::Compact) + "\n";
        partnerSocket->write(jsonDataWinner);
        qDebug() << "Sent JSON to partner (winner):" << jsonDataWinner;
        // 승리자에 대한 clientMap, clientStatus, availableClientId 처리 : 비제거(무동작), status 0, 무동작
        clientStatus[partnerId] = false;
        qDebug() << "[processMessage] Winner (ID: " << partnerId << ") remains in the system.";


        // 패배자(발신자)에게도 JSON 메시지 전송, 패배자는 UI에 자신이 보낸 메시지가 보이는 상태이므로 따로 자신이 보내는 메시지를 다시 받지는 않음
        senderJSON.insert("gameResult", "패배, 3초 후 창이 자동으로 닫힙니다.");
        QJsonDocument docLoser(senderJSON);
        QByteArray jsonDataLoser = docLoser.toJson(QJsonDocument::Compact) + "\n";
        senderSocket->write(jsonDataLoser);
        qDebug() << "Sent JSON to sender (loser):" << jsonDataLoser;
        // 패배자에 대한 clientMap, clientStatus, availableClientId 처리 : 제거, 제거, 제거
        clientMap.remove(senderId);
        clientStatus.remove(senderId);
        availableIDs.insert(senderId);
        qDebug() << "[processMessage] Loser (ID: " << senderId << ") removed from system.";

        // 3초 후, 비동기적으로 두 소켓 모두 연결 종료 (QTimer 사용), SLOT을 람다식형태로 작성했다
        QTimer::singleShot(3000, partnerSocket, [partnerSocket]() {
            partnerSocket->disconnectFromHost();
        });
        QTimer::singleShot(3000, senderSocket, [partnerSocket]() {
            partnerSocket->disconnectFromHost();
        });

        // 게임의 결과가 정해졌으므로, STM32로 UART를 통해 roomNum와 Disconnect 문자열을 포함한 데이터를 보내야한다.
        // 그러기 위해선 해당 사용자가 속한 방 번호를 찾아야한다.
        // 아... 그러면 애초에 방 생성 할때부터 참가자몇번이 몇번방에 속한다는 데이터셋을 마련해야하는데, 귀찮은 작업이다.
        // 그러므로 구현의 단순함을 위해 클라이언트측에서 메시지로 NetworkHandler로 방번호를 포함한 데이터를 보내게하였다.

    }
}

//유저가 연결이 끊겼을 때 처리하는 함수
void NetworkHandle::clientDisconnected(){
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());

    if (!client) return;

    int userID = -1;

    // userID 찾기
    for (auto it = clientMap.begin(); it != clientMap.end(); ++it) {
        if (it.value() == client) {
            userID = it.key();
            break;
        }
    }

    // if (userID != -1) {
    //     clientMap.remove(userID);
    //     clientStatus.remove(userID);
    //     //별도로해주어야하는것: 클라이언트가 탈락되면 availableIDs에 꼭 다시 등록해주어야한다. (원노트 id계산 참고)
    //     availableIDs.insert(userID);
    //     qDebug() << "Client disconnected. ID:" << userID;
    // }

    if (userID == -1) return;

    // // **승리자라면 탈락자로 기록되지 않도록 그냥 리턴**
    // if (clientStatus.contains(userID) && clientStatus[userID] == true) {
    //     qDebug() << "User ID " << userID << " is a winner. Not marking as eliminated.";
    //     return;
    // }

    // // **일반적인 클라이언트 제거 로직**
    // clientMap.remove(userID);
    // clientStatus.remove(userID);
    // availableIDs.insert(userID);
    // qDebug() << "Client disconnected. ID:"; //process~에서 수행

    //emit clientMapChanged()는 clientMap이 바뀌는 경우에만 호출한다. Room관련된 것은 RoomHandle에서 처리
    if (clientMap.size() > 0) {
        emit clientMapChanged(); // -> c++ slot : broadcastUserCount (클라이언트의 QML 변경, 클라이언트가 남아있을때만 클라이언트의 UI를 바꿔주니 클라이언트가 한명이라도 있을때만 emit하는게 효과적)
    }
    changeInfo(); // -> qml slot : FrontmanWindow.qml의 UI를 변경

    client->deleteLater();
}


void NetworkHandle::startServer(int port){
    if (!server->listen(QHostAddress::LocalHost, port)) {
        qDebug() << "Server could not start:" << server->errorString();
    }
    else {
        qDebug() << "Server started on loopback address (127.0.0.1) at port:" << port;
    }
}


void NetworkHandle::RandomMatching(int roomNum){

    qDebug() << "Testing : Room matching is valid? (RoomNum:" << roomNum << ")";
    //유저의 목록을 가져와서, 랜덤매칭
    QList<int> availableClients;
    //참여
    for (auto it = clientStatus.begin(); it != clientStatus.end(); ++it) {
        if (!it.value()) { // true이면 채팅에 참여중이므로 false인 것들을 available에 담는다.
            availableClients.append(it.key());
        }
    }
    //0~리스트크기-1 범위에서 인덱스 2개를 랜덤으로 뽑아서 매칭한다.
    int count = availableClients.size();
    if (count >= 2) {
        qDebug() << "Room matching start (RoomNum:" << roomNum << ")";
        int idx1 = std::rand() % count;
        int idx2 = std::rand() % count;
        while (idx2 == idx1) {
            idx2 = std::rand() % count;
        }

        // 선택된 두 클라이언트를 매칭 처리
        int client1_id = availableClients.at(idx1);
        int client2_id = availableClients.at(idx2);

        //두 클라이언트에게 메시지 전송(map에서 해당하는 id의 value에 wrtie)
        QJsonObject json;
        json.insert("matched", roomNum);
        QJsonDocument doc(json);
        QByteArray match_msg = doc.toJson(QJsonDocument::Compact) + "\n";
        qDebug() << match_msg << " <- this message will be written to each client (id:" << client1_id << "," << client2_id << ")";
        clientMap[client1_id]->write(match_msg);
        clientMap[client2_id]->write(match_msg);

        whoIsMyPartner[client1_id] = client2_id;
        whoIsMyPartner[client2_id] = client1_id;

        //방 번호에 체크 표시
        RoomHandle::instance().setRoomStatus(roomNum, true);
        qDebug() << "Room matching completed, emit to generate new room window (RoomNum:" << roomNum << ")";

        //UI업데이트를 위한 signal emit (아직 연결 안함)
        emit matchingCompleted();

    }
    else{
        qDebug() << "too few people to participate in chatting (RoomNum:" << roomNum << ")";
        //같은 메시지를 UART로 전송하는 부분도 추가
        //SerialHandle::writeData("too few people to participate in chatting");
    }
}

//QT->STM32로 disconnect신호 전송 시에는 SerialHandler에 직접 roomNum을 바로 전달할것이다. 굳이 이 슬롯을 거칠필요없다.
//다만 STM32->QT로 disconnect 전송 시에 이 슬롯이 필요하다.
void NetworkHandle::DisconnectRoom(int roomNum){
    //유저가 속한 Room의 Status를 바꾸고, 나머지 클라이언트 한명을 대기(Ready)상태로 만든다.
    //유저가 어떤 방에 속했는지 아는 법
        //RandomMatching(int roomNum)을 누가, 어디서 호출? (roomNum은 1,2,3)
        //어떤 클라이언트ID가 어떤 ROOMNUM에 연결됐는지 유지하는 정보가 필요하다.

}



// 기존 getStatusInfo()와 비슷한 로직을 수행하는 함수
void NetworkHandle::changeInfo() {
    QVariantList newInfo;
    newInfo.push_back(totalClientCount);            // 전체 클라이언트의 수
    newInfo.push_back(clientMap.size());              // 생존 클라이언트의 수
    newInfo.push_back(totalClientCount - clientMap.size());  // 탈락 클라이언트의 수

    setInfo(newInfo);
}


void NetworkHandle::setInfo(QVariantList &newInfo) {
    //note: if (info != newInfo) 구문은 QList (즉, QVariantList)의 내장 비교 연산자를 사용하기 때문에 원소들을 한 개씩 비교(모든 대응하는 원소들이 operator==로 같을 경우에만 같다고 판단)
    if (info != newInfo) {
        info = newInfo;
        emit infoChanged();
    }
}



QVariantList NetworkHandle::getInfo(){
    return info;
}


//for to Client program
void NetworkHandle::broadcastUserCount() {
    int userCount = clientMap.size();  // 현재 접속한 유저 수

    // JSON 데이터 생성
    QJsonObject json;
    json["connected_users"] = userCount;
    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    for (auto client : clientMap) {
        client->write(jsonData + "\n");
        qDebug() << "client: " << client << " write jsonData: " << jsonData;
    }
}
