/**
 ******************************************************************************
 * File Name           : master_thread.c
  * Description        : Master Thread that governs the system.
  ******************************************************************************

  *
  ******************************************************************************
 */


/* includes -----------------------------------------------------------*/
#include "master_thread.h"
//#include "stm32wbxx_hal_i2c.h"
#include "stm32wbxx_hal.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "app_thread.h"
#include "rtc.h"
#include "blink.h"
#include "inertial_sensing.h"
/* typedef -----------------------------------------------------------*/


/* defines -----------------------------------------------------------*/
#define JULIAN_DATE_BASE     2440588   // Unix epoch time in Julian calendar (UnixTime = 00:00:00 01.01.1970 => JDN = 2440588)

/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/


struct blinkData	blinkMsgReceived;
struct parsedSecondaryProcessorPacket secondaryProcessorMsgReceived;
struct inertialData inertialMsgReceived;

//static const uint16_t week_day[] = { 0x4263, 0xA8BD, 0x42BF, 0x4370, 0xABBF, 0xA8BF, 0x43B2 };
/* Functions Definition ------------------------------------------------------*/



/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
 * @brief Master Thread function
 * @param  None
 * @retval None
 */

void MasterThreadTask(void *argument)
{
	while(1)
	{
		// check if the queue has a new message (a command to start/stop logging)
		//   .... this function waits forever
		osMessageQueueGet(togLoggingQueueHandle, &togLogMessageReceived, 0U, osWaitForever);

//		togLogMessageReceived.status = 1;
//		togLogMessageReceived.logStatus = 1;
//		togLogMessageReceived.blinkEnabled = 0;
//		togLogMessageReceived.tempEnabled = 1;
		togLogMessageReceived.intertialEnabled = 0;
//		togLogMessageReceived.positionEnabled = 0;


		// if the received command enables logging
		//    otherwise, skip if statement and wait for an enabling command
		if(togLogMessageReceived.logStatus == ENABLE_LOG)
		{
			// keep record of this message so new message doesn't overwrite
			memcpy(&prevLogMessage, &togLogMessageReceived, sizeof(struct LogMessage));

			// start all sensor subsystems
			if(togLogMessageReceived.blinkEnabled == SENSOR_ENABLE)
			{
				osThreadFlagsSet(blinkTaskHandle, 0x00000001U);
			}
//
//			if(togLogMessageReceived.positionEnabled == SENSOR_ENABLE)
//			{
//				osSemaphoreRelease(posSemaphore);
//			}
//
			if( (togLogMessageReceived.tempEnabled == SENSOR_ENABLE))
			{
				osThreadFlagsSet(interProcessorTaskHandle, 0x00000001U);
			}

			if( (togLogMessageReceived.intertialEnabled == SENSOR_ENABLE))
			{
				osThreadFlagsSet(InertialSensingTask, 0x00000001U);
			}


			while(1)
			{
				/**********************************************************************************/
				/*.... WAIT UNTIL DATA PACKET IS READY.....*/
				/**********************************************************************************/

				if(togLogMessageReceived.blinkEnabled == SENSOR_ENABLE)
				{
					osMessageQueueGet(blinkMsgQueueHandle, &blinkMsgReceived, 0U, osWaitForever);
				}

				//						if(prevLogMessage.positionEnabled == SENSOR_ENABLE)
				//						{
				//
				//						}

				if( (togLogMessageReceived.tempEnabled == SENSOR_ENABLE))
				{
					osMessageQueueGet(interProcessorMsgQueueHandle, &secondaryProcessorMsgReceived, 0U, osWaitForever);
				}

				if( (togLogMessageReceived.intertialEnabled == SENSOR_ENABLE))
				{
					osMessageQueueGet(inertialSensingQueueHandle, &inertialMsgReceived, 0U, osWaitForever);
				}

				packetizeData(&sensorPacket, &blinkMsgReceived, NULL, &secondaryProcessorMsgReceived, &inertialMsgReceived);

				/**********************************************************************************/
				/*.... SEND PACKET TO BORDER ROUTER .....*/
				/**********************************************************************************/

				APP_THREAD_SendBorderPacket(&sensorPacket);

				/**********************************************************************************/
				/*.... CHECK IF NODE HAS BEEN REQUESTED TO STOP .....*/
				/**********************************************************************************/

				// check if the queue has a new message (potentially a command to stop logging)
				//   otherwise, timeout
				if(osMessageQueueGet(togLoggingQueueHandle, &togLogMessageReceived, 0U, 0) == osOK)
				{
					// disable threads
					if(togLogMessageReceived.status == DISABLE_SENSING){

						if(prevLogMessage.blinkEnabled == SENSOR_ENABLE)
						{
							osThreadFlagsSet(blinkTaskHandle, 0x00000002U);
						}
//
//						if(prevLogMessage.positionEnabled == SENSOR_ENABLE)
//						{
//
//						}

						if( (prevLogMessage.tempEnabled == SENSOR_ENABLE))
						{
							osThreadFlagsSet(interProcessorTaskHandle, 0x00000002U);
						}

						if( (togLogMessageReceived.intertialEnabled == SENSOR_ENABLE))
						{
							osThreadFlagsSet(InertialSensingTask, 0x00000002U);
						}

						// break out of first while loop and wait until told to start logging again
						break;
					}
				}
			}
		}

	}
}

