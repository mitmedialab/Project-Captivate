/**
 ******************************************************************************
 * File Name           : intertial_sensing.c
 * Description        : Header for Lights.
 ******************************************************************************

 *
 ******************************************************************************
 */

/* includes -----------------------------------------------------------*/
#include "inertial_sensing.h"
#include "driver_BNO080.h"
#include "math.h"
#include "stm32wbxx_hal.h"
#include "main.h"
#include "inter_processor_comms.h"
#include "captivate_config.h"
#include "String.h"
#include "i2c.h"

/* typedef -----------------------------------------------------------*/

/* defines -----------------------------------------------------------*/

/* macros ------------------------------------------------------------*/

/* function prototypes -----------------------------------------------*/

/* variables -----------------------------------------------*/
float quatI;
float quatJ;
float quatK;
float quatReal;
float rotSample;

float x;
float y;
float z;

/* Functions Definition ------------------------------------------------------*/
void packAndSend(uint8_t dataType, GenericThreeAxisPayload *data);

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

uint8_t activityClasses[9];
uint32_t enableActivities = 0x1F; //Enable all 9 possible activities including Unknown
uint8_t inertialEnabled = 0;

CaptivatePacket *captivatePacket;
uint8_t accPayloadID, gyroPayloadID;

struct inertialData inertialPacket;

//void InertialSensingTask(void *argument) {
//	inertialEnabled = 1;
//#ifndef DONGLE_CODE
//	IMU_begin(BNO080_ADDRESS, IMU_INT_Pin, IMU_INT_GPIO_Port);
//#endif
//
//	uint32_t evt = 0;
//
//	while (1) {
//
//		/********* WAIT FOR START CONDITION FROM MASTER THREAD ************************/
//		osThreadFlagsWait(0x00000001U, osFlagsWaitAny, osWaitForever);
//
//		// configure IMU
//		osDelay(500);
//		IMU_enableRotationVector(ROT_VEC_PERIOD);
//		osDelay(100);
//		IMU_enableActivityClassifier(ACT_CLASS_PERIOD, enableActivities,
//				activityClasses);
//
//
//		// give some time for things to buffer
//		// TODO: remove this to see if it still works fine
//		osDelay(400);
//
//		while (1) {
//
//			// grab packets
//			osDelay(100);
//			osMessageQueueGet(rotationSampleQueueHandle,
//					&inertialPacket.rotationMatrix, 0U, 100);
//			osMessageQueueGet(activitySampleQueueHandle,
//					&inertialPacket.activity, 0U, 0);
//			osMessageQueuePut(inertialSensingQueueHandle, &inertialPacket, 0U,
//					0);
//
//			if (HAL_GPIO_ReadPin(IMU_INT_GPIO_Port, IMU_INT_Pin)
//					== GPIO_PIN_RESET)
//				IMU_dataAvailable();
//
//			// check for break condition
//			evt = osThreadFlagsWait(0x00000002U, osFlagsWaitAny, 0);
//
//			// stop timer and put thread in idle if signal was reset
//			if ((evt & 0x00000002U) == 0x00000002U) {
//
//				// reset IMU
//				IMU_softReset();
//
//				// give some time to ensure no interrupts are handled
//				osDelay(500);
//
//				inertialEnabled = 0;
//
//				// empty queues
//				osMessageQueueReset(inertialSensingQueueHandle);
//				osMessageQueueReset(activitySampleQueueHandle);
//				osMessageQueueReset(rotationSampleQueueHandle);
//
//				// clear any flags
//				osThreadFlagsClear(0x0000000EU);
//
//				// exit and wait for next start condition
//				break;
//			}
//		}
//	}
//}


uint8_t numcalls=0;

GenericThreeAxisPayload *accData;
GenericThreeAxisPayload *gyroData;

