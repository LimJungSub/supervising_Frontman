## 주요 사용 기술

STM32F411RE(Nucleo)

FreeRTOS

QT 6.4 + QML

## 기능 명세서

| 항목 구분 | 세부 항목 | 내용 및 상세 설명 |
|---|---|---|
| **하드웨어 구성** | MCU 및 주변 장치 | • STM32F4 Discovery 보드 <br> • 4개 스위치 (Start, Room1, Room2, Room3) <br> • 상태 LED (Room1 ~ Room3) <br> • PWM 부저 (알림음) <br> • UART 통신용 포트 (USART2) |
| **물리 통신** | 인터페이스 및 설정 | • 양방향 UART (STM32 ↔ PC) <br> • BaudRate: 115200, 8N1 <br> • 수신: 인터럽트 방식 `HAL_UART_Receive_IT()` 사용 <br> • 송신 시 **Safe_UART_Transmit()** 함수를 통해 mutex 보호 (충돌 방지) <br> • mutex 정의: `uartMutexHandle = osMutexNew()` |
| **전송 보호** | 보호 적용 위치 | • `Safe_UART_Transmit()` 함수 내부 <br>   - `osMutexAcquire(uartMutexHandle, osWaitForever)` 호출 <br>   - HAL_UART_Transmit() 사용 후 `osMutexRelease()` <br> • 이유: FreeRTOS 다중 태스크 환경에서 동시 UART 송신 시 충돌 방지 |
| **시작 시나리오** | 절차 및 호출 함수 | • Start 버튼 눌림 (GPIO 인터럽트) <br> • `RunQtTask` 실행, `RUN_QT\n` 전송 <br> • PC Qt App의 `SerialHandle::readSerialData_processBuffering()` 함수에서 수신 <br> • QML `FrontmanWindow` 창 생성 트리거 |
| **방 생성 트리거** | 시리얼 송신 및 처리 | • Room 버튼 클릭 시 `CMD_QT:<roomNum>:1` 문자열 전송 <br> • 전송 함수: `Safe_UART_Transmit()` 사용 <br> • Qt 측 `SerialHandle::processCompleteSerialMessage()` 함수에서 수신 & 파싱 <br> • `addRoom(roomNum)` 시그널 emit |
| **매칭 프로세스** | 랜덤 매칭 함수 및 흐름 | • `NetworkHandle::RandomMatching(roomNum)` 호출 <br> • 대기 클라이언트 2명 랜덤 선택 <br> • JSON `{ "matched": roomNum }` 전송 <br> • `whoIsMyPartner` QMap에 매칭 저장 <br> • 매칭 완료 후 `matchingCompleted()` 시그널 emit (UI 갱신 가능) |
| **비속어 필터링** | 검출 및 조치 | • `NetworkHandle::processMessage()` 내 호출: `BadWordHandle::isContainBadWord()` <br> • 비속어 발견 시: <br>  &nbsp;• 승자에게 `{gameResult: 승리}` JSON 전송 <br>  &nbsp;• 패자에게 `{gameResult: 패배}` JSON 전송 <br> • 승패 결정난 후 QTimer(3초 후) 두 소켓 `disconnectFromHost()` 호출 -> 강제 종료 |
| **클라이언트 UI** | QML 파일 및 동작 | • `Init.qml`: 로그인 입력 창 <br> • `FrontmanWindow.qml`: 대기 및 사용자 현황 표시 <br> • `ChatWindow.qml`: 채팅방 창 <br> • `ResultWindow.qml`: 게임 종료 결과 및 자동 창 닫힘 표시 |
| **종료 프로세스** | 탈락 처리 및 통신 | • 서버가 패자 판단 후 `[Disconnect]:roomNum` 전송 <br> • STM32 `UARTReceiveTaskFunc()` 에서 수신 및 파싱 <br> • 해당 방 LED OFF, buzzerQueue 에 부저 명령 전달 <br> • PC 측 클라이언트 소켓 3초 후 강제 종료 |
| **FreeRTOS Task** | 각 태스크 기능 상세 | • **ChatRoomTask**: 버튼 입력 큐 읽기 → LED 상태 변경 및 부저 큐 전송 <br> • **RunQtTask**: start 플래그 세트 시 `RUN_QT` 전송 <br> • **UARTReceiveTask**: Disconnect 수신 시 LED OFF <br> • **BuzzerTask**: 큐 기반으로 특정 채널 부저 울림 |
| **큐 및 보호 리소스** | 정의 및 사용 위치 | • `uartMutex`: `Safe_UART_Transmit()`에서 사용, STM32<->QT 양방향 송수신 시 필요했음 <br> • `ChatRoomQueue`: GPIO 버튼 입력 큐 <br> • `UARTReceiveQueue`: 종료 메시지 수신용 큐 <br> • `BuzzerQueue`: 부저 출력 요청 큐 <br> • 각 큐 크기 16개, 데이터형: uint16_t 또는 uint8_t |
| **예외 및 디버깅 처리** | 구현 방식 | • STM32: `HAL_UART_ErrorCallback()` 호출 시 자동 수신 재시작 <br> • Qt 서버: 잘못된 메시지/네트워크 오류 시 qDebug 로그 <br> • 각 단계에서 Debug_UART_Print() 및 qDebug() 호출 |
