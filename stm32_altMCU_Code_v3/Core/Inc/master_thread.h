/**
 ******************************************************************************
 * File Name          : master_thread.h
  * Description        : the master thread that governs the system.
  ******************************************************************************

  *
  ******************************************************************************
 */
#ifndef MASTER_THREAD_H
#define MASTER_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes -----------------------------------------------------------*/
#include "stdint.h"
#include "cmsis_os.h"
#include "config.h"
//#include "blink.h"
#include "stm32f3xx_hal.h"
#include "thermopile.h"
//#include "inertial_sensing.h"
//#include "position.h"
//#include "inter_processor_comms.h"
/* typedef -----------------------------------------------------------*/

#define SAMPLE_DISABLE		0
#define SAMPLE_ENABLE		1

//struct LogPacket
//{
//	struct thermopilePackagedData			temp;
////	tempData				temp;
////	inertialData			inertial;
////	positionData			pos;
//	uint32_t				tick_ms;
//	uint32_t				epoch;
//};

struct LogMessage
{
	uint8_t		status;
	uint8_t		logStatus;
	uint8_t		blinkEnabled;
	uint8_t		tempEnabled;
	uint8_t		intertialEnabled;
	uint8_t		positionEnabled;

};

struct secondaryProcessorData
{
	struct thermopilePackagedData	temp;
//	struct inertialData				inertial;
	uint32_t						tick_ms;
	uint32_t						epoch;
};

/* defines -----------------------------------------------------------*/


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/
void packetizeData(struct secondaryProcessorData *packet,
		struct thermopilePackagedData *temp);

void MasterThreadTask(void *argument);


uint32_t RTC_ToEpoch(RTC_TimeTypeDef *time, RTC_DateTypeDef *date);

/* variables -----------------------------------------------*/
//osThreadId_t 		threadFrontLightsTaskHandle;
//osMessageQueueId_t	lightsSimpleQueueHandle;
osMessageQueueId_t	togLoggingQueueHandle;
osThreadId_t masterThreadTaskHandle;

osSemaphoreId_t messageI2C_LockSem;


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

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
*************************************************************/





#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
