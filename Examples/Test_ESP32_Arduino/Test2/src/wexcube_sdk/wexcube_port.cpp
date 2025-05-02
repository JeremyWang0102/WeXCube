/**
 * @file wexcube_port.c
 * @author JeremyWang (jeremywang0102@gmail.com / gin0101@126.com)
 * @brief WeXCube 接口文件
 * @version 
 * @date 2025-01-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "wexcube_port.h"

BLECharacteristic *_pNotifyChara = nullptr;

/**
 * @brief BLE 参数配置
 * 
 * @param pNotifyChara 通知特征值指针
 */
void wex_ble_config(BLECharacteristic *pNotifyChara)
{
  _pNotifyChara = pNotifyChara;
}


/**
 * @brief 端口初始化函数
 * 
 */
void wex_port_init(void)
{
  // 在 Test2.ino 的 ble_init 中实现
}


/**
 * @brief 发送数据
 * 
 * @param pucData 数据指针
 * @param ucLen 数据长度
 */
void wex_port_send(wex_u8_t *pucData, wex_u8_t ucLen)
{
  if (_pNotifyChara)
  {
    // 设置通知特征的值
    _pNotifyChara->setValue(pucData, ucLen);
    // 发送通知
    _pNotifyChara->notify();
  }
}


/**
  * @brief  This function handles UART interrupt request.
  *
  */
void UART_IRQHandler(void)
{
  // 在 Test2.ino 的 class MyCharacteristicCallbacks 中实现
}
