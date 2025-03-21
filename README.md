항목 구분

세부 항목

내용 및 상세 설명

하드웨어 구성

MCU 및 주변 장치

• STM32F4 Discovery 보드   • 4개 스위치 (Start, Room1, Room2, Room3)   • 상태 표시용 LED (각 방별 3개)   • PWM 부저 (음성 알림 기능)   • UART 통신 포트 (USART2)

물리 통신

인터페이스 및 설정

• 양방향 UART 통신 (STM32 ↔ PC)   • 전송 속도: 115200 bps   • 8N1 포맷 (8-bit data, no parity, 1 stop bit)   • 인터럽트 기반 수신 처리   • 송신 시 mutex 보호로 충돌 방지 구현

시작 시나리오

초기화 절차

• Start 스위치 클릭   • RunQtTask 실행 후 RUN_QT 전송   • PC 측 Qt 앱에서 SerialHandle 수신   • FrontmanWindow.qml 로 채팅 서버 초기화 및 GUI 표시 시작

방 생성 트리거

Room 버튼 입력 처리

• Room1~3 스위치 클릭 시:     CMD_QT:roomNum:1 UART 송신   • ChatRoomTask에서 LED 점등 및 부저 알림   • Qt 서버 SerialHandle에서 파싱 후 addRoom 시그널 emit

매칭 프로세스 (서버)

NetworkHandle 동작 흐름

• addRoom 호출 시 RandomMatching(roomNum) 실행   • 대기 중 클라이언트 2명 랜덤 선택   • 양쪽 클라이언트에 JSON { "matched": roomNum } 전송   • whoIsMyPartner QMap에 매칭 정보 등록

채팅 및 필터링

송수신 로직과 검열 과정

• 클라이언트 메시지 수신   • 비속어 검출: BadWordHandle::isContainBadWord()   • 정상 메시지 → 상대방에 JSON 전달   • 비속어 발견 시: 승/패 JSON 송신 및 3초 후 자동 disconnect 예약

클라이언트 UI

QML 창 구성

• FrontmanWindow.qml: 사용자 상태 및 접속 현황 표시   • ChatWindow.qml: 실시간 채팅 인터페이스   • ResultWindow.qml: 승패 결과 및 종료 안내 (자동 닫힘)

종료 프로세스

탈락 처리 및 후속 절차

• 서버 측 탈락 판단 시 [Disconnect]:roomNum 송신   • STM32 UARTReceiveTask 수신 후 LED OFF   • BuzzerTask 실행 (경고음 출력)   • 클라이언트 소켓 3초 후 강제 종료 예약 및 정리

FreeRTOS Task 구조

각 Task 상세 기능

• ChatRoomTask: 버튼 입력 큐 수신, LED 상태 제어, BuzzerQueue 전송   • RunQtTask: Start 버튼 누르면 RUN_QT 송신   • UARTReceiveTask: QT 메시지 수신 후 LED 및 상태 초기화   • BuzzerTask: BuzzerQueue 기반 주파수 출력 및 멜로디 실행

큐 및 리소스 관리

Queue/MUTEX 세부 설정

• uartMutex: USART 송신 보호   • ChatRoomQueue: 버튼 입력용 큐 (크기 16)   • UARTReceiveQueue: 종료 메시지 큐   • BuzzerQueue: 부저 신호 큐 (크기 16)   • 데이터형: uint16_t / uint8_t

에러 및 예외 처리

오류 대응 및 디버깅 방식

• STM32: UART 오류 시 ErrorCallback() 통해 자동 재수신 재개   • Qt 서버: 잘못된 포맷 / 네트워크 오류 발생 시 qDebug 출력   • 모든 주요 프로세스 단계별 디버깅 로그 및 qDebug 메시지 포함



