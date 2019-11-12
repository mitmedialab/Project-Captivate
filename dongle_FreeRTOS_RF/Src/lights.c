/**
 ******************************************************************************
  * File Name          : lights.c
  * Description        : Lights control.
  ******************************************************************************

  *
  ******************************************************************************
  */

/* includes -----------------------------------------------------------*/
#include "lights.h"
#include "main.h"
#include "stm32wbxx_hal.h"
//#include "stm32wbxx_hal_gpio.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"

/* typedef -----------------------------------------------------------*/


/* defines -----------------------------------------------------------*/


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/


/* Functions Definition ------------------------------------------------------*/

/*
  * @brief  Function activating lights
  * @param  argument: Not used
  * @retval None
  */
void ThreadCheckToggleLightsTask(void *argument)
{
//	LP5523 ledDriver;
//	ledDriver.begin();
	while(1){
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		osDelay(1000);
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		osDelay(1000);
	}
}
