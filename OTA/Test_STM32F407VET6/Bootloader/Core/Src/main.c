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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "wexcube.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef void (*pFunction)(void); // 定义函数指针类型
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FLASH_APP_START_ADDR   	0x08008000
#define FLASH_EACH_WRITE_SIZE   1024        // 每次写 1KB
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
uint8_t upgradeRecBuff[FLASH_EACH_WRITE_SIZE + 64] __attribute__((aligned(4)));	// 4 字节对齐存储
uint32_t upgradeRecBuffIndex = 0;
uint32_t upgradeRecBuffSum = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
void JumpToApp(void) {
	pFunction jump_to_app;
	uint32_t stack_pointer;

	// 1. 关闭全局中断
	__disable_irq();

	// 2. 关闭 SysTick 定时器（避免中断干扰）
	SysTick->CTRL = 0;

	// 3. 设置 App 的栈顶指针（SP）
	stack_pointer = *(volatile uint32_t*)FLASH_APP_START_ADDR;
	__set_MSP(stack_pointer); // 设置主堆栈指针

	// 4. 设置 App 的复位向量地址（Reset Handler）
	jump_to_app = (pFunction)(*(volatile uint32_t*)(FLASH_APP_START_ADDR + 4));

	// 5. 设置中断向量表偏移寄存器（VTOR）
	SCB->VTOR = FLASH_APP_START_ADDR; // 对于 Cortex-M4，VTOR 有效

	// 6. 跳转到 App
	jump_to_app();

	// 7. 理论上不会执行到这里
	while (1);
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	uint16_t reqPackNo = 0;		// 请求的升级数据包编号
	const t_sWexCmd *psWexCmd;
	uint32_t times = 25;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART6_UART_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
	wex_init();
	wex_start();
	
	// 开启接收中断
	__HAL_UART_ENABLE_IT(&huart6, UART_IT_RXNE);
	
	// 留一定时间监测升级事件。
	// 如果担心APP升级错误导致程序无法运行，可以增加此处时间，建议4s以上。
	// 当APP无法正常运行的升级步骤：
	// 1. 给设备断电，
	// 2. 在WeXCube小程序升级界面选择好升级文件，
	// 3. 再选择需要升级的设备，
	// 4. 然后给设备上电，
	// 5. 当连接成功立马点击传输文件，
	// 此处监测升级事件时间越长操作越顺利。
	while (times)
	{
		psWexCmd = wex_process();
		if (psWexCmd->eCmdType == eWexCmd_Upgrade) break;
		HAL_Delay(200);
		times--;
	}
	
	// 进入 APP 程序段执行 
	if (times == 0)
	{
		wex_sendDisconnect();
		wex_process();
		HAL_Delay(200);
		JumpToApp();
	}
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		psWexCmd = wex_process();
    switch (psWexCmd->eCmdType)
    {
			case eWexCmd_Upgrade:	// 升级数据
			{
				// 升级数据包编号错误，请求重新传输
				if ((psWexCmd->sUpgrade.usPackNo != reqPackNo) && (psWexCmd->sUpgrade.usPackNo != 0xFFFF))
				{
					wex_askUpradePack(reqPackNo);
					break;
				}
				
				// 第一包数据执行擦除Flash
				if (psWexCmd->sUpgrade.usPackNo == 0)
				{
					// 禁用中断
					__disable_irq();
					// 解锁 Flash
					HAL_FLASH_Unlock();

					// 擦除 APP Sector
					FLASH_EraseInitTypeDef EraseInit;
					uint32_t SectorError = 0;
					EraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
					EraseInit.Sector = FLASH_SECTOR_2;
					EraseInit.NbSectors = 6;
					EraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

					if (HAL_FLASHEx_Erase(&EraseInit, &SectorError) != HAL_OK) {
							Error_Handler();
					}
					// 恢复中断
					__enable_irq();
				}
				
				memcpy(upgradeRecBuff + upgradeRecBuffIndex, psWexCmd->sUpgrade.pucPackData, psWexCmd->sUpgrade.ucPackSize);
				upgradeRecBuffIndex += psWexCmd->sUpgrade.ucPackSize;
				// 保存在后段 Flash 中
				if (upgradeRecBuffIndex >= FLASH_EACH_WRITE_SIZE)
				{
					// 写数据
					uint32_t *data = (uint32_t*)upgradeRecBuff;
					uint32_t beginAdrr = FLASH_APP_START_ADDR + upgradeRecBuffSum;
					// 禁用中断
					__disable_irq();
					for (uint32_t i = 0; i < FLASH_EACH_WRITE_SIZE; i += 4) {
						if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, beginAdrr + i, data[i / 4]) != HAL_OK) {
								Error_Handler();
						}
					}
					// 恢复中断
					__enable_irq();
					
					upgradeRecBuffSum += FLASH_EACH_WRITE_SIZE;
					
					// 保留剩余的数据
					if (upgradeRecBuffIndex > FLASH_EACH_WRITE_SIZE)
					{
						memcpy(upgradeRecBuff, upgradeRecBuff + FLASH_EACH_WRITE_SIZE, upgradeRecBuffIndex - FLASH_EACH_WRITE_SIZE);
						upgradeRecBuffIndex -= FLASH_EACH_WRITE_SIZE;
					}
					else
					{
						upgradeRecBuffIndex = 0; 
					}
				}
				
				if (psWexCmd->sUpgrade.usPackNo != 0xFFFF)
				{
					reqPackNo = psWexCmd->sUpgrade.usPackNo + 1;
					wex_askUpradePack(reqPackNo);
					
					// 闪灯提示正在传输
					HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_2);
				}
				else
				{
					HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
					
					// 禁用中断
					__disable_irq();
					// 把剩余数据写入 Flash
					if (upgradeRecBuffIndex)
					{
						uint32_t *data = (uint32_t*)upgradeRecBuff;
						uint32_t beginAdrr = FLASH_APP_START_ADDR + upgradeRecBuffSum;
						for (uint32_t i = 0; i < upgradeRecBuffIndex; i += 4) {
							if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, beginAdrr + i, data[i / 4]) != HAL_OK) {
									Error_Handler();
							}
						}
					}
					// 重新加锁 Flash
					HAL_FLASH_Lock();
					// 恢复中断
					__enable_irq();
					
					// 进入 APP 程序段执行
					JumpToApp();
					
					// 异常时
					upgradeRecBuffSum = 0;
					upgradeRecBuffIndex = 0;
					reqPackNo = 0;
				}
			}
			break;
    
			default:
      break;
    }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* USART6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART6_IRQn);
}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin : PE2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
