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

#include "stc8h.h"

bit txBusy = 0;

/**
 * @brief 串口初始化函数
 * 
 */
void wex_port_init(void)
{
	//115200bps@33.1776MHz
	S2CON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = 0xB8;			//设置定时初始值
	T2H = 0xFF;			//设置定时初始值
	AUXR |= 0x10;		//定时器2开始计时
	IE2 |= 0x01;		//使能串口2中断
	
	P_SW2 &= ~0x01;		//UART2/USART2: RxD2(P1.0), TxD2(P1.1)
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
		while(txBusy);
		S2BUF = *pucData++;
		txBusy = 1;
	}
}


/**
  * @brief  This function handles UART interrupt request.
  * @param  None
  * @retval None
  */
void UART2_Isr() interrupt 8
{
	// 发送中断
    if (S2CON & S2TI)
    {
        S2CON &= ~S2TI;                         //清中断标志
		txBusy = 0;
    }
	
	// 接收中断
    if (S2CON & S2RI)
    {
		wex_u8_t recData = S2BUF;
        S2CON &= ~S2RI;                         //清中断标志
		wex_push(&recData, 1);
    }	
}
