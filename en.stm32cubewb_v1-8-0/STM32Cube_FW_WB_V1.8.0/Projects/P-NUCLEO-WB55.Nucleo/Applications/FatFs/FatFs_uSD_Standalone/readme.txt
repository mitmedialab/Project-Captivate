/**
  @page FatFs_uSD_Standalone  FatFs with uSD card drive application

  @verbatim
  ******************************************************************************
  * @file    FatFS/FatFS_uSD_Standalone/readme.txt
  * @author  MCD Application Team
  * @brief   Description of the FatFs with uSD card drive application
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  @endverbatim

@par Application Description

How to use STM32Cube firmware with FatFs middleware component as a generic FAT
file system module. This example develops an application that exploits FatFs
features to configure a microSD drive.

At the beginning of the main program the HAL_Init() function is called to reset
all the peripherals, initialize the Flash interface and the systick.
Then the SystemClock_Config() function is used to configure the system clock
(SYSCLK) to run at 64 MHz.

The application is based on writing and reading back a text file from a drive,
and it's performed using FatFs APIs to access the FAT volume as described
in the following steps:

 - Link the uSD disk I/O driver;
 - Register the file system object (mount) to the FatFs module for the uSD drive;
 - Create a FAT file system (format) on the uSD drive;
 - Create and Open new text file object with write access;
 - Write data to the text file;
 - Close the open text file;
 - Open text file object with read access;
 - Read back data from the text file;
 - Close the open text file;
 - Check on read data from text file;
 - Unlink the uSD disk I/O driver.

It is worth nothing that the application manages any error occurred during the
access to FAT volume, when using FatFs APIs. Otherwise, user can check if the
written text file is available on the uSD card.

It is possible to fine tune needed FatFs features by modifying defines values
in FatFs configuration file "ffconf.h" available under the project includes
directory, in a way to fit the application requirements.

@note: for some uSD's, replacing it  while the application is running makes the application
       fail. It is recommended to reset the board using the "Reset button" after replacing
       the uSD.

P-NUCLEO-WB55's LED can be used to monitor the application status:
  - LED2 is ON when the application runs successfully.
  - LED3 is ON when any error occurs.

@par Keywords

Middleware, SD Card, FatFs, File system, FAT Volume, Format, Mount, Read, Write,

@note SD module is part of the Adafruit shield.

@note Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
      based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
      a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
      than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
      To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.

@note The application needs to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.

@par Directory contents

  - FatFs/FatFs_uSD_Standalone/Core/Inc/stm32wbxx_hal_conf.h               HAL configuration file
  - FatFs/FatFs_uSD_Standalone/Core/Inc/stm32wbxx_it.h                     Interrupt handlers header file
  - FatFs/FatFs_uSD_Standalone/Core/Inc/main.h                             Header for main.c module
  - FatFs/FatFs_uSD_Standalone/Core/Src/stm32wbxx_it.c                     Interrupt handlers
  - FatFs/FatFs_uSD_Standalone/Core/Src/main.c                             Main program
  - FatFs/FatFs_uSD_Standalone/Core/Src/system_stm32wbxx.c                 STM32WBxx system source file
  - FatFs/FatFs_uSD_Standalone/FATFS/Target/ffconf.h                       FAT file system module configuration file
  - FatFs/FatFs_uSD_Standalone/FATFS/Target/sd_diskio.h                    uSD diskio header file
  - FatFs/FatFs_uSD_Standalone/FATFS/Target/sd_diskio.c                    FatFs uSD diskio driver
  - FatFs/FatFs_uSD_Standalone/FATFS/App/app_fatfs.h                       Header file for App_fatfs.c file
  - FatFs/FatFs_uSD_Standalone/FATFS/App/app_fatfs.c                       FatFs application code

@par Hardware and Software environment

  - This application runs on STM32WB55RGVx devices

  - This application has been tested with P-NUCLEO-WB55 board and can be
    easily tailored to any other supported device and development board.

@par How to use it ?

In order to make the program work, you must do the following:
 - Insert a microSD card in the board appropriate slot from Adafruit shield
 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory
 - Run the application

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
