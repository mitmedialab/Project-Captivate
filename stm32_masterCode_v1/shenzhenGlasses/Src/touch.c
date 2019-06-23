/**
  ******************************************************************************
  * File Name          : touch.c
  * Description        : This file provides code for touch sensing
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "touch.h"
#include "tsc.h"
#include "gpio.h"
#include "stm32wbxx_hal.h"
#include "spi.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"
/*
 * THREADS
 */

void touchSensingThread(void){

  while(1){
    osDelay(100);

    // discharge the touch-sensing IOs
    HAL_TSC_IODischarge(&htsc, ENABLE);
    osDelay(1);

    // sample
    HAL_TSC_Start_IT(&htsc);
  }
}

/*
 * Helper Functions
 */

