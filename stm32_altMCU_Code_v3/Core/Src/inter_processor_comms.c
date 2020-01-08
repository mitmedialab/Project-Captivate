/**
 ******************************************************************************
 * File Name           : inter_processor_comms.c
  * Description        :
  ******************************************************************************

  *
  ******************************************************************************
 */


/* includes -----------------------------------------------------------*/
#include "stm32f3xx_hal.h"
#include "main.h"
#include "inter_processor_comms.h"
#include "master_thread.h"
#include "i2c.h"
#include "task.h"
/* typedef -----------------------------------------------------------*/


/* defines -----------------------------------------------------------*/


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/


struct secondaryProcessorData packetReceived;
struct LogMessage logMessage;

uint8_t logTracking = 0;

/* Functions Definition ------------------------------------------------------*/
void SendPacketToMainTask(void *argument){
	uint32_t evt = 0;

	while(1){

		// wait for packet
//		osMessageQueueGet(sendMsgToMainQueueHandle, &packetReceived, 0U, osWaitForever);

//		// don't do anything until I2C is ready
//		while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY){
//			osDelay(50);
//		}

//		taskENTER_CRITICAL();

//		// trigger interrupt line to MCU to tell it that a packet is ready
//		HAL_GPIO_WritePin(EXPANSION_INT_GPIO_Port, EXPANSION_INT_Pin, GPIO_PIN_SET);

		// wait for Master to send command to slave
//		osSemaphoreAcquire(interprocessMessageLockSem, osWaitForever);
//		while(HAL_OK != HAL_I2C_Slave_Receive(&hi2c1, (uint8_t *) &logMessage, sizeof(struct LogMessage), 100)){
//			osSemaphoreRelease(interprocessMessageLockSem);
//			osDelay(100);
//			osSemaphoreAcquire(interprocessMessageLockSem, osWaitForever);
//		}
//		osSemaphoreRelease(interprocessMessageLockSem);


		while(HAL_I2C_Slave_Receive_IT(&hi2c1, (uint8_t *) &logMessage, sizeof(struct LogMessage)) != HAL_OK);

		evt = osThreadFlagsWait (0x00000004U, osFlagsWaitAny, osWaitForever);

		// if master is requesting to stop sampling, stop
		if(logMessage.status == SAMPLE_DISABLE){
			logTracking = 0;

			// stop sampling
			osMessageQueuePut(togLoggingQueueHandle, &logMessage, 0U, osWaitForever);
			HAL_GPIO_WritePin(EXPANSION_INT_GPIO_Port, EXPANSION_INT_Pin, GPIO_PIN_RESET);
		}

		// if master is requesting to start sampling
		else if(logMessage.status == SAMPLE_ENABLE && logTracking==0)
		{
			logTracking = 1;

			// start sampling
			osMessageQueuePut(togLoggingQueueHandle, &logMessage, 0U, osWaitForever);
		}

		// if logging is already happening and a message is ready
		else
		{
			if(osMessageQueueGet(sendMsgToMainQueueHandle, &packetReceived, 0U, 0) == osOK)
			{
				while(HAL_I2C_Slave_Transmit_IT(&hi2c1, (uint8_t *) &packetReceived, sizeof(struct secondaryProcessorData)) != HAL_OK);

				// wait for transmit to succeed
				evt = osThreadFlagsWait (0x00000001U, osFlagsWaitAny, osWaitForever);

				// reset trigger
				HAL_GPIO_WritePin(EXPANSION_INT_GPIO_Port, EXPANSION_INT_Pin, GPIO_PIN_RESET);
			}
			else
			{
				// reset trigger
				HAL_GPIO_WritePin(EXPANSION_INT_GPIO_Port, EXPANSION_INT_Pin, GPIO_PIN_RESET);
			}
		}

		osDelay(10);

		// if there is another message queued up, assert interrupt pin
		if( osMessageQueueGetCount(sendMsgToMainQueueHandle) > 0)
		{
			HAL_GPIO_WritePin(EXPANSION_INT_GPIO_Port, EXPANSION_INT_Pin, GPIO_PIN_SET);
		}

//		// force a delay
//		osDelay(50);
	}
}

//void ReceivePacketFromMainTask(void *argument){
//	uint32_t evt = 0;
//
//	while(1){
//
////		if(HAL_GPIO_ReadPin(EXPANSION_INT_GPIO_Port, EXPANSION_INT_Pin) == GPIO_PIN_SET){
////			osDelay(30);
////		}
////
////		taskENTER_CRITICAL();
////		while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY){
////		}
////
////		// receive packet via I2C
////		while(HAL_I2C_Slave_Receive_IT(&hi2c1, (uint8_t *) &logMessage, sizeof(struct LogMessage), 50) != HAL_OK){
////			taskEXIT_CRITICAL();
////			osDelay(500);
////			taskENTER_CRITICAL();
////		}
////		taskEXIT_CRITICAL();
////		// wait 50ms until I2C message is received (depending on implementation, the 50ms is useless if polling above)
//////		evt = osThreadFlagsWait (0x00000001U, osFlagsWaitAny, 50);
//////
//////		if(evt != osFlagsErrorTimeout)
//////		{
////		// put received message in queue for master thread to handle
////		osMessageQueuePut(togLoggingQueueHandle, &logMessage, 0U, osWaitForever);
//////		}
//
//		// force a delay
//		osDelay(500);
//	}
//}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	// notify sending thread that message has been sent
	osThreadFlagsSet(sendMsgToMainTaskHandle, 0x00000001U);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	//osThreadFlagsSet(sendMsgToMainTaskHandle, 0x00000004U);
	// notify receiving thread that a message has been received
//	osThreadFlagsSet(receivePacketFromMainTaskHandle, 0x00000001U);
}

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