void InertialSensingTask_Accel_Gyro(void *argument) {
	inertialEnabled = 1;
#ifndef DONGLE_CODE
	IMU_begin(BNO080_ADDRESS, IMU_INT_Pin, IMU_INT_GPIO_Port);
	//Wait and then disable interrupt
#endif

	uint32_t evt = 0;
	osStatus_t status = osErrorTimeout;
	uint8_t tries = 0;

	while (1) {

		/********* WAIT FOR START CONDITION FROM MASTER THREAD ************************/
		osThreadFlagsWait(0x00000001U, osFlagsWaitAny, osWaitForever);

		osDelay(200);

		accPayloadID = 0,
		gyroPayloadID = 0;

		// configure IMU
		IMU_enableAccelerometer(20);
//		IMU_enableRawAccelerometer(10); // outputs at ((input)/1000) period
		IMU_enableGyro(10);
//		IMU_enableGyro(20);
//		IMU_enableRawGyro(10); // outputs at ((input)/1000) period

		//HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);

		// give some time for things to buffer
		// TODO: remove this to see if it still works fine
		osDelay(400);



		while (1) {

			// get acceleration data if available
			if(osOK == osMessageQueueGet(accSampleQueueHandle,
						     &accData, 0U, 0)){
			    packAndSend(ACC_DATA, accData);
			    status = osOK;
			}

			if(osOK == osMessageQueueGet(gyroSampleQueueHandle,
						     &gyroData, 0U, 0)){
			    packAndSend(GYRO_DATA, gyroData);
			    status = osOK;

			}

			// grab packets
			/*if(status != osOK){
			    osDelay(10);

				if (HAL_GPIO_ReadPin(IMU_INT_GPIO_Port, IMU_INT_Pin)
						== GPIO_PIN_RESET)
					IMU_dataAvailable();
			}*/

			//we enter here if we have no data to process;
			//no need to exit until we succeed
			if(status != osOK){

				//try a bunch, if we still fail
				for (tries=0; tries<100; tries++){
					if (HAL_GPIO_ReadPin(IMU_INT_GPIO_Port, IMU_INT_Pin) == GPIO_PIN_RESET){
								IMU_dataAvailable();
								break;
					}
					osDelay(10);
				}

				if (tries>=99){
					//Deinit, init, IMU_reset, 2x
					for (tries=0; tries<2; tries++){
						osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
						HAL_I2C_DeInit(&hi2c1);
						HAL_I2C_Init(&hi2c1);

						//HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

						osSemaphoreRelease(messageI2C_LockHandle);
						osDelay(200);
						numcalls = 0;
						IMU_softReset();
						osMessageQueueReset(accSampleQueueHandle);
						osMessageQueueReset(gyroSampleQueueHandle);
						osDelay(200);
						//HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
						IMU_enableAccelerometer(20);
						IMU_enableGyro(10);


					}
				}

			}

			status = osErrorTimeout;
//			osMessageQueuePut(inertialSensingQueueHandle, &inertialPacket, 0U,
//					0);


			// check for break condition
			evt = osThreadFlagsWait(0x00000002U, osFlagsWaitAny, 0);

			// stop timer and put thread in idle if signal was reset
			if ((evt & 0x00000002U) == 0x00000002U) {

				// reset IMU
				IMU_softReset();

				// give some time to ensure no interrupts are handled
				osDelay(500);

				inertialEnabled = 0;

				// empty queues
				osMessageQueueReset(accSampleQueueHandle);
				osMessageQueueReset(gyroSampleQueueHandle);

				// clear any flags
				osThreadFlagsClear(0x0000000EU);

				// exit and wait for next start condition
				break;
			}
		}
	}
}


void packAndSend(uint8_t dataType, GenericThreeAxisPayload *data){
  // grab available memory for packet creation
  if(osOK != osMessageQueueGet(capPacketAvail_QueueHandle, &captivatePacket, 0U,
	      10)){
      if(dataType==ACC_DATA){
	  accPayloadID++;
      }
      else if(dataType==GYRO_DATA){
	  gyroPayloadID++;
      }
      return; //no memory available so drop packet
  }

    // copy payload
    memcpy(captivatePacket->payload,
	   data,
	   sizeof(GenericThreeAxisPayload));

    captivatePacket->header.packetType = dataType;
    captivatePacket->header.msFromStart = HAL_GetTick();
    captivatePacket->header.epoch = 0;
    captivatePacket->header.payloadLength = sizeof(GenericThreeAxisPayload);

    // add tick cnt
    if(dataType==ACC_DATA){
	captivatePacket->header.packetID  = accPayloadID;
	accPayloadID++;
    }
    else if(dataType==GYRO_DATA){
	captivatePacket->header.packetID  = gyroPayloadID;
	gyroPayloadID++;
    }

    // put into queue
    osMessageQueuePut(capPacket_QueueHandle,
		    &captivatePacket, 0U, 0);

}


//struct LogMessage calLogMessage;
//void calibrate_tare(void){
//
//	calLogMessage.blinkEnabled = 0;
//	calLogMessage.intertialEnabled = 0;
//	calLogMessage.positionEnabled = 0;
//	calLogMessage.tempEnabled = 0;
//	calLogMessage.status = 0;
//	calLogMessage.logStatus = 0;
//
//	// first calibrate all sensors
//	IMU_calibrateAll();
//
//	// start sending data to server for calibration validation
//	calLogMessage.status = 1;
//	calLogMessage.logStatus = 1;
//	calLogMessage.intertialEnabled = 1;
//
//	osMessageQueuePut(togLoggingQueueHandle, &logMessage, 0U, 0U);
//
//	// wait 30 seconds
//	osDelay(30000);
//
//	// end calibration
//	IMU_endCalibration();
//
//
//}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	// if interrupt is triggered, sample!
	// todo: only do when inertial measurements are enabled?
	if ((GPIO_Pin == IMU_INT_Pin) && numcalls++<8){ //(inertialEnabled == 1)) {
		IMU_dataAvailable();
	}else {
		numcalls=10;
	}
}

/**
 * @brief Thread initialization.
 * @param  None
 * @retval None
 */
void Setup_BNO080(void) {

}

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
 *************************************************************/
