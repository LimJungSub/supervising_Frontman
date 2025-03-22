#ifndef ROOMHANDLE_H
#define ROOMHANDLE_H

#include <QObject>
#include <QDebug>

//singleton
//RandomMatching, DisconnectRoom은 NetworkHandle에서 한다.
class RoomHandle : public QObject
{
    Q_OBJECT
    //roomCount는 QML이 읽기만 하므로 WRITE를 지정할 필요는 없다
    Q_PROPERTY(int roomCount READ roomCount NOTIFY roomStatusChanged FINAL)

public:
    //Spring에서 배웠던 Factory method같은 형태이다.
    static RoomHandle& instance()
    {
        static RoomHandle s_instance;
        return s_instance;
    }

    // 방 상태 접근 함수 -> 얘는 QML에서 필요없다 UI에 출력을 안하기 떄문
    bool getRoomStatus(int index) const
    {
        return roomStatus[index];
    }

    // QML에서 roomHandle.roomCount로 사용하자.
    int roomCount(){
        int cnt = 0;
        for(int i=0; i<4; i++){
            if(roomStatus[i] == true){
                cnt++;
            }
        }
        return cnt;
    }

    void setRoomStatus(int index, bool value)
    {
        roomStatus[index] = value;
        emit roomStatusChanged();
    }

private:
    RoomHandle(QObject *parent = nullptr)
        : QObject(parent)
    {
        qDebug() << "RoomHandle initialized.";
    }

    // 복사 방지
    RoomHandle(const RoomHandle&) = delete;
    RoomHandle& operator=(const RoomHandle&) = delete;

    bool roomStatus[4] = {false,};  //false이면 해당 방이 만들어지지 않았다는 뜻

signals:
    void roomStatusChanged();

};

#endif // ROOMMANAGER_H
