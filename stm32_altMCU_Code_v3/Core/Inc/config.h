/**
 ******************************************************************************
 * File Name          : config.h
  * Description        : configuration file
  ******************************************************************************

  *
  ******************************************************************************
 */
#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes -----------------------------------------------------------*/

/* typedef -----------------------------------------------------------*/
struct inertialData{
	uint8_t			data[100];
	uint32_t		tick_ms;
};

struct positionData{
	uint8_t			data[100];
	uint32_t		tick_ms;
};

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

#define ENABLE_LOG					1
#define DISABLE_LOG					0
#define SENSOR_ENABLE				1
// uncomment if programming the STM32 USB dongle
//#define DONGLE_CODE					1

/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


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
