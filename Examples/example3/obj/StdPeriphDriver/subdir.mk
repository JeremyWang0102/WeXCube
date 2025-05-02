################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../StdPeriphDriver/CH57x_adc.c \
../StdPeriphDriver/CH57x_clk.c \
../StdPeriphDriver/CH57x_flash.c \
../StdPeriphDriver/CH57x_gpio.c \
../StdPeriphDriver/CH57x_pwr.c \
../StdPeriphDriver/CH57x_sys.c \
../StdPeriphDriver/CH57x_timer0.c \
../StdPeriphDriver/CH57x_uart0.c \
../StdPeriphDriver/CH57x_uart1.c \
../StdPeriphDriver/CH57x_uart3.c \
../StdPeriphDriver/CH57x_usbdev.c 

OBJS += \
./StdPeriphDriver/CH57x_adc.o \
./StdPeriphDriver/CH57x_clk.o \
./StdPeriphDriver/CH57x_flash.o \
./StdPeriphDriver/CH57x_gpio.o \
./StdPeriphDriver/CH57x_pwr.o \
./StdPeriphDriver/CH57x_sys.o \
./StdPeriphDriver/CH57x_timer0.o \
./StdPeriphDriver/CH57x_uart0.o \
./StdPeriphDriver/CH57x_uart1.o \
./StdPeriphDriver/CH57x_uart3.o \
./StdPeriphDriver/CH57x_usbdev.o 

C_DEPS += \
./StdPeriphDriver/CH57x_adc.d \
./StdPeriphDriver/CH57x_clk.d \
./StdPeriphDriver/CH57x_flash.d \
./StdPeriphDriver/CH57x_gpio.d \
./StdPeriphDriver/CH57x_pwr.d \
./StdPeriphDriver/CH57x_sys.d \
./StdPeriphDriver/CH57x_timer0.d \
./StdPeriphDriver/CH57x_uart0.d \
./StdPeriphDriver/CH57x_uart1.d \
./StdPeriphDriver/CH57x_uart3.d \
./StdPeriphDriver/CH57x_usbdev.d 


# Each subdirectory must supply rules for building sources it contributes
StdPeriphDriver/%.o: ../StdPeriphDriver/%.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused  -g -DDEBUG=1 -DCH573 -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\APP\app_drv_fifo" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\APP\ble_uart_service" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\APP\include" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\HAL\include" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\Ld" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\LIB" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\Profile\include" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\RVMSIS" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\Startup" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\StdPeriphDriver\inc" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\wexcube_sdk" -I"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\APP\usb_hid" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

