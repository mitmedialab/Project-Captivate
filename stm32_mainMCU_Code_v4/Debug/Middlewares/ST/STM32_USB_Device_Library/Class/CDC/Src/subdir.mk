################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c 

C_DEPS += \
./Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.d 

OBJS += \
./Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.o: ../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DST_OT_DEBUG -DUSE_HAL_DRIVER -DDYNAMIC_MODE -DARM_MATH_CM4 '-DOPENTHREAD_CONFIG_FILE=<openthread_api_config_ftd.h>' -DTHREAD_WB '-DHAL_TSC_MODULE_ENABLED=1' -DDEBUG -DSTM32WB55xx -c -I"C:/Users/patrick/STM32Cube/Repository/Packs/ARM/CMSIS/5.6.0/CMSIS/DSP/Include" -I../Core/Inc -I../STM32_WPAN/App -I../USB_Device/App -I../USB_Device/Target -I../Drivers/STM32WBxx_HAL_Driver/Inc -I../Drivers/STM32WBxx_HAL_Driver/Inc/Legacy -I../Utilities/lpm/tiny_lpm -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/ST/STM32_WPAN -I../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread -I../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/tl -I../Middlewares/ST/STM32_WPAN/interface/patterns/ble_thread/shci -I../Middlewares/ST/STM32_WPAN/utilities -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32WBxx/Include -I../Middlewares/ST/STM32_WPAN/thread/openthread/core/openthread_api -I../Middlewares/ST/STM32_WPAN/thread/openthread/stack/include -I../Middlewares/ST/STM32_WPAN/thread/openthread/stack/include/openthread -I../Middlewares/ST/STM32_WPAN/thread/openthread/stack/include/openthread/platform -I../Drivers/CMSIS/Include -I"C:/ST/STM32CubeIDE_1.5.1/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.7-2018-q2-update.win32_1.5.0.202011040924/tools/arm-none-eabi/include" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble/core/auto" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble/core" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble/svc/Inc" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble/core/template" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble/svc" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble/mesh/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

