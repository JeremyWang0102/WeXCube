#include "stm32f0xx_hal.h"

// 宏定义
#define DS18B20_PIN GPIO_PIN_1
#define DS18B20_PORT GPIOA

#define DS18B20_CMD_SKIPROM 0xCC        // 跳过 ROM 命令
#define DS18B20_CMD_CONVERTTEMP 0x44    // 温度转换命令
#define DS18B20_CMD_RSCRATCHPAD 0xBE    // 读取寄存器

// 延时函数
void delay_us(uint16_t us) {
    uint32_t count = us;
    while (count--) {
			uint32_t n = 5;
			while (n--) {
        __NOP();
			}
    }
}

// GPIO 配置为输出
void DS18B20_GPIO_Output(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS18B20_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;  // 开漏输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
}

// GPIO 配置为输入
void DS18B20_GPIO_Input(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS18B20_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  // 输入模式
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
}

// 单总线复位
uint8_t DS18B20_Reset(void) {
    DS18B20_GPIO_Output();                  // 配置为输出模式
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);  // 拉低
    delay_us(1100);                          // 保持拉低
    DS18B20_GPIO_Input();                   // 配置为输入模式
    delay_us(40);                           // 等待
    uint8_t presence = HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN);  // 检测存在脉冲
    delay_us(460);                          // 剩余时间
    return (presence == GPIO_PIN_RESET) ? 1 : 0;  // 返回是否检测到设备
}

// 写入一个字节
void DS18B20_WriteByte(uint8_t data) {
    DS18B20_GPIO_Output();
    for (int i = 0; i < 8; i++) {
        HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);  // 拉低
        delay_us(2);
        if (data & 0x01) {  // 根据当前位写入 1 或 0
            HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
        }
        delay_us(60);  // 等待
        HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);  // 释放
        delay_us(2);
        data >>= 1;  // 移位到下一位
    }
}

// 读取一个字节
uint8_t DS18B20_ReadByte(void) {
    uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        DS18B20_GPIO_Output();
        HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);  // 拉低
        delay_us(2);
        DS18B20_GPIO_Input();
        delay_us(12);  // 等待数据稳定
        if (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN)) {
            data |= (1 << i);  // 读取位
        }
        delay_us(50);  // 等待完成
    }
    return data;
}

// 读取温度
float DS18B20_GetTemperature(void) {
    if (!DS18B20_Reset()) {
        return -99.9f;  // 如果未检测到 DS18B20，返回错误
    }
    DS18B20_WriteByte(DS18B20_CMD_SKIPROM);      // 跳过 ROM 匹配
    DS18B20_WriteByte(DS18B20_CMD_CONVERTTEMP);  // 触发温度转换
    DS18B20_Reset();
    DS18B20_WriteByte(DS18B20_CMD_SKIPROM);      // 跳过 ROM 匹配
    DS18B20_WriteByte(DS18B20_CMD_RSCRATCHPAD);  // 读取数据命令

    uint8_t tempL = DS18B20_ReadByte();  // 读取低字节
    uint8_t tempH = DS18B20_ReadByte();  // 读取高字节

    int16_t rawTemp = (tempH << 8) | tempL;  // 组合为 16 位温度数据
    return rawTemp * 0.0625f;  // 转换为摄氏度
}

