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
#include "cmsis_os2.h"

/* typedef -----------------------------------------------------------*/

/* defines -----------------------------------------------------------*/
// uncomment if programming the STM32 USB dongle
#define DONGLE_CODE					1
#define OTA_ENABLED					1

#ifdef DONGLE_CODE
#define NODE_TYPE				"dongle"  // max 11 bytes
#define NODE_DESCRIPTION  		"don_1"	  // max 11 bytes
#else
#define NODE_TYPE				"captivates"  // max 11 bytes
#define NODE_DESCRIPTION  		"cap_1"	  // max 11 bytes
#endif

//// enable sensing subsystems
#define BLINK_SENSING_ENABLE		1
#define TEMP_SENSING_ENABLE			1
#define POS_SENSING_ENABLE			1
#define	INERTIA_SENSING_ENABLE		1

#define DISABLE_SENSING				0

/* SYSTEM GLOBAL CONFIGURATION */
#define PACKET_SEND_PERIOD			100

#define C_LIGHT_RESSOURCE						"light"
#define C_LIGHTS_SIMPLE_RESSOURCE				"lightS"
#define C_LIGHTS_COMPLEX_RESSOURCE				"lightC"
#define C_BORER_TIME_RESSOURCE					"borderTime"
#define C_NODE_INFO_RESSOURCE					"nodeInfo"
#define C_TOGGLE_LOGGING_RESSOURCE				"togLog"

#ifdef OTA_ENABLED
#define C_RESSOURCE_FUOTA_PROVISIONING     "FUOTA_PROVISIONING"
#define C_RESSOURCE_FUOTA_PARAMETERS       "FUOTA_PARAMETERS"
#define C_RESSOURCE_FUOTA_SEND             "FUOTA_SEND"

#define FUOTA_NUMBER_WORDS_64BITS                50
#define FUOTA_PAYLOAD_SIZE                FUOTA_NUMBER_WORDS_64BITS * 8
#endif

#define ENABLE_LOG								1
#define DISABLE_LOG								0
#define SENSOR_ENABLE							1

/* RADIO SPECIFIC */
#define TRANSMIT_POWER							6 //in dbm
#define CHILD_SUPERVISION_INTERVAL				2 // default is 129 (how often a router broadcasts to its child to ensure its alive)
#define CHILD_SUPERVISION_TIMEOUT				3	// default is 190 (when child trying to find a new router)

/* WATCHDOG TIMER */
#define WATCHDOG_PERIOD							5000 // 5 seconds


/* ************************************************************************** */
/* ************************************************************************** */
/* ************************************************************************** */

/* VIVE SPECIFIC CONFIGURATION */
#define VIVE_SAMPLE_PERIOD			2000
#define	GET_3D_LOC_TIMEOUT			1500
//#define VIVE_THREAD_INFINITE_TIMEOUT	1

/* TOUCH SPECIFIC CONFIGURATION */
#define ALPHA_WEIGHT 	0.01    // updated_va = new_val * (ALPHA_WEIGHT) + old_val * (1-ALPHA_WEIGHT)
#define CALIBRATION_SAMPLES 100 // how many samples to take before starting sensing
#define THRESHOLD_TOLERANCE	60 // how much below the dynamic threshold before classifying as a "touch"

//#define TSCx_TS1_MINTHRESHOLD			0
//#define TSCx_TS1_MAXTHRESHOLD			7400
//#define TSCx_TS2_MINTHRESHOLD			0
//#define TSCx_TS2_MAXTHRESHOLD			8800

/* IMU SPECIFIC CONFIGURATION */
#define ROT_VEC_PERIOD				100
#define ACT_CLASS_PERIOD			500
#define STEP_CNT_PERIOD				500
#define STABILITY_CLASS_PERIOD		500

/* BLINK SPECIFIC CONFIGURATION */
#define BLINK_HALF_BUFFER_SIZE	1000
#define BLINK_PACKET_SIZE		100
#define BLINK_ITERATOR_COUNT 	BLINK_HALF_BUFFER_SIZE / BLINK_PACKET_SIZE



/* macros ------------------------------------------------------------*/

/* function prototypes -----------------------------------------------*/

/* freertos types  -----------------------------------------------*/

extern osThreadId_t blinkTaskHandle;
extern osMessageQueueId_t blinkMsgQueueHandle;

extern osThreadId_t frontLightsTaskHandle;
extern osMessageQueueId_t lightsSimpleQueueHandle;

extern osMessageQueueId_t togLoggingQueueHandle;
extern osThreadId_t masterTaskHandle;

extern osSemaphoreId_t messageI2C_LockHandle;

extern osThreadId_t inertialTaskHandle;
extern osMessageQueueId_t inertialSensingQueueHandle;

extern osMessageQueueId_t activitySampleQueueHandle;
extern osMessageQueueId_t rotationSampleQueueHandle;

extern osThreadId_t interProcTaskHandle;
extern osMessageQueueId_t interProcessorMsgQueueHandle;

extern osThreadId_t pulseTaskHandle;

extern osTimerId_t viveTimerHandle;

extern osMessageQueueId_t statusQueueHandle;

extern osSemaphoreId_t locNotifyHandle;

extern osSemaphoreId_t locCompleteHandle;

extern void startSensorThreads(void);

/* variables -----------------------------------------------*/
struct SystemStatus {
	int blinkThread :1;
	int inertialThread :1;
	int interProcThread :1;
	int frontLightsThread :1;
};

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
