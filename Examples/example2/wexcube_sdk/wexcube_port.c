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


bool wexSendFlag = false;

/**
 * @brief 串口初始化函数
 * 
 */
void wex_port_init(void)
{
    // 单片机内置 ble，初始化已在 CH57X_BLEInit 实现
}


/**
 * @brief 发送数据
 * 
 * @param pucData 数据指针
 * @param ucLen 数据长度
 */
void wex_port_send(wex_u8_t *pucData, wex_u8_t ucLen)
{
    extern app_drv_fifo_t app_rx_fifo;
    app_drv_fifo_write_from_same_addr(&app_rx_fifo, pucData, ucLen);
    wexSendFlag = true;
}


/**
  * @brief  This function handles UART interrupt request.
  * @param  None
  * @retval None
  */
void UART_IRQHandler(void)
{
    // 在  peripheral.c 中的 on_bleuartServiceEvt 实现
}
