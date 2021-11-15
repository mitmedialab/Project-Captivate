/**
 ******************************************************************************
 * File Name          : inter_processor_comms.c
 * Description        : This file provides code to talk between primary and secondary MCUs
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "inter_processor_comms.h"
#include "gpio.h"
#include "stm32wbxx_hal.h"
//#include "spi.h"
#include "i2c.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"

#include "master_thread.h"
#include "string.h"

#include "task.h"

#include "captivate_config.h"

///*
// * THREADS
// */

struct secondaryProcessorData receivedPacket;
struct parsedSecondaryProcessorPacket parsedPacket;
extern struct LogPacket sensorPacket;
extern struct LogMessage togLogMessageReceived;
static const struct LogMessage nullMessage = { 0 };
struct LogMessage commandToSend;
static thermopileData_BLE packetPayload;

void InterProcessorTask(void *argument) {
	uint32_t evt = 0;
	uint32_t packetIndex = 0;
	uint32_t payload_ID = 0;
	CaptivatePacket *captivatePacket;

#ifndef DONGLE_CODE
	// ensure secondary processor is not active, trying to send data
	// 		note: this should only happen when debugging and resetting the main processor while secondary is logging
	osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
	while (HAL_I2C_Master_Transmit(&hi2c1, SECONDARY_MCU_ADDRESS << 1,
			(uint8_t*) &nullMessage, sizeof(togLogMessageReceived), 10)
			!= HAL_OK) {
		osSemaphoreRelease(messageI2C_LockHandle);
		osDelay(100);
		osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
		/* todo: the below code is found to be needed because it was found that if the secondary processor
		 * calls HAL_I2C_Slave_Receive_IT after the main processor first attempts to send a message via
		 *  HAL_I2C_Master_Transmit call, the I2C locks up and needs to be reset for the slave (secondary
		 *  processor) to have its RX I2C interrupt callback triggered.
		 *
		 *  This issue needs further investigation since this is an inefficient and unideal fix. A thing to
		 *  try would be to go into the I2C peripheral on the main processor and see what is not being reset
		 *  between HAL_I2C_Master_Transmit retries.

		*/
		HAL_I2C_DeInit(&hi2c1);
		HAL_I2C_Init(&hi2c1);
	};
	osSemaphoreRelease(messageI2C_LockHandle);
#endif
	while (1) {

		evt = osThreadFlagsWait(0x00000001U, osFlagsWaitAny, osWaitForever);

		// if signal was received successfully, start task
		if ((evt & 0x00000001U) == 0x00000001U) {

			// tell secondary processor to start logging (in blocking mode)
			memcpy(&commandToSend, &togLogMessageReceived,
					sizeof(struct LogMessage));
//			osThreadFlagsClear(0x0000000FU);
			osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
			//while(HAL_I2C_Master_Transmit(&hi2c1, SECONDARY_MCU_ADDRESS << 1, (uint8_t *) &commandToSend, sizeof(togLogMessageReceived), 100) != HAL_OK);
			while (HAL_I2C_Master_Transmit(&hi2c1, SECONDARY_MCU_ADDRESS << 1,
					(uint8_t*) &commandToSend, sizeof(togLogMessageReceived),
					10) != HAL_OK) {
				osSemaphoreRelease(messageI2C_LockHandle);
				osDelay(100);
				osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
				/* todo: the below code is found to be needed because it was found that if the secondary processor
				 * calls HAL_I2C_Slave_Receive_IT after the main processor first attempts to send a message via
				 *  HAL_I2C_Master_Transmit call, the I2C locks up and needs to be reset for the slave (secondary
				 *  processor) to have its RX I2C interrupt callback triggered.
				 *
				 *  This issue needs further investigation since this is an inefficient and unideal fix. A thing to
				 *  try would be to go into the I2C peripheral on the main processor and see what is not being reset
				 *  between HAL_I2C_Master_Transmit retries.

				*/
				HAL_I2C_DeInit(&hi2c1);
				HAL_I2C_Init(&hi2c1);
			}
			osDelay(100);
			osSemaphoreRelease(messageI2C_LockHandle);

			// message passing until told to stop
			while (1) {

				// grab an event flag if available
				evt = osThreadFlagsWait(0x00000006U, osFlagsWaitAny,
						osWaitForever);

				// if an interrupt is received indicating a message is waiting to be received
				if ((evt & 0x00000004U) == 0x00000004U) {

					osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);

					// send command packet to MCU
					while (HAL_I2C_Master_Transmit(&hi2c1,
							SECONDARY_MCU_ADDRESS << 1,
							(uint8_t*) &commandToSend,
							sizeof(struct LogMessage), 10) != HAL_OK) {
						osSemaphoreRelease(messageI2C_LockHandle);
						osDelay(100);
						osSemaphoreAcquire(messageI2C_LockHandle,
								osWaitForever);
						/* todo: the below code is found to be needed because it was found that if the secondary processor
						 * calls HAL_I2C_Slave_Receive_IT after the main processor first attempts to send a message via
						 *  HAL_I2C_Master_Transmit call, the I2C locks up and needs to be reset for the slave (secondary
						 *  processor) to have its RX I2C interrupt callback triggered.
						 *
						 *  This issue needs further investigation since this is an inefficient and unideal fix. A thing to
						 *  try would be to go into the I2C peripheral on the main processor and see what is not being reset
						 *  between HAL_I2C_Master_Transmit retries.

						*/
						HAL_I2C_DeInit(&hi2c1);
						HAL_I2C_Init(&hi2c1);
					}
					// wait until transmission is successful
// 					evt = osThreadFlagsWait(0x00000010U, osFlagsWaitAny, osWaitForever);
					// ensure I2C is disabled
// 					HAL_I2C_Master_Abort_IT(&hi2c1, SECONDARY_MCU_ADDRESS << 1);

					// clear receiving flag
					osThreadFlagsClear(0x00000008U);
					// grab packet from secondary MCU
					while (HAL_I2C_Master_Receive_IT(&hi2c1,
							SECONDARY_MCU_ADDRESS << 1,
							(uint8_t*) &receivedPacket,
							sizeof(struct secondaryProcessorData)) != HAL_OK) {
						osSemaphoreRelease(messageI2C_LockHandle);
						osDelay(100);
						osSemaphoreAcquire(messageI2C_LockHandle,
								osWaitForever);
						/* todo: the below code is found to be needed because it was found that if the secondary processor
						 * calls HAL_I2C_Slave_Receive_IT after the main processor first attempts to send a message via
						 *  HAL_I2C_Master_Transmit call, the I2C locks up and needs to be reset for the slave (secondary
						 *  processor) to have its RX I2C interrupt callback triggered.
						 *
						 *  This issue needs further investigation since this is an inefficient and unideal fix. A thing to
						 *  try would be to go into the I2C peripheral on the main processor and see what is not being reset
						 *  between HAL_I2C_Master_Transmit retries.

						*/
						HAL_I2C_DeInit(&hi2c1);
						HAL_I2C_Init(&hi2c1);
					}
// 					taskEXIT_CRITICAL();
					// wait until packet is received
					evt = osThreadFlagsWait(0x0000000AU, osFlagsWaitAny,
							osWaitForever);
					// ensure I2C is disabled
//					HAL_I2C_Master_Abort_IT(&hi2c1, SECONDARY_MCU_ADDRESS << 1);

					osSemaphoreRelease(messageI2C_LockHandle);

//					evt = osThreadFlagsWait(0x00000002U, osFlagsWaitAny, 0);
					// if thread was told to stop, break from while loop!
					if ((evt & 0x00000002U) == 0x00000002U)
						break;

					// package received data into 100ms chunks and put in queue
					parsedPacket.tick_ms = receivedPacket.tick_ms;
					parsedPacket.epoch = receivedPacket.epoch;

					memcpy(&packetPayload.data[packetIndex], &receivedPacket, sizeof(struct secondaryProcessorData));
					packetIndex++;

					if(packetIndex == MAX_THERMAL_ENTRIES){
					  packetIndex = 0;

					  // grab available memory for packet creation
					  if(osOK != osMessageQueueGet(capPacketAvail_QueueHandle, captivatePacket, 0U,
						      5)){
					      payload_ID++;
					      continue; //no memory available so drop packet
					  }

					  // copy payload
					  memcpy(captivatePacket->payload,
							  &packetPayload,
							  sizeof(thermopileData_BLE));

					  captivatePacket->header.packetType = THERMAL_DATA;
					  captivatePacket->header.packetID = payload_ID;
					  captivatePacket->header.msFromStart = HAL_GetTick();
					  captivatePacket->header.epoch = 0;
					  captivatePacket->header.payloadLength = sizeof(thermopileData_BLE);

					  // add tick cnt
					  payload_ID++;

					  // put into queue
					  osMessageQueuePut(capPacket_QueueHandle,
							  (void*) captivatePacket, 0U, 0);
					}

				}

				// stop thread and clear queues
				if ((evt & 0x00000002U) == 0x00000002U) {

					/// clear transmission flag
// 					osThreadFlagsClear(0x00000010U);
					// tell secondary processor to stop logging (in blocking mode)
					osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
					while (HAL_I2C_Master_Transmit(&hi2c1,
							SECONDARY_MCU_ADDRESS << 1, (uint8_t*) &nullMessage,
							sizeof(togLogMessageReceived), 10) != HAL_OK) {
						osSemaphoreRelease(messageI2C_LockHandle);
						osDelay(100);
						osSemaphoreAcquire(messageI2C_LockHandle,
								osWaitForever);
						/* todo: the below code is found to be needed because it was found that if the secondary processor
						 * calls HAL_I2C_Slave_Receive_IT after the main processor first attempts to send a message via
						 *  HAL_I2C_Master_Transmit call, the I2C locks up and needs to be reset for the slave (secondary
						 *  processor) to have its RX I2C interrupt callback triggered.
						 *
						 *  This issue needs further investigation since this is an inefficient and unideal fix. A thing to
						 *  try would be to go into the I2C peripheral on the main processor and see what is not being reset
						 *  between HAL_I2C_Master_Transmit retries.

						*/
						HAL_I2C_DeInit(&hi2c1);
						HAL_I2C_Init(&hi2c1);
					};
					osSemaphoreRelease(messageI2C_LockHandle);
					// wait until transmit is complete
//					evt = osThreadFlagsWait(0x00000010U, osFlagsWaitAny, osWaitForever);

					// empty queue
					osMessageQueueReset(interProcessorMsgQueueHandle);

					packetIndex = 0;
					payload_ID = 0;

					break;
				}

			}
		}
	}
}

//void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c){
//	osThreadFlagsSet(interProcessorTaskHandle, 0x00000008U);
//}

volatile uint8_t test_1 = 0;
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *I2cHandle) {
	// notify sending thread that message has been sent
//	osThreadFlagsSet(sendMsgToMainTaskHandle, 0x00000001U);
	osThreadFlagsSet(interProcTaskHandle, 0x00000010U);
	test_1 = 1;
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *I2cHandle) {
	// notify receiving thread that a message has been received
	osThreadFlagsSet(interProcTaskHandle, 0x00000008U);
}
