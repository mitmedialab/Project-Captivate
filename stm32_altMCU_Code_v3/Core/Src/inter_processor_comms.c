///**
// ******************************************************************************
// * File Name           : inter_processor_comms.c
//  * Description        :
//  ******************************************************************************
//
//  *
//  ******************************************************************************
// */
//
//
///* includes -----------------------------------------------------------*/
//#include "stm32f3xx_hal.h"
//#include "main.h"
//#include "inter_processor_comms.h"
//#include "master_thread.h"
///* typedef -----------------------------------------------------------*/
//
//
///* defines -----------------------------------------------------------*/
//
//
///* macros ------------------------------------------------------------*/
//
//
///* function prototypes -----------------------------------------------*/
//
//
///* variables -----------------------------------------------*/
//
//
//struct LogPacket packetReceived;
//
///* Functions Definition ------------------------------------------------------*/
//void SendPacketToMainTask(void *argument){
//
//	// wait for packet
//	osMessageQueueGet(sendMsgToMainQueueHandle, &packetReceived, 0U, osWaitForever);
//
//	// trigger interrupt line to MCU to tell it that a packet is ready
//	HAL_GPIO_WritePin(EXPANSION_INT_GPIO_Port, EXPANSION_INT_Pin, GPIO_PIN_SET);
//
//	// send packet via I2C
//	osDelay(100);
//
//	// reset trigger
//	HAL_GPIO_WritePin(EXPANSION_INT_GPIO_Port, EXPANSION_INT_Pin, GPIO_PIN_RESET);
//
//
//}
//
//
///*************************************************************
// *
// * LOCAL FUNCTIONS
// *
// *************************************************************/
//
///**
// * @brief Thread initialization.
// * @param  None
// * @retval None
// */
//
///*************************************************************
// *
// * FREERTOS WRAPPER FUNCTIONS
// *
//*************************************************************/
