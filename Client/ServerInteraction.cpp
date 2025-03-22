#ifndef SERVER_INTERACTION_H
#define SERVER_INTERACTION_H

#include <QTcpSocket>
#include <QObject>
#include <QTcpServer>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class serv_interaction : public QObject
{
    Q_OBJECT
private:
    QTcpSocket *m_tcpSocket;
    QString userName;
    bool isAcceptedUser; // [초기 유저명을 통한 검증과 이후 서버의 일반적인 데이터전송을 나눠서 처리하기 위해 등록한 멤버변수](하나의 시그널에 두가지슬롯을 등록후 입맛따라 슬롯을 호출하는것은 불가능하기때문에 따로 이런 변수 마련)
    QVariantMap userMap;

public:
    serv_interaction(QObject *parent = NULL){
        isAcceptedUser = false;
    }

    Q_INVOKABLE bool getServerConnectionResult(QString username, QString serverAddress, int serverPort){
        m_tcpSocket = new QTcpSocket(this);
        this->userName = username;
        m_tcpSocket->connectToHost(serverAddress, serverPort);

        /* 아래 두 QTcp의 signal은 자동발생하므로 따로 emit필요 없다 */
        /* 생성자에서 했을때에는 아마 m_tcpSocket이 초기화되지 않아서 에러나는것 같기도 하다*/
        // 연결 성공 시 유저 이름 검증 작업을 수행하게 한다.(시그널 슬롯 메커니즘)
        connect(m_tcpSocket, SIGNAL(connected()), this, SLOT(sendUserName_getResult()));

        //연결 실패 시, 로그를 출력하고 QML에 팝업창을 띄우자.
        connect(m_tcpSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(connectionFailed(QAbstractSocket::SocketError)));

        // 읽을 데이터가 있을 때 readData 사용
        connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
    }

    Q_INVOKABLE QList<QString> _keys() const {
        return userMap.keys();
    }

    Q_INVOKABLE QVariant _values(QString userName) const{
        qDebug() << "[serv_Interaction.h] _vaules(" << userName << ") is called.";
        qDebug() << "[serv_Interaction.h] _vaules : " << userMap[userName] << " is return value.";
        return userMap[userName];
    }

signals:
    void imRegisteredUser();
    void imNotRegisteredUser();
    void sendDataToQml(QString msg);
    void connectionFail(); //시그널 to 시그널로 연결된 시그널로, errorOccured가 발생하면 자동으로 발생하니 emit할 필요 X
    void changeUserMap(QVariantMap newUserMap);

private slots:

    void sendUserName_getResult(){
        int bytesWritten = m_tcpSocket->write((this->userName).toUtf8()+"\n"); //서버측에서 canReadLine으로 읽으므로 \n붙이기
        m_tcpSocket->flush();  // 데이터를 바로 전송하도록 강제해야 아마 잘 인식하지 않을까 싶다.
        qDebug() << "[Connection Success / ready to be verified] " <<(this->userName).toUtf8() << " is toUtf8() applied userName. Sending to server. " << bytesWritten <<  "bytes written. ";
    }

    void readData(){
        //유저명을 통한 검증이 이루어지지 않은 상태에서 유저명 검증값 받음
        if(isAcceptedUser == false){
            QString msg = QString(m_tcpSocket->readAll());
            if(msg == "Accepted\n"){    //qml에 성공 메시지 전달
                qDebug() << "[Connection Success] From server, Accepted message is received.";
                isAcceptedUser = true;
                emit imRegisteredUser(); //QML에서 메인대화방 입장
            }
            else if(msg == "Rejected\n"){
                qDebug() << "[Connection Success] From server, Rejected message is received.";
                emit imNotRegisteredUser(); //QML에서 실패메시지 팝업
            }
            else{
                qDebug() << "[Connection Success] From server, other message is received.";
            }
        }
        //유저명 검증이 이루어진 상황에서 서버가 보내는 일반적인 데이터를 처리함(자신->자신은 미리 서버에서 걸러줬다)
        else{
            QByteArray receivedData = m_tcpSocket->readAll();
            QString dataString = QString::fromUtf8(receivedData);
            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(receivedData, &parseError);
            qDebug() << "jsonDoc: "<<jsonDoc;
            // json처리
            if (parseError.error == QJsonParseError::NoError && jsonDoc.isObject()) {
                QJsonObject jsonObj = jsonDoc.object();

                if (jsonObj.contains("userMap")) {
                    // userMap 처리
                    QJsonObject userMapObj = jsonObj["userMap"].toObject();

                    // QJsonObject를 QVariantMap으로 변환하여 QML에서 처리할수있게하자
                    for (auto it = userMapObj.constBegin(); it != userMapObj.constEnd(); ++it) {
                        userMap[it.key()] = it.value().toVariant();
                    }
                    // 유저 목록 업데이트
                    qDebug() << "Generated userMap:" << userMap;
                    emit changeUserMap(userMap);
                } else{
                    qDebug() << "Data parsing failed:" << parseError.errorString();
                }
            }
            //일반 메시지 처리
            else {
                qDebug() << "normal message received.";
            }
        }
    }

    void connectionFailed(QAbstractSocket::SocketError socketError){
        qDebug() << "[Connection Fail] errorOccured. [Error Content] is " << socketError;
        emit connectionFail();
    }

};

#endif // SERVER_INTERACTION_H
