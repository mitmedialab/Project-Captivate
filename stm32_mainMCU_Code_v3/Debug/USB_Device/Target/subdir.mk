################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_Device/Target/usbd_conf.c 

OBJS += \
./USB_Device/Target/usbd_conf.o 

C_DEPS += \
./USB_Device/Target/usbd_conf.d 


# Each subdirectory must supply rules for building sources it contributes
USB_Device/Target/usbd_conf.o: ../USB_Device/Target/usbd_conf.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g -DUSE_HAL_DRIVER -DHAL_TSC_MODULE_ENABLED -DARM_MATH_CM4 '-DOPENTHREAD_CONFIG_FILE=<openthread_api_config_ftd.h>' -DTHREAD_WB -DDEBUG -DSTM32WB55xx -c -I"C:/Users/patri/STM32Cube/Repository/Packs/ARM/CMSIS/5.6.0/CMSIS/DSP/Lib/GCC" -I"C:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Drivers/STM32WBxx_HAL_Driver/Src" -I"C:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/ST/STM32_TouchSensing_Library/inc" -I../Core/Inc -I../USB_Device/Target -I"C:/Users/patri/STM32Cube/Repository/Packs/ARM/CMSIS/5.6.0/CMSIS/DSP/Lib/ARM" -I../USB_Device/App -I../STM32_WPAN/App -I"C:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.4.0/Middlewares/ST/STM32_TouchSensing_Library/src" -I"C:/dev/attentionDevBoard/stm32_mainMCU_Code_v3/Utilities" -I"C:/Users/patri/STM32Cube/Repository/Packs/ARM/CMSIS/5.6.0/CMSIS/DSP/Include" -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Drivers/STM32WBxx_HAL_Driver/Inc -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Drivers/STM32WBxx_HAL_Driver/Inc/Legacy -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Utilities/lpm/tiny_lpm -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/Third_Party/FreeRTOS/Source/include -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/ST/STM32_WPAN -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/ST/STM32_WPAN/utilities -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/ST/STM32_USB_Device_Library/Core/Inc -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Drivers/CMSIS/Device/ST/STM32WBxx/Include -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/ST/STM32_WPAN/thread/openthread/core/openthread_api -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/ST/STM32_WPAN/thread/openthread/stack/include -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/ST/STM32_WPAN/thread/openthread/stack/include/openthread -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Middlewares/ST/STM32_WPAN/thread/openthread/stack/include/openthread/platform -IC:/Users/patri/STM32Cube/Repository/STM32Cube_FW_WB_V1.10.0/Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"USB_Device/Target/usbd_conf.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

