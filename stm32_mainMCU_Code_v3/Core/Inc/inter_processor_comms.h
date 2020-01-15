/**
 ******************************************************************************
 * File Name          : inter_processor_comms.h
  * Description        :
  ******************************************************************************

  *
  ******************************************************************************
 */
#ifndef INTER_PROCESSOR_COMMS_H
#define INTER_PROCESSOR_COMMS_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes -----------------------------------------------------------*/
#include "stdint.h"
#include "cmsis_os.h"
#include "stm32wbxx_hal.h"
/* typedef -----------------------------------------------------------*/


/***** TEMPERATURE SPECIFIC DATATYPES ************************/

struct thermopileData{
	uint16_t thermopile;
	uint16_t thermistor;
	uint32_t tick_ms;
};

struct thermopilePackagedData{
	struct thermopileData temple[5];
	struct thermopileData nose[5];
};

/***** IMU SPECIFIC DATATYPES *********************************/

//struct rotationData{
//	float quatI;
//	float quatJ;
//	float quatK;
//	float quatReal;
//	float quatRadianAccuracy;
//	uint32_t tick_ms;
//};

//struct inertialInferenceData{
//	uint8_t activityConfidence[9];
//	uint32_t tick_ms_activity;
//
//	uint8_t stabilityClass;
//	uint32_t tick_ms_stability;
//
//	uint16_t stepCount;
//	uint32_t tick_ms_step;
//};

//struct inertialData{
//	struct rotationData rotationMatrix[5];
//	struct inertialInferenceData inferenceInfo;
//};

/***** COMPILED DATA PACKET FROM SECONDARY MCU *******************/

struct secondaryProcessorData
{
	struct thermopilePackagedData	temp;
//	struct inertialData				inertial;
	uint32_t						tick_ms;
	uint32_t						epoch;
};

/***** PARSEDPACKET FROM SECONDARY MCU *******************/

struct parsedSecondaryProcessorPacket{
	struct thermopileData temple;
	struct thermopileData nose;
//	struct rotationData rotationMatrix;
//	struct inertialInferenceData inferenceInfo;
	uint32_t tick_ms;
	uint32_t epoch;
};



/* defines -----------------------------------------------------------*/
#define SECONDARY_MCU_ADDRESS 	0x73


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/
osThreadId_t interProcessorTaskHandle;
osMessageQueueId_t	 interProcessorMsgQueueHandle;

osSemaphoreId_t	interProcessorInterruptSemHandle;


/* Functions Definition ------------------------------------------------------*/
void InterProcessorTask(void *argument);


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
