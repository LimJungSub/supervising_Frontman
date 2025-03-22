#ifndef NETWORKHANDLE_H
#define NETWORKHANDLE_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>

//client 상태 정보도 여기서 관리한다.
class NetworkHandle : public QObject
{
    Q_OBJECT
    //Q_PROPERTY의 READ에 파라미터를 넣어 개별 요소를 반환하게 할 수는 없다.
    //READ만 하므로 Write지정은 불필요하다.
    Q_PROPERTY(QVariantList info READ getInfo NOTIFY infoChanged)
    //QVector<int> info는 .QML에서 해석할 수 없다. QVariantList같은 것을 써야한다.
public:
    explicit NetworkHandle(QObject *parent = nullptr);
    void startServer(int port);

    void changeInfo();
    void setInfo(QVariantList &newInfo);
    QVariantList getInfo();

private:
    QTcpServer* server;
    QTcpSocket* clientSocket;

    //유저ID를 기준으로 클라이언트를 구분한다.
    QMap<int, QTcpSocket*> clientMap;   // 유저 ID , 클라이언트 소켓
    QMap<int, bool> clientStatus;       // 유저 ID , 채팅 참여 여부 (true이면 채팅에 참여중)
    QSet<int> availableIDs = {1, 2, 3, 4, 5, 6};    // 재사용 가능한 ID 목록 (1번~6번ID)

    QVariantList info = {0,0,0};    //초기에 모두 0

    int generateUserID();

    static int totalClientCount; //참여했던 클라이언트의 수를 나타냄 (증가하기만 함)
    static QMap<int,int> whoIsMyPartner;

    int isEndMsg(const QByteArray& data);

signals:
    void matchingCompleted();   //Room Network matching 작업 후 Window 생성 위한 시그널 (채팅방 UI아직 생성X)
    void disconnectCompleted(); //Room Network disconnect 작업 후 Window 삭제 위한 시그널
    void infoChanged(); //QML에 묶음
    void clientMapChanged(); //C++ Slot에 묶음

public slots:
    void RandomMatching(int roomNum);
    void DisconnectRoom(int roomNum);
    void NewConnection();
    void processMessage();
    void clientDisconnected();
    void broadcastUserCount(); //클라이언트로 유저수에 변동이 있을때마다 실시간 데이터를 보낸다.
};

#endif // NETWORKHANDLE_H
