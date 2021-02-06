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
#include <captivate_config.h>
#include "blink.h"
#include "stm32wbxx_hal.h"
#include "inter_processor_comms.h"
#include "inertial_sensing.h"
#include "messages.h"
/* typedef -----------------------------------------------------------*/

struct LogPacket {
	struct blinkData blink;
	struct parsedSecondaryProcessorPacket procData;
	struct inertialData inertial;
	VIVEVars pos;
	uint32_t tick_ms;
	uint32_t epoch;
};

struct LogMessage {
	uint8_t status;
	uint8_t logStatus;
	uint8_t blinkEnabled;
	uint8_t tempEnabled;
	uint8_t intertialEnabled;
	uint8_t positionEnabled;

};
/* defines -----------------------------------------------------------*/

/* macros ------------------------------------------------------------*/

/* function prototypes -----------------------------------------------*/
void packetizeData(struct LogPacket *packet, struct blinkData *blink,
		struct parsedSecondaryProcessorPacket *processorMsg,
		struct inertialData *inertialMsg, VIVEVars *posMsg);
void MasterThreadTask(void *argument);
void masterExitRoutine(void);
void grabSensorData(void);
void masterEnterRoutine(void);

uint32_t RTC_ToEpoch(RTC_TimeTypeDef *time, RTC_DateTypeDef *date);

/* variables -----------------------------------------------*/

struct LogPacket sensorPacket;

struct LogMessage togLogMessageReceived;
struct LogMessage prevLogMessage;

extern uint32_t viveStateVar;
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
