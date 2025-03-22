#include "SerialHandle.h"
#include "RoomHandle.h"
#include "QThread"

QSerialPort* SerialHandle::m_serial = nullptr;

SerialHandle::SerialHandle(NetworkHandle* m_network, int tcpPort, QObject *parent)
    : QObject(parent)
{
    // QSerialPort를 동적 할당하고, this(SerialHandler)를 부모로 설정
    //m_serial = new QSerialPort(this);
    if (!m_serial) {
        qDebug() << "m_serial is now absent, will be create soon.";
        m_serial = new QSerialPort();
    }
    qDebug() << "foreach before... m_serial is present";
    // 사용 가능한 시리얼 포트 자동 검색
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if (info.portName().contains("usbmodem")) { //내가 맥에서 사용하는 포트 정보이다 (SerialTools로 확인)
            m_serial->setPort(info);
            break;
        }
    }

    // 시리얼 포트 설정
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    // readyRead 시그널과 readSerialData() 슬롯 연결
    connect(m_serial, &QSerialPort::readyRead, this, &SerialHandle::readSerialData_processBuffering); //readyRead - 원노트 참고

    connect(this, &SerialHandle::addRoom, m_network , &NetworkHandle::RandomMatching); //시그널과 슬롯에 모두 roomNum인자가 있다. 잘 넘어간다.
    connect(this, &SerialHandle::delRoom, m_network , &NetworkHandle::DisconnectRoom);

    // 포트 열기
    m_serial->open(QIODevice::ReadWrite);

    // SerialPort가 잘 열렸다면, 네트워크 통신 시작
    if (m_serial->isOpen() && m_network) {
        m_network->startServer(tcpPort);
    }
}

SerialHandle::~SerialHandle()
{
    // 소멸자에서 포트 닫기 (열려있다면)
    if (m_serial && m_serial->isOpen()) {
        m_serial->close();
    }
}

void SerialHandle::readSerialData_processBuffering()
{
    // 수신된 데이터를 저장할 버퍼
    static QByteArray buffer;

    // 클래스 멤버 m_serial을 사용하여 데이터 읽기
    QByteArray data = m_serial->readAll();
    buffer.append(data);  // 받은 데이터를 버퍼에 추가

    // 개행 문자를 기준으로 메시지를 분리
    while (buffer.contains("\n")) {
        int index = buffer.indexOf("\n");  // 개행 문자의 위치 찾기
        QByteArray message = buffer.left(index).trimmed();  // 개행 문자 앞까지 메시지 추출

        if (!message.isEmpty()) {  //완전한 메시지라는 뜻,
            qDebug() << "Full Message Received:" << message;
            processCompleteSerialMessage(message);  // 이때는 메시지 처리 함수 호출
        }

        buffer.remove(0, index + 1);  // 처리된 메시지를 버퍼에서 제거
        //여러 개의 메시지가 한 번에 들어와도 while 루프가 개행 문자(\n)를 기준으로 하나씩 올바르게 분리.
        //마지막에 처리되지 않은 데이터가 남아 있으면 buffer에 유지 -> 해당 buffer는 static이므로 다음 수신될 데이터와 함께 처리 가능
    }

    qDebug() << "[processBuffering] current buffer: " << buffer;
}

void SerialHandle::processCompleteSerialMessage(QByteArray completeMsg){
    // 특정 키워드가 포함되어 있으면 시그널로 알림
    if (completeMsg.startsWith("RUN_QT")) {
        emit dataReceived("RUN_QT");  // Frontman Window 띄우기
    }
    else if(completeMsg.startsWith("CMD_QT")){
        //콜론 뒤에가 0이면 Del, 1이면 Add 시그널 발생
        // 데이터 포맷: CMD_QT:0 또는 CMD_QT:1
        QList<QByteArray> parts = completeMsg.split(':');  // ':' 기준 분리

        if (parts.size() == 2) {  // CMD_QT와 값으로 분리되었는지 확인
            QString command = parts[1].trimmed();
            int roomNum = parts[1].toInt();

            qDebug() << "[processCompleteSerialMessage] cmd: " << command;

            if (command == "1") {
                if(!RoomHandle::instance().getRoomStatus(roomNum)){ //false: 방이 안만들어졌다이므로 false일때 addRoom
                    qDebug() << "[processCompleteSerialMessage] AddRoom signal emitted.";
                    emit addRoom(roomNum);
                }
                else{
                    qDebug() << "[processCompleteSerialMessage] Room status is not fit in your command.";
                }
            }
            else if (command == "0") {
                if(!RoomHandle::instance().getRoomStatus(roomNum)){
                    qDebug() << "[processCompleteSerialMessage] DelRoom signal emitted.";
                    emit delRoom(roomNum);
                }
                else{
                    qDebug() << "[processCompleteSerialMessage] Room status is not fit in your command.";
                }
            }
            else {
                qDebug() << "Unknown command received:" << command;
            }
        }
        else {
            qDebug() << "[processCompleteSerialMessage] Invalid CMD_QT format received:" << completeMsg;
        }
    }
}

void SerialHandle::writeData(const QByteArray &data){
    if (m_serial->isOpen()) {
        qint64 bytesWritten = m_serial->write(data);

        if (bytesWritten == -1) {
            qDebug() << "[SerialHandle] UART Writing Failed!";
            return;
        }

        //QSerialPort::bytesToWrite()를 활용하여 실제 전송이 완료될 때까지 기다린다.
        if (!m_serial->waitForBytesWritten(100)) {
            qDebug() << "[SerialHandle] UART Writing Timeout!";
        }

        qDebug() << "[SerialHandle] UART Written Data: " << data;
    }
    else {
        qDebug() << "[SerialHandle] UART Port is not open. Writing failed.";
    }
}
