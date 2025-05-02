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
#include "wexcube.h"


/**
 * @brief 端口初始化函数
 * 
 */
void wex_port_init(void)
{
  extern wex_u32_t baudrate;
  // 计算波特率寄存器值
  uint16_t baud_setting = (F_CPU / 16 / baudrate) - 1;
  
  // 设置波特率寄存器
  UBRR0H = (baud_setting >> 8);   // 高位
  UBRR0L = baud_setting;          // 低位
  
  // 启用接收和发送功能
  UCSR0B |= (1 << RXEN0) | (1 << TXEN0); // RXEN0 + TXEN0

  // 启用接收中断
  UCSR0B |= (1 << RXCIE0); 

  // 8位数据，无校验，1停止位
  UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
}


/**
 * @brief 发送数据
 * 
 * @param pucData 数据指针
 * @param ucLen 数据长度
 */
void wex_port_send(wex_u8_t *pucData, wex_u8_t ucLen)
{
  // 直接操作寄存器发送数据（无需中断）
  for (wex_u8_t i = 0; i < ucLen; i++)
  {
    while (!(UCSR0A & (1 << UDRE0)));   // 等待发送缓冲区就绪
    UDR0 = pucData[i];                  // 发送字符
  }
}


/**
  * @brief  This function handles UART interrupt request.
  *
  */
ISR(USART_RX_vect)
{
  char c = UDR0; // 直接读取接收到的字节
  wex_push(&c, 1);
}
