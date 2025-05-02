################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include wexcube_sdk/subdir.mk
-include StdPeriphDriver/subdir.mk
-include Startup/subdir.mk
-include RVMSIS/subdir.mk
-include Profile/subdir.mk
-include HAL/subdir.mk
-include APP/usb_hid/subdir.mk
-include APP/ble_uart_service/subdir.mk
-include APP/app_drv_fifo/subdir.mk
-include APP/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
example3.hex \

SECONDARY_LIST += \
example3.lst \

SECONDARY_SIZE += \
example3.siz \


# All Target
all: example3.elf secondary-outputs

# Tool invocations
example3.elf: $(OBJS) $(USER_OBJS)
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused  -g -T "E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -L"../" -L"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\LIB" -L"E:\WORK\JeremyWang_Gitee\wexcube\Examples\example3\StdPeriphDriver" -Xlinker --print-memory-usage -Wl,-Map,"example3.map" --specs=nano.specs --specs=nosys.specs -o "example3.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@	@
example3.hex: example3.elf
	@	riscv-none-embed-objcopy -O ihex "example3.elf"  "example3.hex"
	@	@
example3.lst: example3.elf
	@	riscv-none-embed-objdump --source --all-headers --demangle --line-numbers --wide "example3.elf" > "example3.lst"
	@	@
example3.siz: example3.elf
	@	riscv-none-embed-size --format=berkeley "example3.elf"
	@	@
# Other Targets
clean:
	-$(RM) $(ASM_UPPER_DEPS)$(OBJS)$(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_DEPS)$(S_UPPER_DEPS)$(C_DEPS) example3.elf
	-@
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
