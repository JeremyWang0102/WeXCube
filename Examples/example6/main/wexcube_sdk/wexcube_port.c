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

#include "driver/uart.h"


/**
 * @brief 串口初始化函数
 * 
 */
void wex_port_init(void)
{
}


/**
 * @brief 发送数据
 * 
 * @param pucData 数据指针
 * @param ucLen 数据长度
 */
void wex_port_send(wex_u8_t *pucData, wex_u8_t ucLen)
{
	uart_write_bytes(UART_NUM_1, pucData, ucLen);
}
