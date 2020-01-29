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

//// CONFIGURATION REGISTER
//#define LMP91051_CFG_REG      0x0
//#define TP_NOSE_SEL           0x00
//#define TP_TEMPLE_SEL         0x80
//#define PGA1_EN               0x40
//#define PGA2_EN               0x20
//#define EXT_FILT_EN           0x10
//#define CMN_MODE_2_59         0x08
//#define CMN_MODE_1_15         0x00
//#define GAIN2_4               0x00
//#define GAIN2_8               0x02
//#define GAIN2_16              0x04
//#define GAIN2_32              0x06
//#define GAIN1_250             0x00
//#define GAIN1_42              0x01
//
//// DAC REGISTER
////   The output DC level will shift according to the formula Vout_shift = -33.8mV * (NDAC - 128)
//#define LMP91051_DAC_REG      0x1
//
////SDIO ENABLE REGISTER
////  To enter SDIO Mode, write the successive sequence 0xFE and 0xED.
////  Write anything other than this sequence to get out of mode.
//#define LMP91051_SDIO_EN_REG  0xF

///*
// * THREADS
// */

struct secondaryProcessorData receivedPacket;

struct parsedSecondaryProcessorPacket parsedPacket;

extern struct LogPacket sensorPacket;
extern struct LogMessage togLogMessageReceived;
static const struct LogMessage nullMessage = {0};

struct LogMessage commandToSend;

void InterProcessorTask(void *argument){
	uint32_t evt = 0;

	// ensure secondary processor is not active, trying to send data
	// 		note: this should only happen when debugging and resetting the main processor while secondary is logging
	osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
	while(HAL_I2C_Master_Transmit(&hi2c1, SECONDARY_MCU_ADDRESS << 1, (uint8_t *) &nullMessage, sizeof(togLogMessageReceived), 100) != HAL_OK);
	osSemaphoreRelease(messageI2C_LockHandle);

	while(1){

		evt = osThreadFlagsWait (0x00000001U, osFlagsWaitAny, osWaitForever);

		// if signal was received successfully, start blink task
		if ( (evt & 0x00000001U) == 0x00000001U)  {

			// tell secondary processor to start logging (in blocking mode)
			memcpy(&commandToSend, &togLogMessageReceived, sizeof(struct LogMessage));
//			osThreadFlagsClear(0x0000000FU);
			osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
			while(HAL_I2C_Master_Transmit(&hi2c1, SECONDARY_MCU_ADDRESS << 1, (uint8_t *) &commandToSend, sizeof(togLogMessageReceived), 100) != HAL_OK);
			osDelay(100);
			osSemaphoreRelease(messageI2C_LockHandle);
			// message passing until told to stop
			while(1){

				// grab an event flag if available
				evt = osThreadFlagsWait (0x00000006U, osFlagsWaitAny, osWaitForever);

				// if an interrupt is received indicating a message is waiting to be received
 				if( (evt & 0x00000004U) == 0x00000004U){
 					osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
// 					taskENTER_CRITICAL();
 					// clear transmission flag
// 					osThreadFlagsClear(0x00000010U);
 					// send command packet to MCU
 					while(HAL_I2C_Master_Transmit(&hi2c1, SECONDARY_MCU_ADDRESS << 1, (uint8_t *) &commandToSend, sizeof(struct LogMessage), 100) != HAL_OK){
// 						osSemaphoreRelease(messageI2C_LockSem);
// 						osDelay(100);
// 						osSemaphoreAcquire(messageI2C_LockSem, osWaitForever);
 					}
 					// wait until transmission is successful
// 					evt = osThreadFlagsWait(0x00000010U, osFlagsWaitAny, osWaitForever);
 					// ensure I2C is disabled
// 					HAL_I2C_Master_Abort_IT(&hi2c1, SECONDARY_MCU_ADDRESS << 1);

 					// clear receiving flag
 					osThreadFlagsClear(0x00000008U);
 					// grab packet from secondary MCU
 					while(HAL_I2C_Master_Receive_IT(&hi2c1, SECONDARY_MCU_ADDRESS << 1, (uint8_t *) &receivedPacket, sizeof(struct secondaryProcessorData)) != HAL_OK){
// 						osSemaphoreRelease(messageI2C_LockSem);
// 						osDelay(100);
// 						osSemaphoreAcquire(messageI2C_LockSem, osWaitForever);
 					}
// 					taskEXIT_CRITICAL();
					// wait until packet is received
					evt = osThreadFlagsWait(0x0000000AU, osFlagsWaitAny, osWaitForever);
					// ensure I2C is disabled
//					HAL_I2C_Master_Abort_IT(&hi2c1, SECONDARY_MCU_ADDRESS << 1);

					osSemaphoreRelease(messageI2C_LockHandle);


//					evt = osThreadFlagsWait(0x00000002U, osFlagsWaitAny, 0);
					// if thread was told to stop, break from while loop!
					if( (evt & 0x00000002U) == 0x00000002U ) break;

					// package received data into 100ms chunks and put in queue
					parsedPacket.tick_ms = receivedPacket.tick_ms;
					parsedPacket.epoch = receivedPacket.epoch;

					for(int i = 0; i < 5; i++)
					{
						memcpy(&parsedPacket.temple, &receivedPacket.temp.temple[i], sizeof(struct thermopileData));
						memcpy(&parsedPacket.nose, &receivedPacket.temp.nose[i], sizeof(struct thermopileData));

						// pass to master thread to handle
						osMessageQueuePut(interProcessorMsgQueueHandle, (void *) &parsedPacket, 0U, 0);

					}
				}

				// stop thread and clear queues
				if( (evt & 0x00000002U) == 0x00000002U){

					/// clear transmission flag
// 					osThreadFlagsClear(0x00000010U);
					// tell secondary processor to stop logging (in blocking mode)
					osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
					while(HAL_I2C_Master_Transmit(&hi2c1, SECONDARY_MCU_ADDRESS << 1, (uint8_t *) &nullMessage, sizeof(togLogMessageReceived), 100) != HAL_OK);
					osSemaphoreRelease(messageI2C_LockHandle);
					// wait until transmit is complete
//					evt = osThreadFlagsWait(0x00000010U, osFlagsWaitAny, osWaitForever);

					// empty queue
					osMessageQueueReset(interProcessorMsgQueueHandle);

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
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	// notify sending thread that message has been sent
//	osThreadFlagsSet(sendMsgToMainTaskHandle, 0x00000001U);
	osThreadFlagsSet(interProcTaskHandle, 0x00000010U);
	test_1 = 1;
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	// notify receiving thread that a message has been received
	osThreadFlagsSet(interProcTaskHandle, 0x00000008U);
}
