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
	arm-none-eabi-gcc -mcpu=cortex-m4 -g -DHAL_TSC_MODULE_ENABLED -c -x assembler-with-cpp -MMD -MP -MF"Core/Startup/startup_stm32wb55cgux.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

