################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Utilities/subdir.mk
-include USB_Device/Target/subdir.mk
-include USB_Device/App/subdir.mk
-include STM32_WPAN/Target/subdir.mk
-include STM32_WPAN/App/subdir.mk
-include Middlewares/USB_Device_Library/subdir.mk
-include Middlewares/STM32_WPAN/subdir.mk
-include Middlewares/FreeRTOS/subdir.mk
-include Drivers/STM32WBxx_HAL_Driver/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
stm32_mainMCU_Code_v3.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
stm32_mainMCU_Code_v3.list \

OBJCOPY_BIN += \
stm32_mainMCU_Code_v3.bin \


# All Target
all: stm32_mainMCU_Code_v3.elf secondary-outputs

# Tool invocations
stm32_mainMCU_Code_v3.elf: $(OBJS) $(USER_OBJS) C:\dev\Resenv-Shenzhen-2019-Code\stm32_mainMCU_Code_v3\STM32WB55CGUX_FLASH.ld
	arm-none-eabi-gcc -o "stm32_mainMCU_Code_v3.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\dev\Resenv-Shenzhen-2019-Code\stm32_mainMCU_Code_v3\STM32WB55CGUX_FLASH.ld" --specs=nosys.specs -Wl,-Map="stm32_mainMCU_Code_v3.map" -Wl,--gc-sections -static -L"C:\Users\patri\STM32Cube\Repository\Packs\ARM\CMSIS\5.6.0\CMSIS\DSP\Lib\GCC" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

stm32_mainMCU_Code_v3.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "stm32_mainMCU_Code_v3.list"
	@echo 'Finished building: $@'
	@echo ' '

stm32_mainMCU_Code_v3.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "stm32_mainMCU_Code_v3.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
