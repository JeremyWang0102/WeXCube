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
#include "ds18b20.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// 设备控制页面控件ID
#define TEXT_ID       2   // 文本框 ID
#define SWITCH_ID     5   // 开关ID
#define SLIDER_ID     7   // 滑动条ID
#define COLOR1_ID			9		// 颜色1
#define COLOR2_ID			10	// 颜色2
#define COLOR3_ID			11	// 颜色3
#define COLOR4_ID			12	// 颜色4
#define COLOR5_ID			13	// 颜色5
#define COLOR6_ID			14	// 颜色6
#define COLOR7_ID			15	// 颜色7
#define COLOR8_ID			16	// 颜色8
#define COLOR9_ID			17	// 颜色9
#define COLOR10_ID		18	// 颜色10
#define MODEL1_ID			20	// 模式1
#define MODEL2_ID			22	// 模式2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
static void updateLED(wex_u8_t ucRed, wex_u8_t ucGreen, wex_u8_t ucBlue, wex_u8_t ucLevel);
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
	wex_u8_t lightOn		= 0;
  wex_u8_t lightLevel = 0;
	wex_u8_t model 			= 0;			// 0 - 正常，1 - 呼吸
	wex_u8_t ledRGB[3] 	= {255, 255, 255};
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
  MX_TIM3_Init();
  MX_USART1_UART_Init();
	
	/* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
	
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
	
	wex_init();
	wex_start();
	
	// 开启接收中断
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		const t_sWexCmd *pWexCmd = wex_process();
    switch (pWexCmd->eCmdType)
    {
			case eWexCmd_Connect:       		// WeXCube 连接指令
			{
				wex_setValue(SWITCH_ID, lightOn);
				wex_setValue(SLIDER_ID, lightLevel);
				if (model == 0)
				{
					wex_setBackColor(MODEL1_ID, eWexColor_Green);
					wex_setBackColor(MODEL2_ID, eWexColor_Gray2);
				}
				else
				{
					wex_setBackColor(MODEL1_ID, eWexColor_Gray2);
					wex_setBackColor(MODEL2_ID, eWexColor_Green);
				}
			}
      break;

			case eWexCmd_Event:         // 控件事件触发
      {
        switch (pWexCmd->ucCtrlId)
        {
					case SWITCH_ID:         // 开关控件
					{
						if (pWexCmd->ucValue)
						{
							lightOn     = 1;
							lightLevel  = 25;
						}
						else
						{
							lightOn     = 0;
							lightLevel  = 0;
						}
						wex_setValue(SLIDER_ID, lightLevel);
						updateLED(ledRGB[0], ledRGB[1], ledRGB[2], lightLevel);
					}
          break;
        
					case SLIDER_ID:          // 滑动条控件
					{
						lightLevel = pWexCmd->ucValue;
						if (lightLevel)
						{
							if (!lightOn)
							{
								lightOn = 1;
								wex_setValue(SWITCH_ID, 1);
							}
						}
						else
						{
							if (lightOn)
							{
								lightOn = 0;
								wex_setValue(SWITCH_ID, 0);
							}
						}
						updateLED(ledRGB[0], ledRGB[1], ledRGB[2], lightLevel);
					}
					break;
					
					case COLOR1_ID:					// 颜色控件
					case COLOR2_ID:					// 颜色控件
					case COLOR3_ID:					// 颜色控件
					case COLOR4_ID:					// 颜色控件
					case COLOR5_ID:					// 颜色控件
					case COLOR6_ID:					// 颜色控件
					case COLOR7_ID:					// 颜色控件
					case COLOR8_ID:					// 颜色控件
					case COLOR9_ID:					// 颜色控件
					case COLOR10_ID:				// 颜色控件
					{
						wex_askBackRGB(pWexCmd->ucCtrlId);	// 询问控件的背景颜色
					}
					break;
					
					case MODEL1_ID:					// 灯光模式控件1
					{
						model = 0;
						wex_setBackColor(MODEL1_ID, eWexColor_Green);
						wex_setBackColor(MODEL2_ID, eWexColor_Gray2);
						if (lightOn == 1) updateLED(ledRGB[0], ledRGB[1], ledRGB[2], lightLevel);
					}
					break;
					
					case MODEL2_ID:					// 灯光模式控件2
					{
						model = 1;
						wex_setBackColor(MODEL1_ID, eWexColor_Gray2);
						wex_setBackColor(MODEL2_ID, eWexColor_Green);
					}
					break;
					
					default:
					break;
				}
      }
      break;
			
			case eWexCmd_BackRGB:				// 控件背景 RGB 颜色
			{
				// 根据控件背景颜色设置 LED 颜色
				ledRGB[0] = pWexCmd->sColor.ucR;
				ledRGB[1] = pWexCmd->sColor.ucG;
				ledRGB[2] = pWexCmd->sColor.ucB;
				updateLED(ledRGB[0], ledRGB[1], ledRGB[2], lightLevel);
			}
			break;
    
			default:
      break;
    }

    // 板子上按键处理
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
    {
      HAL_Delay(20);
      if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
      {
        if (lightOn)
        {
          lightOn     = 0;
          if (wex_getConnectState())
          {
            wex_setValue(SWITCH_ID, 0);
            wex_setValue(SLIDER_ID, 0);
          }
					updateLED(ledRGB[0], ledRGB[1], ledRGB[2], 0);
        }
        else
        {
          lightOn     = 1;
          lightLevel  = lightLevel == 0 ? 25 : lightLevel;
          if (wex_getConnectState())
          {
            wex_setValue(SWITCH_ID, 1);
            wex_setValue(SLIDER_ID, lightLevel);
          }
					updateLED(ledRGB[0], ledRGB[1], ledRGB[2], lightLevel);
        }
				
        // 等待按键释放
			  while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET);
			}
		}
		
		HAL_Delay(10);
		
		// 如果灯光模式为1-呼吸，则持续改变灯光亮度
		if ((model == 1) && (lightOn == 1))
		{
			static wex_u32_t cnt = 0;
			static wex_u8_t level = 0;
			static wex_u8_t dir = 0;
			cnt++;
			// 3 * 10 = 30ms 更新一次温度值
			if (cnt >= 3)
			{
				cnt = 0;
				if (dir)
				{
					if (level >= lightLevel) dir = 0;
					else level++;
				}
				else
				{
					if (level == 0) dir = 1;
					else level--;
				}
				updateLED(ledRGB[0], ledRGB[1], ledRGB[2], level);
			}
		}
		
		// 读取温度值
		if (1)
		{
			static wex_u32_t cnt = 0;
			cnt++;
			// 50 * 10 = 500ms 更新一次温度值
			if (cnt >= 50)
			{
				wex_f32_t tempC = DS18B20_GetTemperature();
				cnt = 0;
				// 上传温度值
				if (wex_getConnectState()) wex_setText(TEXT_ID, wex_floatToStr(tempC, 1));
			}
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 20;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 256 * 50;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PA3 PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
  * @brief LED 颜色、亮度更新
  * @param ucRed 红色分量
  * @param ucGreen 绿色分量
  * @param ucBlue 蓝色分量
  * @param ucLevel 亮度
  * @retval None
  */
static void updateLED(wex_u8_t ucRed, wex_u8_t ucGreen, wex_u8_t ucBlue, wex_u8_t ucLevel)
{
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ucLevel * ucRed);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, ucLevel * ucGreen);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, ucLevel * ucBlue);
}

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
