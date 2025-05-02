/**
 * @file wexcube_port.h
 * @author JeremyWang (jeremywang0102@gmail.com / gin0101@126.com)
 * @brief WeXCube 接口头文件
 * @version 
 * @date 2025-01-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef __WEXCUBE_PORT_H__
#define __WEXCUBE_PORT_H__

#include <BLEDevice.h>
#include "wexcube.h"

/************************ 打印接口 ************************/
#define wex_printf(...)  // 不使用打印功能时可以使用此行
/*********************************************************/


void wex_ble_config(BLECharacteristic *pNotifyChara);

void wex_port_init(void);                               // 初始化端口
void wex_port_send(wex_u8_t *pucData, wex_u8_t ucLen);  // 发送数据


#endif /* __WEXCUBE_PORT_H__ */
