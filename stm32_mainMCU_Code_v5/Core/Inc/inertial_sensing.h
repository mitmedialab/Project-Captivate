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

struct rotationData {
	float quatI;
	float quatJ;
	float quatK;
	float quatReal;
	float quatRadianAccuracy;
	uint32_t tick_ms;
};

struct genericThreeAxisData {
	uint16_t x;
	uint16_t y;
	uint16_t z;
	uint16_t imu_tick_ms;
	uint32_t tick_ms;
	float accuracy;
//	uint32_t tick_ms;
};


struct activityData {
	uint8_t activityConfidence[9];
	uint32_t tick_ms;
};

struct stabilityData {
	uint8_t stabilityClass;
	uint32_t tick_ms;
};

struct stepData {
	uint16_t stepCount;
	uint32_t tick_ms;
};

struct inertialData {
	struct rotationData rotationMatrix;
	struct activityData activity;
//	struct stabilityData stability;
//	struct stepData	step;

};

/* defines -----------------------------------------------------------*/

/* macros ------------------------------------------------------------*/

/* function prototypes -----------------------------------------------*/

/* variables -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
void InertialSensingTask(void *argument);
void InertialSensingTask_Accel_Gyro(void *argument);
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
