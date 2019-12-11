/**
 ******************************************************************************
 * File Name          : camera_detector.h
  * Description        : Header for Lights.
  ******************************************************************************

  *
  ******************************************************************************
 */
#ifndef CAMERA_DETECTOR_H
#define CAMERA_DETECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes -----------------------------------------------------------*/
#include "stdint.h"
#include "stdint.h"
//#include "cmsis_os2.h"
/* typedef -----------------------------------------------------------*/



/* defines -----------------------------------------------------------*/


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/
//osThreadId_t cameraDetectionTaskHandle;
//osMessageQueueId_t cameraDetectionQueueHandle;



/* Functions Definition ------------------------------------------------------*/

//void convertADC_ToColor(uint16_t* diodeSamples, union ColorComplex *setColors);
void cameraDetectionTask(void *argument);

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

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
*************************************************************/





#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
