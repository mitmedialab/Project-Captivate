/**
  @page RCC_OutputSystemClockOnMCO RCC example
  
  @verbatim
  ******************************************************************************
  * @file    Examples_LL/RCC/RCC_OutputSystemClockOnMCO/readme.txt 
  * @author  MCD Application Team
  * @brief   Description of the RCC example.
  ******************************************************************************
  *
  * Copyright (c) 2019 STMicroelectronics. All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  @endverbatim

@par Example Description

Configuration of MCO pin (PA8) to output the system clock.

At start-up, User push-button (SW1) and MCO pin are configured. The program configures SYSCLK
to the max frequency using the PLL with MSI as clock source.

The signal on PA8 (or pin 25 of CN10 connector) can be monitored with an oscilloscope
to check the different MCO configuration set at each User push-button (SW1) press.
Different configuration will be observed
 - SYSCLK frequency with frequency value around @64MHz.
 - PLLCLK frequency value divided by 4, hence around @16MHz.
 - MSI frequency value divided by 2, hence around @2MHz.

When user press User push-button, a LED2 toggle is done to indicate a change in MCO config.

@par Keywords

System, RCC, PLL, HSI, PLLCLK, SYSCLK, HSE, Clock, Oscillator


@par Directory contents 

  - RCC/RCC_OutputSystemClockOnMCO/Inc/stm32wbxx_it.h          Interrupt handlers header file
  - RCC/RCC_OutputSystemClockOnMCO/Inc/main.h                  Header for main.c module
  - RCC/RCC_OutputSystemClockOnMCO/Inc/stm32_assert.h          Template file to include assert_failed function
  - RCC/RCC_OutputSystemClockOnMCO/Src/stm32wbxx_it.c          Interrupt handlers
  - RCC/RCC_OutputSystemClockOnMCO/Src/main.c                  Main program
  - RCC/RCC_OutputSystemClockOnMCO/Src/system_stm32wbxx.c      STM32WBxx system source file


@par Hardware and Software environment

  - This example runs on STM32WB55xx devices.
    
  - This example has been tested with P-NUCLEO-WB55 board and can be
    easily tailored to any other supported device and development board.

  - P-NUCLEO-WB55 Set-up
    - Connect the MCO pin to an oscilloscope to monitor the different waveforms:
      - PA.08: connected to pin 25 of CN10 connector for Nucleo-68  (MB1355)

@par How to use it ? 

In order to make the program work, you must do the following :
 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the example

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
