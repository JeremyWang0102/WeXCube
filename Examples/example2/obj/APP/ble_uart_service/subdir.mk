################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP/ble_uart_service/ble_uart_service_16bit.c 

OBJS += \
./APP/ble_uart_service/ble_uart_service_16bit.o 

C_DEPS += \
./APP/ble_uart_service/ble_uart_service_16bit.d 


# Each subdirectory must supply rules for building sources it contributes
APP/ble_uart_service/%.o: ../APP/ble_uart_service/%.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused  -g -DDEBUG=1 -DCH573 -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example2\APP\app_drv_fifo" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example2\APP\ble_uart_service" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example2\APP\include" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example2\HAL\include" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example2\Ld" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example2\LIB" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example2\Profile\include" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example2\RVMSIS" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example2\Startup" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example2\StdPeriphDriver\inc" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example2\wexcube_sdk" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example2\APP\usb_hid" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

