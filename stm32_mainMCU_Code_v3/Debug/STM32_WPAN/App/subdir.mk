################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../STM32_WPAN/App/app_thread.c 

OBJS += \
./STM32_WPAN/App/app_thread.o 

C_DEPS += \
./STM32_WPAN/App/app_thread.d 


# Each subdirectory must supply rules for building sources it contributes
STM32_WPAN/App/app_thread.o: ../STM32_WPAN/App/app_thread.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DARM_MATH_CM4 '-DOPENTHREAD_CONFIG_FILE=<openthread_api_config_ftd.h>' -DTHREAD_WB -DDEBUG -DSTM32WB55xx -c -I../tiny_lpm -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/ST/STM32_WPAN/thread/openthread/stack/include -I"C:/Users/patri/STM32Cube/Repository/Packs/ARM/CMSIS/5.6.0/CMSIS/DSP/Include" -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Utilities/lpm/tiny_lpm -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/ST/STM32_WPAN/thread/openthread/stack/include/openthread -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/ST/STM32_WPAN/thread/openthread/stack/include/openthread/platform -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../STM32_WPAN/App -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Drivers/STM32WBxx_HAL_Driver/Inc -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/Third_Party/FreeRTOS/Source/include -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/ST/STM32_WPAN -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Drivers/CMSIS/Include -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../Core/Inc -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/ST/STM32_WPAN/thread/openthread/core/openthread_api -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/ST/STM32_WPAN/utilities -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Drivers/STM32WBxx_HAL_Driver/Inc/Legacy -I"C:/Users/patri/STM32Cube/Repository/Packs/ARM/CMSIS/5.6.0/CMSIS/DSP/Lib/ARM" -I"C:/dev/attentionDevBoard/stm32_mainMCU_Code_v3/Utilities" -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"STM32_WPAN/App/app_thread.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

