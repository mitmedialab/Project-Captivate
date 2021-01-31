################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32wb55cgux.s 

OBJS += \
./Core/Startup/startup_stm32wb55cgux.o 

S_DEPS += \
./Core/Startup/startup_stm32wb55cgux.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/startup_stm32wb55cgux.o: ../Core/Startup/startup_stm32wb55cgux.s
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DARM_MATH_CM4 '-DHAL_TSC_MODULE_ENABLED=1' -c -I"C:/ST/STM32CubeIDE_1.5.1/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.7-2018-q2-update.win32_1.5.0.202011040924/tools/arm-none-eabi/include" -I"C:/Users/patrick/STM32Cube/Repository/Packs/ARM/CMSIS/5.6.0/CMSIS/DSP/Lib/ARM" -I"C:/Users/patrick/STM32Cube/Repository/Packs/ARM/CMSIS/5.6.0/CMSIS/DSP/Lib/GCC" -I"C:/Users/patrick/STM32Cube/Repository/Packs/ARM/CMSIS/5.6.0/CMSIS/DSP/Include" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble/core/auto" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble/core" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble/svc/Inc" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble/core/template" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble/svc" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble/svc/Src" -I"D:/dev/Resenv-Shenzhen-2019-Code/stm32_mainMCU_Code_v4/Middlewares/ST/STM32_WPAN/ble/mesh/Inc" -x assembler-with-cpp -MMD -MP -MF"Core/Startup/startup_stm32wb55cgux.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

