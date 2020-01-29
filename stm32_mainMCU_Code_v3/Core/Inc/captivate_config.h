/**
 ******************************************************************************
 * File Name          : config.h
  * Description        : configuration file
  ******************************************************************************

  *
  ******************************************************************************
 */
#ifndef CAPTIVATE_CONFIG_H
#define CAPTIVATE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes -----------------------------------------------------------*/

/* typedef -----------------------------------------------------------*/
//struct inertialData{
//	uint8_t			data[100];
//	uint32_t		tick_ms;
//};
//

struct tempData{
	uint8_t			data[100];
	uint32_t		tick_ms;
};

/* defines -----------------------------------------------------------*/

// enable sensing subsystems
#define BLINK_SENSING_ENABLE		1
#define TEMP_SENSING_ENABLE			0
#define POS_SENSING_ENABLE			0
#define	INERTIA_SENSING_ENABLE		0

#define DISABLE_SENSING				0

// uncomment if programming the STM32 USB dongle
//#define DONGLE_CODE					1

/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/

/* freertos types  -----------------------------------------------*/

extern osThreadId_t blinkTaskHandle;
extern osMessageQueueId_t	blinkMsgQueueHandle;

extern osThreadId_t 		frontLightsTaskHandle;
extern osMessageQueueId_t	lightsSimpleQueueHandle;

extern osMessageQueueId_t	togLoggingQueueHandle;
extern osThreadId_t masterTaskHandle;

extern osSemaphoreId_t messageI2C_LockHandle;

extern osThreadId_t inertialTaskHandle;
extern osMessageQueueId_t inertialSensingQueueHandle;

extern osMessageQueueId_t activitySampleQueueHandle;
extern osMessageQueueId_t rotationSampleQueueHandle;

extern osThreadId_t interProcTaskHandle;
extern osMessageQueueId_t	 interProcessorMsgQueueHandle;

extern osThreadId_t pulseTaskHandle;

extern void startSensorThreads(void);

/* variables -----------------------------------------------*/


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
