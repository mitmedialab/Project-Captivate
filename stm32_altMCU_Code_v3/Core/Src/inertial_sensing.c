/**
 ******************************************************************************
 * File Name           : intertial_sensing.c
  * Description        : Header for Lights.
  ******************************************************************************

  *
  ******************************************************************************
 */


/* includes -----------------------------------------------------------*/
#include <inertial_sensing.h>
#include "driver_BNO080.h"
#include "math.h"
#include "stm32f3xx_hal.h"
#include "main.h"
#include "inter_processor_comms.h"

/* typedef -----------------------------------------------------------*/


/* defines -----------------------------------------------------------*/


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/
//BNO080 inertialSensor;

float quatI;
float quatJ;
float quatK;
float quatReal;
float rotSample;

float x;
float y;
float z;

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

#define ROT_VEC_PERIOD				1000
#define ACT_CLASS_PERIOD			500
#define STEP_CNT_PERIOD				500
#define STABILITY_CLASS_PERIOD		500

#define WAIT_TOLERANCE				50

uint8_t activityClasses[9];
uint32_t enableActivities = 0x1F; //Enable all 9 possible activities including Unknown

struct inertialData inertialPacket;

void InertialSensingTask(void *argument){
	osSemaphoreAcquire(interprocessMessageLockSem, osWaitForever);
	IMU_begin(BNO080_ADDRESS, IMU_INT_Pin, IMU_INT_GPIO_Port);
	osSemaphoreRelease(interprocessMessageLockSem);

//	IMU_enableAccelerometer(50); //Send data update every 50ms

//	while(1){
//		 if (IMU_dataAvailable() == true)
//		  {
////			 x = IMU_getAccelX();
////			 y = IMU_getAccelY();
////			 z = IMU_getAccelZ();
//			quatI = IMU_getQuatI();
//			quatJ = IMU_getQuatJ();
//			quatK = IMU_getQuatK();
//			quatReal = IMU_getQuatReal();
//			quatRadianAccuracy = IMU_getQuatRadianAccuracy();
//		  }
//	}

	//  sensorChoice sensor = nose;
	  uint32_t evt = 0;

	  while(1){

		  	/********* WAIT FOR START CONDITION FROM MASTER THREAD ************************/
			evt = osThreadFlagsWait (0x00000001U, osFlagsWaitAny, osWaitForever);
		  	//evt = 0x00000001U;
		    // if signal was received successfully, start blink task
			if (evt == 0x00000001U)  {

				// configure IMU
				IMU_enableRotationVector(ROT_VEC_PERIOD);
				IMU_enableActivityClassifier(ACT_CLASS_PERIOD , enableActivities, activityClasses);
				IMU_enableStabilityClassifier(STABILITY_CLASS_PERIOD);
				IMU_enableStepCounter(STEP_CNT_PERIOD);

//				 give IMU some time to get setup
				//osDelay(5);
				uint8_t temp = 0;
				while(1){
					if(IMU_dataAvailable()){
						temp++;
					}else{

					}
//					// grab packets
//					osMessageQueueGet(activitySampleQueueHandle, &inertialPacket.rotationMatrix[0], 0U, ROT_VEC_PERIOD + WAIT_TOLERANCE);
//					osMessageQueueGet(activitySampleQueueHandle, &inertialPacket.rotationMatrix[1], 0U, ROT_VEC_PERIOD + WAIT_TOLERANCE);
//					osMessageQueueGet(activitySampleQueueHandle, &inertialPacket.rotationMatrix[2], 0U, ROT_VEC_PERIOD + WAIT_TOLERANCE);
//					osMessageQueueGet(activitySampleQueueHandle, &inertialPacket.rotationMatrix[3], 0U, ROT_VEC_PERIOD + WAIT_TOLERANCE);
//					osMessageQueueGet(activitySampleQueueHandle, &inertialPacket.rotationMatrix[4], 0U, ROT_VEC_PERIOD + WAIT_TOLERANCE);
//					osMessageQueueGet(rotationSampleQueueHandle, &inertialPacket.activity, 0U, WAIT_TOLERANCE);
//					osMessageQueueGet(stepSampleQueueHandle, &inertialPacket.stability, 0U, WAIT_TOLERANCE);
//					osMessageQueueGet(stabilitySampleQueueHandle, &inertialPacket.step, 0U, WAIT_TOLERANCE);
//
//					// check for break condition
//					evt = osThreadFlagsWait (0x00000001U, osFlagsWaitAny, 0);
//
//					if( (evt & 0x00000002U) == 0x00000002U){
//						break;
//					}
//					// if not told to shut down sampling
//					else{
//						// send data to master thread
////						osMessageQueuePut(inertialSensingQueueHandle, &inertialPacket, 0U, 0);
//					}
//
//
				}
//
//				// stop timer and put thread in idle if signal was reset
//				if( (evt & 0x00000002U) == 0x00000002U){
//
//					// reset IMU
//					IMU_softReset();
//
//					// empty queues
//					osMessageQueueReset(inertialSensingQueueHandle);
//					osMessageQueueReset(activitySampleQueueHandle);
//					osMessageQueueReset(rotationSampleQueueHandle);
//					osMessageQueueReset(stepSampleQueueHandle);
//					osMessageQueueReset(stabilitySampleQueueHandle);
//
//					// clear any flags
//					osThreadFlagsClear(0x0000000EU);
//				}
			}
		}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // if interrupt is triggered, sample!
	if(GPIO_Pin == GPIO_PIN_5){
		IMU_dataAvailable();
	}
}

/**
 * @brief Thread initialization.
 * @param  None
 * @retval None
 */
void Setup_BNO080(void){

}


/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
*************************************************************/
