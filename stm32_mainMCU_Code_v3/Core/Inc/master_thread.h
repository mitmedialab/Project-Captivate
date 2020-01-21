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
#include "blink.h"
#include "stm32wbxx_hal.h"
//#include "position.h"
#include "inter_processor_comms.h"
#include "inertial_sensing.h"
/* typedef -----------------------------------------------------------*/

struct LogPacket
{
	struct blinkData						blink;
	struct parsedSecondaryProcessorPacket 	procData;
//	tempData						temp;
	struct inertialData						inertial;
//	positionData					pos;
	uint32_t						tick_ms;
	uint32_t						epoch;
};

struct LogMessage
{
	uint8_t		status;
	uint8_t		logStatus;
	uint8_t		blinkEnabled;
	uint8_t		tempEnabled;
	uint8_t		intertialEnabled;
	uint8_t		positionEnabled;

};
/* defines -----------------------------------------------------------*/


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/
void packetizeData(struct LogPacket *packet,
		struct blinkData *blink,
		struct positionData *pos,
		struct parsedSecondaryProcessorPacket *processorMsg,
		struct inertialData *inertialMsg);
void MasterThreadTask(void *argument);
void masterExitRoutine(void);
void grabSensorData(void);


uint32_t RTC_ToEpoch(RTC_TimeTypeDef *time, RTC_DateTypeDef *date);

/* variables -----------------------------------------------*/
//osThreadId_t 		threadFrontLightsTaskHandle;
//osMessageQueueId_t	lightsSimpleQueueHandle;
osMessageQueueId_t	togLoggingQueueHandle;
osThreadId_t masterThreadTaskHandle;

osSemaphoreId_t messageI2C_LockSem;

struct LogPacket 		sensorPacket;

struct LogMessage 		togLogMessageReceived;
struct LogMessage 		prevLogMessage;
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
