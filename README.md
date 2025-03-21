## 기능 명세

| 항목 구분 | 세부 항목 | 내용 및 상세 설명 |
|---|---|---|
| **하드웨어 구성** | MCU 및 주변 장치 | • STM32F4 Discovery 보드 <br> • 4개 스위치 (Start, Room1, Room2, Room3) <br> • 상태 표시용 LED (3개) <br> • PWM 부저 <br> • UART 통신 (USART2) |
| **물리 통신** | 인터페이스 및 설정 | • 양방향 UART 통신 (STM32 ↔ PC) <br> • 전송 속도: 115200 bps <br> • 8N1 포맷 <br> • 인터럽트 기반 수신 및 Mutex 보호 |
| **시작 시나리오** | 초기화 절차 | • Start 스위치 클릭 <br> • RunQtTask 실행 → `RUN_QT` 전송 <br> • Qt 앱 SerialHandle 수신 <br> • FrontmanWindow.qml 로 채팅 서버 GUI 시작 |
| **방 생성 트리거** | Room 버튼 입력 | • Room 버튼 클릭 → `CMD_QT:roomNum:1` 전송 <br> • ChatRoomTask 에서 LED 점등 및 부저 울림 <br> • Qt 서버 addRoom 시그널 emit |
| **매칭 프로세스** | NetworkHandle 동작 | • addRoom → RandomMatching 실행 <br> • 대기중 클라이언트 2명 랜덤 선택 <br> • 양쪽에 `{ "matched": roomNum }` JSON 전송 <br> • 파트너 관계 QMap 저장 |
| **채팅 및 필터링** | 송수신 및 검열 | • 클라이언트 메시지 수신 <br> • `BadWordHandle::isContainBadWord()` 검사 <br> • 정상 시 상대방에 전달 <br> • 비속어 발견 시 승패 JSON 송신 및 3초 후 자동 disconnect |
| **클라이언트 UI** | QML 창 구성 | • FrontmanWindow.qml: 접속 상태 표시 <br> • ChatWindow.qml: 실시간 채팅 UI <br> • ResultWindow.qml: 승패 안내 및 자동 종료 |
| **종료 프로세스** | 탈락 및 정리 절차 | • 서버에서 `[Disconnect]:roomNum` 송신 <br> • STM32에서 LED OFF 및 부저 울림 <br> • 클라이언트 소켓 종료 및 리소스 정리 |
| **FreeRTOS Task 구조** | Task 기능 | • ChatRoomTask: 큐 수신 후 LED & 부저 <br> • RunQtTask: `RUN_QT` 송신 <br> • UARTReceiveTask: 종료 메시지 수신 & 처리 <br> • BuzzerTask: 부저 출력 |
| **큐 및 리소스 관리** | 구성 | • `uartMutex`: 전송 보호 <br> • ChatRoomQueue, BuzzerQueue, UARTReceiveQueue <br> • 데이터형: uint16_t / uint8_t |
| **에러 처리** | 오류 대응 | • STM32: UART ErrorCallback 자동 복구 <br> • Qt: 잘못된 포맷 & 네트워크 오류 시 qDebug 출력 <br> • 단계별 로그 출력 |

