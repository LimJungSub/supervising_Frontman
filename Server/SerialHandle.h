#ifndef SERIALHANDLE_H
#define SERIALHANDLE_H

#include "NetworkHandle.h"
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QtCore/qdebug.h>

class SerialHandle : public QObject {
    Q_OBJECT
public:
    explicit SerialHandle(NetworkHandle* m_network, int networkPort, QObject *parent = nullptr);
    ~SerialHandle();

    void startNetworkServer(int port);
    static void writeData(const QByteArray &data);  /*NetworkHandle에서도 UART출력을 직접적으로 할 것이기 때문에 이렇게 사용
        헤더파일 순환 include를 해결하기 위해 NetworkHandle.cpp에서 SerialHandler을 include (헤더에서 하지 않음)*/
    void processCompleteSerialMessage(QByteArray completeMsg);


public slots:
    void readSerialData_processBuffering();


signals:
    void dataReceived(QString message);  // QML로 데이터 전달 - RUN , ADD, DEL
    void addRoom(int roomNum);
    void delRoom(int roomNum);

private:
    static QSerialPort *m_serial;  // static으로 선언함으로써 writeData할때도 하나의 m_serial만 사용
    NetworkHandle *m_network;
};

#endif // SERIALHANDLER_H
