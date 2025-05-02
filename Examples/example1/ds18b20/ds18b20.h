#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "stm32f0xx_hal.h"


#ifdef __cplusplus
extern "C" {
#endif

void DS18B20_GPIO_Output(void);
float DS18B20_GetTemperature(void);

#ifdef __cplusplus
}
#endif

#endif /* __DS18B20_H__ */
