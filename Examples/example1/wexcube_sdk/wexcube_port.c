/**
 * @file wexcube_port.c
 * @author JeremyWang (jeremywang0102@gmail.com / gin0101@126.com)
 * @brief WeXCube 接口文件
 * @version 
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "wexcube_port.h"
#include "wexcube.h"

#include "stm32f0xx_hal.h"

extern UART_HandleTypeDef huart1;

/**
 * @brief 串口初始化函数
 * 
 */
void wex_port_init(void)
{
	// 已经在 main.c 的 MX_USART1_UART_Init 中初始化了
}


/**
 * @brief 发送数据
 * 
 * @param pucData 数据指针
 * @param ucLen 数据长度
 */
void wex_port_send(wex_u8_t *pucData, wex_u8_t ucLen)
{
	wex_u8_t n = ucLen;

	while(n--)
	{
		huart1.Instance->TDR = *pucData++;
		while(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) == RESET);	// 等待发送缓冲区为空
		while((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET));	// 等待串口发送完毕
	}
}


/**
  * @brief  This function handles UART interrupt request.
  * @param  None
  * @retval None
  */
void UART_IRQHandler(void)
{
	// 已经在 stm32f0xx_it.c 的 USART1_IRQHandler 中实现了
}
