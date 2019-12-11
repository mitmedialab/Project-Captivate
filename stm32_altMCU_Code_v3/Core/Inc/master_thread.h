///**
// ******************************************************************************
// * File Name          : master_thread.h
//  * Description        : the master thread that governs the system.
//  ******************************************************************************
//
//  *
//  ******************************************************************************
// */
//#ifndef MASTER_THREAD_H
//#define MASTER_THREAD_H
//
//#ifdef __cplusplus
//extern "C" {
//#endif
//
///* includes -----------------------------------------------------------*/
//#include "stdint.h"
//#include "cmsis_os.h"
//#include "config.h"
////#include "blink.h"
//#include "stm32f3xx_hal.h"
//#include "thermopile.h"
////#include "position.h"
////#include "inter_processor_comms.h"
///* typedef -----------------------------------------------------------*/
//
//struct LogPacket
//{
//	struct tempData			temp;
////	tempData				temp;
////	inertialData			inertial;
////	positionData			pos;
//	uint32_t				tick_ms;
//	uint32_t				epoch;
//};
//
//struct LogMessage
//{
//	uint8_t		status;
//	uint8_t		logStatus;
//	uint8_t		blinkEnabled;
//	uint8_t		tempEnabled;
//	uint8_t		intertialEnabled;
//	uint8_t		positionEnabled;
//
//};
///* defines -----------------------------------------------------------*/
//
//
///* macros ------------------------------------------------------------*/
//
//
///* function prototypes -----------------------------------------------*/
//void packetizeData(struct LogPacket *packet,
//		struct thermopileData *temp,
//		struct inertialData *imu);
//
//void MasterThreadTask(void *argument);
//
//
//uint32_t RTC_ToEpoch(RTC_TimeTypeDef *time, RTC_DateTypeDef *date);
//
///* variables -----------------------------------------------*/
////osThreadId_t 		threadFrontLightsTaskHandle;
////osMessageQueueId_t	lightsSimpleQueueHandle;
//osMessageQueueId_t	togLoggingQueueHandle;
//osThreadId_t masterThreadTaskHandle;
//
//
//
///* Functions Definition ------------------------------------------------------*/
//
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
//
//
//
//
//
//#ifdef __cplusplus
//} /* extern "C" */
//#endif
//
//#endif
