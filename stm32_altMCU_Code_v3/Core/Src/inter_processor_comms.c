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

struct secondaryProcessorData packetReceived;


/* Functions Definition ------------------------------------------------------*/
void SendPacketToMainTask(void *argument){

	//Enable listening of I2C
	HAL_I2C_EnableListen_IT(&hi2c1);

	while(1){

		//try to get packetized data
		if(osMessageQueueGet(sendMsgToMainQueueHandle, &packetReceived, 0U, 60000) == osOK) {

			//if successful, assert interrupt pin to notify master a packet is waiting
			HAL_GPIO_WritePin(EXPANSION_INT_GPIO_Port, EXPANSION_INT_Pin, GPIO_PIN_SET);

			// wait for transmit to succeed
			if (osThreadFlagsWait(0x00000001U, osFlagsWaitAny, 30000) == osFlagsErrorTimeout){
				HAL_GPIO_WritePin(EXPANSION_INT_GPIO_Port, EXPANSION_INT_Pin, GPIO_PIN_RESET);
				HAL_I2C_DisableListen_IT(&hi2c1);
				HAL_I2C_DeInit(&hi2c1);
				osDelay(300);
				HAL_I2C_Init(&hi2c1);
				HAL_I2C_EnableListen_IT(&hi2c1);

			}

		}else{

			//non-standard operation; no events for 60 sec.
			HAL_GPIO_WritePin(EXPANSION_INT_GPIO_Port, EXPANSION_INT_Pin, GPIO_PIN_RESET);
			HAL_I2C_DisableListen_IT(&hi2c1);
			osDelay(300);
			HAL_I2C_EnableListen_IT(&hi2c1);

		}

		HAL_GPIO_WritePin(EXPANSION_INT_GPIO_Port, EXPANSION_INT_Pin, GPIO_PIN_RESET);
		osDelay(10);


	}
}

//This keeps the slave listening once HAL_I2C_EnableListen_IT is called.
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c) {
	HAL_I2C_EnableListen_IT(hi2c);
}

//When master knocks, we will get this after the address is delivered.
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	//Now we initiate our send.
	if (TransferDirection == I2C_DIRECTION_RECEIVE){
		//HAL_I2C_Slave_Receive_IT(hi2c, (uint8_t *) &packetReceived, sizeof(struct secondaryProcessorData));
		HAL_I2C_Slave_Seq_Transmit_IT(hi2c, (uint8_t *) &packetReceived, sizeof(struct secondaryProcessorData), I2C_FIRST_AND_LAST_FRAME);
	}
}

//HAL_I2C_Slave_Receive_IT has succeeded.
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
	osThreadFlagsSet(sendMsgToMainTaskHandle, 0x00000001U);
}

//FOR RX, we can use transferDirection else, call HAL_I2C_Slave_Seq_Receive_IT, and the finish with I2c_SlaveRxCpltCallback with a __HAL_I2C_GENERATE_NACK


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
