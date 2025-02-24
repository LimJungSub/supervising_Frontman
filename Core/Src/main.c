/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* Definitions for ChatRoomTask */
osThreadId_t ChatRoomTaskHandle;
const osThreadAttr_t ChatRoomTask_attributes = {
  .name = "ChatRoomTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for RunQtTask */
osThreadId_t RunQtTaskHandle;
const osThreadAttr_t RunQtTask_attributes = {
  .name = "RunQtTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for BuzzerTask */
osThreadId_t BuzzerTaskHandle;
const osThreadAttr_t BuzzerTask_attributes = {
  .name = "BuzzerTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for ChatRoomQueue */
osMessageQueueId_t ChatRoomQueueHandle;
const osMessageQueueAttr_t ChatRoomQueue_attributes = {
  .name = "ChatRoomQueue"
};
/* Definitions for BuzzerQueue */
osMessageQueueId_t BuzzerQueueHandle;
const osMessageQueueAttr_t BuzzerQueue_attributes = {
  .name = "BuzzerQueue"
};
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
void ChatRoomTaskFunc(void *argument);
void RunQtTaskFunc(void *argument);
void BuzzerTaskFunc(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
typedef enum {
	BUZZER_ALL = 0, BUZZER_1 = 1, BUZZER_2 = 2, BUZZER_3 = 3
} BuzzerID_t;


typedef enum {
	DO = 261,    // 도 (C) - 261.63 Hz
	RE = 293,    // 레 (D) - 293.66 Hz
	MI = 329,    // 미 (E) - 329.63 Hz
} NoteFrequency;


typedef struct {
    uint8_t roomNum;  // 채팅방 번호 (1, 2, 3 등)
    uint8_t state;    // 상태: 1이면 활성(On), 0이면 비활성(Off)
} ChatRoomObj;

// 각 채팅방의 상태를 저장 (0: 비활성, 1: 활성)
volatile uint8_t chatRoomStatus[3] = {0, 0, 0};


static void UWriteData(const char data);
int __io_putchar(int ch);


void Set_Buzzer_Frequency(uint8_t buzzer_id);
void Stop_Buzzer(uint8_t buzzer_id);
void Play_Melody(uint8_t buzzer_id);


volatile uint32_t lastDebounceTime = 0;
const uint32_t debounceDelay = 250; // 250ms 디바운싱 시간으로 좀 길게 잡아본다.

uint8_t debouncing_cnt_sw1 = 0; //디바운싱 카운트용 변수
uint8_t debouncing_cnt_sw2 = 0;
uint8_t debouncing_cnt_sw3 = 0;
uint8_t debouncing_cnt_sw4 = 0;


volatile isQtRunning = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	setbuf(stdout, NULL); //버퍼 없이 즉시 출력 - UART

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of ChatRoomQueue */
  ChatRoomQueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &ChatRoomQueue_attributes);

  /* creation of BuzzerQueue */
  BuzzerQueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &BuzzerQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of ChatRoomTask */
  ChatRoomTaskHandle = osThreadNew(ChatRoomTaskFunc, NULL, &ChatRoomTask_attributes);

  /* creation of RunQtTask */
  RunQtTaskHandle = osThreadNew(RunQtTaskFunc, NULL, &RunQtTask_attributes);

  /* creation of BuzzerTask */
  BuzzerTaskHandle = osThreadNew(BuzzerTaskFunc, NULL, &BuzzerTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 83;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Chat1_R_LED_Pin|Chat2_Y_LED_Pin|Chat3_G_LED_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Chat1_R_LED_Pin Chat2_Y_LED_Pin Chat3_G_LED_Pin LD2_Pin */
  GPIO_InitStruct.Pin = Chat1_R_LED_Pin|Chat2_Y_LED_Pin|Chat3_G_LED_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : StartButton_Pin Room1Button_Pin Room3Button_Pin */
  GPIO_InitStruct.Pin = StartButton_Pin|Room1Button_Pin|Room3Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Room2Button_Pin */
  GPIO_InitStruct.Pin = Room2Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Room2Button_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	//시작해야하는 Task를 여기서 호출해주면 될 것이다.
	/*
	 Start -> RunQt
	 Other -> GenerateChattingRoom
	 */
	uint32_t currentTime = HAL_GetTick();
	ChatRoomObj room;
	if ((currentTime - lastDebounceTime) > debounceDelay) {
		if (GPIO_Pin == StartButton_Pin) //StartButton_Pin = Pin0 = 1
		{
			lastDebounceTime = currentTime;  // 마지막 클릭 시간 업데이트
			// RunQtTask에 플래그 0x01 설정 (태스크가 osThreadFlagsWait로 기다림)
			isQtRunning = 1;
			osThreadFlagsSet(RunQtTaskHandle, 0x01);
		}
		else {
			if (isQtRunning) {
				if (GPIO_Pin == Room1Button_Pin)  //Room1Button_Pin = Pin1 = 2
				{
					room.roomNum = 1;
					room.state = (chatRoomStatus[0] == 0) ? 1 : 0; //현재 상태 반전하여 세팅
					chatRoomStatus[0] = room.state; //현재 상태저장한 전역변수 업데이트
					lastDebounceTime = currentTime;  // 마지막 클릭 시간 업데이트
				}
				else if (GPIO_Pin == Room2Button_Pin)
				{
					room.roomNum = 2;
					room.state = (chatRoomStatus[1] == 0) ? 1 : 0; //현재 상태 반전하여 세팅
					chatRoomStatus[1] = room.state; //현재 상태저장한 전역변수 업데이트
					lastDebounceTime = currentTime;  // 마지막 클릭 시간 업데이트
				}
				else if (GPIO_Pin == Room3Button_Pin)
				{
					room.roomNum = 3;
					room.state = (chatRoomStatus[2] == 0) ? 1 : 0; //현재 상태 반전하여 세팅
					chatRoomStatus[2] = room.state; //현재 상태저장한 전역변수 업데이트
					lastDebounceTime = currentTime;  // 마지막 클릭 시간 업데이트
				}
				osMessageQueuePut(ChatRoomQueueHandle, &room, 0, 0);
			}
		}
	}
}
// 부저 주파수를 설정 및 PWM출력을 시작 함.
void Set_Buzzer_Frequency(uint8_t buzzer_id) {
	uint32_t timer_clock = HAL_RCC_GetPCLK2Freq(); // TIM1은 APB2 클럭 사용, APB2의 클럭은 가져온다.
	uint32_t prescaler = 83;  // Prescaler 설정 (1 MHz 클럭을 위해)
	uint32_t arr = (timer_clock / DO * (prescaler + 1)) - 1;

	// TIM1의 Prescaler와 ARR 값을 동적으로 설정
	htim1.Init.Prescaler = prescaler;
	htim1.Init.Period = arr;  // ARR 값으로 주파수 조정
	HAL_TIM_PWM_Init(&htim1);

	uint8_t target;
	switch (buzzer_id) {
	case 1:
		target = TIM_CHANNEL_1;
		break;
	case 2:
		target = TIM_CHANNEL_2;
		break;
	case 3:
		target = TIM_CHANNEL_3;
		break;
	}

	// PWM 출력 시작 (PA8 → TIM1_CHx 사용)
	HAL_TIM_PWM_Start(&htim1, target);

	// 듀티 사이클 50%로 설정
	__HAL_TIM_SET_COMPARE(&htim1, target, arr / 2);
}

// 부저 정지 함수
void Stop_Buzzer(uint8_t buzzer_id) {
	uint8_t target;
		switch (buzzer_id) {
		case 1:
			target = TIM_CHANNEL_1;
			break;
		case 2:
			target = TIM_CHANNEL_2;
			break;
		case 3:
			target = TIM_CHANNEL_3;
			break;
		}
	HAL_TIM_PWM_Stop(&htim1, target);
}

void Play_Melody(uint8_t buzzer_id) {
	Set_Buzzer_Frequency(buzzer_id);
	HAL_Delay(1000);
	Stop_Buzzer(buzzer_id);
}

//////////////*UART용*////////////
void UWriteData(const char data) {
	while (__HAL_UART_GET_FLAG(&huart2,UART_FLAG_TXE) == RESET)
		;
	huart2.Instance->DR = data;
}

int __io_putchar(int ch) {
	UWriteData(ch);
	return ch;
}
////////////////////////////////

/* USER CODE END 4 */

/* USER CODE BEGIN Header_ChatRoomTaskFunc */
/**
 * @brief  Function implementing the ChatRoomTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_ChatRoomTaskFunc */
void ChatRoomTaskFunc(void *argument)
{
  /* USER CODE BEGIN 5 */
	/* Infinite loop */
	ChatRoomObj room;
	uint8_t targetDebCnt;
	//char uartMessage[50]; // 메시지 저장용 버퍼

	for (;;) {
		// 메시지 큐에서 방 번호 수신 (무한 대기)
		if (osMessageQueueGet(ChatRoomQueueHandle, &room, NULL, osWaitForever) == osOK) {
			//UART로 QT 프로그램에 생성된 방 번호 알림.
			//snprintf(uartMessage, sizeof(uartMessage), "RUN_QT_CHATROOM%lu_ENABLED\n", roomNum);
			//HAL_UART_Transmit(&huart2, (uint8_t*)uartMessage, strlen(uartMessage), HAL_MAX_DELAY);
			if (room.roomNum == 1) {
				targetDebCnt = ++debouncing_cnt_sw2;
				// LED on/off
				HAL_GPIO_WritePin(Chat1_R_LED_GPIO_Port, Chat1_R_LED_Pin, room.state);
				//QT로 on/off 메시지 전송
			}
			else if (room.roomNum == 2) {
				targetDebCnt = ++debouncing_cnt_sw3;
				HAL_GPIO_WritePin(Chat2_Y_LED_GPIO_Port, Chat2_Y_LED_Pin, room.state);
			}
			else if (room.roomNum == 3) {
				targetDebCnt = ++debouncing_cnt_sw4;
				HAL_GPIO_WritePin(Chat3_G_LED_GPIO_Port, Chat3_G_LED_Pin, room.state);
			}
			printf("Chatroom%u Status is %u , Counting for pushed%u: %u \r\n",room.roomNum, room.state, room.roomNum, targetDebCnt);

			//버저 출력 : BuzzerTaskHandle에 MessageQueue로 전달.
			BuzzerID_t buzzerCmd = room.roomNum;
			osMessageQueuePut(BuzzerQueueHandle, &buzzerCmd, 0, 0);
		}
	}
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_RunQtTaskFunc */
/**
 * @brief Function implementing the RunQtTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_RunQtTaskFunc */
void RunQtTaskFunc(void *argument)
{
  /* USER CODE BEGIN RunQtTaskFunc */
	/* Infinite loop */
	for (;;) {
		// 0x01 플래그가 설정될 때까지 무한 대기
		osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);

		// Start 버튼이 눌렸을 때 -> UART를 통해 "RUN_QT" 명령 전송, 버저 초기화 , 버저 출력을 위한 MessageQueue에 Put
		//        HAL_UART_Transmit(&huart2, (uint8_t*)"RUN_QT\n", 7, HAL_MAX_DELAY);
		printf("UART to QT Success! Debouncing Count:%u\r\n",
				++debouncing_cnt_sw1);
		BuzzerID_t bz = BUZZER_ALL;
		//        osStatus_t res = osMessageQueuePut(BuzzerQueueHandle, &bz, 0, 0);
		//        if(res != osOK) printf("Buzzer Fail : All\r\n");
	}
  /* USER CODE END RunQtTaskFunc */
}

/* USER CODE BEGIN Header_BuzzerTaskFunc */
/**
 * @brief Function implementing the BuzzerTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_BuzzerTaskFunc */
void BuzzerTaskFunc(void *argument)
{
  /* USER CODE BEGIN BuzzerTaskFunc */
	BuzzerID_t command;
	for (;;) {
		// 메시지가 올 때까지 무한 대기
		if (osMessageQueueGet(BuzzerQueueHandle, &command, NULL, osWaitForever) == osOK) {
			switch (command) {
			case BUZZER_ALL:
				// 1,2,3번 부저 모두 울리기
				Play_Melody(1);
				Play_Melody(2);
				Play_Melody(3);
				break;
			case BUZZER_1:
				Play_Melody(1);
				break;
			case BUZZER_2:
				Play_Melody(2);
				break;
			case BUZZER_3:
				Play_Melody(3);
				break;
			default:
				break;
			}
		}
	}
  /* USER CODE END BuzzerTaskFunc */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
