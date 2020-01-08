/**
 ******************************************************************************
 * File Name          : inertial_sensing.h
  * Description        : Header for Lights.
  ******************************************************************************

  *
  ******************************************************************************
 */
#ifndef INERTIAL_SENSING_H
#define INERTIAL_SENSING_H


#include "stdint.h"
#include "cmsis_os.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

/* includes -----------------------------------------------------------*/

/* typedef -----------------------------------------------------------*/

struct rotationData{
	float quatI;
	float quatJ;
	float quatK;
	float quatReal;
	float quatRadianAccuracy;
	uint32_t tick_ms;
};

struct activityData{
	uint8_t activityConfidence[9];
	uint32_t tick_ms;
};

struct stabilityData{
	uint8_t stabilityClass;
	uint32_t tick_ms;
};

struct stepData{
	uint16_t stepCount;
	uint32_t tick_ms;
};

struct inertialData{
	struct rotationData rotationMatrix[5];
	struct activityData activity;
	struct stabilityData stability;
	struct stepData	step;

};


/* defines -----------------------------------------------------------*/


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/
osThreadId_t inertialSensingTaskHandle;
osMessageQueueId_t inertialSensingQueueHandle;

osMessageQueueId_t activitySampleQueueHandle;
osMessageQueueId_t rotationSampleQueueHandle;
osMessageQueueId_t stepSampleQueueHandle;
osMessageQueueId_t stabilitySampleQueueHandle;

/* Functions Definition ------------------------------------------------------*/
void InertialSensingTask(void *argument);
void Setup_BNO080(void);



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





//#ifdef __cplusplus
//} /* extern "C" */
//#endif

#endif