RTC_TimeTypeDef RTC_time;
RTC_DateTypeDef RTC_date;

void packetizeData(struct LogPacket *packet,
		struct blinkData *blink,
		struct positionData *pos,
		struct parsedSecondaryProcessorPacket *processorMsg,
		struct inertialData *inertialMsg)
{
	// get processor tick counts (in terms of ms)
	packet->tick_ms = HAL_GetTick();

	// get epoch time from RTC
	HAL_RTC_GetTime(&hrtc, &RTC_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &RTC_date, RTC_FORMAT_BIN);
	packet->epoch = RTC_ToEpoch(&RTC_time, &RTC_date);

	// add sensor data
	memcpy ( &(packet->blink), blink, sizeof(struct blinkData) );
	memcpy ( &(packet->procData), processorMsg, sizeof(struct parsedSecondaryProcessorPacket) );
	memcpy ( &(packet->inertial), inertialMsg, sizeof(struct inertialData) );
}

// Convert Date/Time structures to epoch time
uint32_t RTC_ToEpoch(RTC_TimeTypeDef *time, RTC_DateTypeDef *date) {
	uint8_t  a;
	uint16_t y;
	uint8_t  m;
	uint32_t JDN;

	// These hardcore math's are taken from http://en.wikipedia.org/wiki/Julian_day

	// Calculate some coefficients
	a = (14 - date->Month) / 12;
	y = (date->Year + 2000) + 4800 - a; // years since 1 March, 4801 BC
	m = date->Month + (12 * a) - 3; // since 1 March, 4801 BC

	// Gregorian calendar date compute
    JDN  = date->Date;
    JDN += (153 * m + 2) / 5;
    JDN += 365 * y;
    JDN += y / 4;
    JDN += -y / 100;
    JDN += y / 400;
    JDN  = JDN - 32045;
    JDN  = JDN - JULIAN_DATE_BASE;    // Calculate from base date
    JDN *= 86400;                     // Days to seconds
    JDN += time->Hours * 3600;    // ... and today seconds
    JDN += time->Minutes * 60;
    JDN += time->Seconds;

	return JDN;
}
//
//// Convert epoch time to Date/Time structures
//void RTC_FromEpoch(uint32_t epoch, RTC_TimeTypeDef *time, RTC_DateTypeDef *date) {
//	uint32_t tm;
//	uint32_t t1;
//	uint32_t a;
//	uint32_t b;
//	uint32_t c;
//	uint32_t d;
//	uint32_t e;
//	uint32_t m;
//	int16_t  year  = 0;
//	int16_t  month = 0;
//	int16_t  dow   = 0;
//	int16_t  mday  = 0;
//	int16_t  hour  = 0;
//	int16_t  min   = 0;
//	int16_t  sec   = 0;
//	uint64_t JD    = 0;
//	uint64_t JDN   = 0;
//
//	// These hardcore math's are taken from http://en.wikipedia.org/wiki/Julian_day
//
//	JD  = ((epoch + 43200) / (86400 >>1 )) + (2440587 << 1) + 1;
//	JDN = JD >> 1;
//
//    tm = epoch; t1 = tm / 60; sec  = tm - (t1 * 60);
//    tm = t1;    t1 = tm / 60; min  = tm - (t1 * 60);
//    tm = t1;    t1 = tm / 24; hour = tm - (t1 * 24);
//
//    dow   = JDN % 7;
//    a     = JDN + 32044;
//    b     = ((4 * a) + 3) / 146097;
//    c     = a - ((146097 * b) / 4);
//    d     = ((4 * c) + 3) / 1461;
//    e     = c - ((1461 * d) / 4);
//    m     = ((5 * e) + 2) / 153;
//    mday  = e - (((153 * m) + 2) / 5) + 1;
//    month = m + 3 - (12 * (m / 10));
//    year  = (100 * b) + d - 4800 + (m / 10);
//
//    date->RTC_Year    = year - 2000;
//    date->RTC_Month   = month;
//    date->RTC_Date    = mday;
//    date->RTC_WeekDay = dow;
//    time->RTC_Hours   = hour;
//    time->RTC_Minutes = min;
//    time->RTC_Seconds = sec;
//}

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
*************************************************************/
