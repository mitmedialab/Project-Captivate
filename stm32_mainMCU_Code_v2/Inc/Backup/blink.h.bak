/**
 ******************************************************************************
 * File Name          : blink.h
  * Description        : Header for Lights.
  ******************************************************************************

  *
  ******************************************************************************
 */
#ifndef BLINK_H
#define BLINK_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes -----------------------------------------------------------*/
#include "stdint.h"
#include "cmsis_os.h"
#include "stm32wbxx_hal.h"

/* typedef -----------------------------------------------------------*/
struct blinkData{
	uint8_t			data[100];
	uint32_t		tick_ms;
};

/* defines -----------------------------------------------------------*/


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/
osThreadId_t blinkTaskHandle;
osMessageQueueId_t	blinkMsgQueueHandle;

//osSemaphoreDef(blinkSemDef);
//osSemaphoreId (blinkSemaphore);

//osSemaphoreId osSemaphoreCreate (blinkSemDef, 0);
//
//osSemaphoreWait(multiplex_id, osWaitForever);
//        // do something
//        osSemaphoreRelease(blinkSemaphore);

/* Functions Definition ------------------------------------------------------*/



/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
 * @brief Thread initialization.
 * @param  None
 * @retval None
 */
void BlinkTask(void);

/**
 * @brief blink setup.
 * @param  None
 * @retval None
 */
void SetupBlinkSensing(void);

/**
 * @brief DMA Transfer Complete Callback.
 * @param  None
 * @retval None
 */
//void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc);
//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
*************************************************************/







#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
