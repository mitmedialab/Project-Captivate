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
#include "stm32wbxx_hal.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "app_thread.h"
#include "rtc.h"
#include "blink.h"
#include "inertial_sensing.h"
#include "input.h"
#include "app_entry.h"
#include "task.h"
#include "captivate_config.h"
/* typedef -----------------------------------------------------------*/

/* defines -----------------------------------------------------------*/

/* macros ------------------------------------------------------------*/

/* function prototypes -----------------------------------------------*/

/* variables -----------------------------------------------*/

struct blinkData blinkMsgReceived;
struct parsedSecondaryProcessorPacket secondaryProcessorMsgReceived;
struct inertialData inertialMsgReceived;
struct VIVEVars vive_loc;
struct LogMessage statusMessage;

static const struct LogMessage nullStatusMessage = { 0 };
static const struct blinkData nullBlinkMsg = { 0 };
static const struct parsedSecondaryProcessorPacket nullSecondaryProcessorMsgReceived = { 0 };
static const struct inertialData nullInertialMsgReceived = { 0 };
static const struct VIVEVars nullViveMsgReceived = { 0 };

// variables for storing epoch time
RTC_TimeTypeDef RTC_time;
RTC_DateTypeDef RTC_date;

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

uint8_t logEnabled = 0;
uint8_t lightLabDemoEnabled = 0;
//struct SystemStatus systemStatus = { 0 };
uint32_t startTime = 0;

void MasterThreadTask(void *argument) {

#ifndef DONGLE_CODE
	touchSensingStart();
#endif

	while (1) {
		// check if the queue has a new message (a command to start/stop logging)
		//   .... this function waits forever
		osMessageQueueGet(togLoggingQueueHandle, &togLogMessageReceived, 0U,
		osWaitForever);

		// this below togLogMessageReceived manipulation is for debugging
//		togLogMessageReceived.status = 1;
//		togLogMessageReceived.logStatus = 1;
#ifndef BLINK_SENSING_ENABLE
		togLogMessageReceived.blinkEnabled = 0;
#endif

#ifndef TEMP_SENSING_ENABLE
		togLogMessageReceived.tempEnabled = 0;
#endif

#ifndef POS_SENSING_ENABLE
		togLogMessageReceived.positionEnabled = 0;
#endif

#ifndef INERTIA_SENSING_ENABLE
		togLogMessageReceived.intertialEnabled = 0;
#endif



		// pass variable to share system state
		osMessageQueueReset(statusQueueHandle);
		osMessageQueuePut(statusQueueHandle, (void*) &nullStatusMessage, 0U, 0);

		// if the received command enables logging
		//    otherwise, skip if statement and wait for an enabling command
		if (togLogMessageReceived.logStatus == ENABLE_LOG) {
			logEnabled = 1;

			// keep record of this message so new message doesn't overwrite
			memcpy(&prevLogMessage, &togLogMessageReceived, sizeof(struct LogMessage));

			// start all sensor subsystems
			masterEnterRoutine();

			// add a delay to ensure all threads are given enough time to collect initial samples
			osDelay(500);

			while (1) {
				startTime = HAL_GetTick();

				/**********************************************************************************/
				/*.... WAIT UNTIL DATA PACKET IS READY.....*/
				/**********************************************************************************/

				// grab data from sensor thread queues
				grabSensorData();

				// add all sensor data into a packet
				packetizeData(&sensorPacket, &blinkMsgReceived, &secondaryProcessorMsgReceived, &inertialMsgReceived,
						&vive_loc);

				/**********************************************************************************/
				/*.... SEND PACKET TO BORDER ROUTER .....*/
				/**********************************************************************************/

				APP_THREAD_SendBorderPacket(&sensorPacket);

				/**********************************************************************************/
				/*.... CHECK IF NODE HAS BEEN REQUESTED TO STOP .....*/
				/**********************************************************************************/

				// check if the queue has a new message (potentially a command to stop logging)
				//   otherwise, timeout
				if (osMessageQueueGet(togLoggingQueueHandle, &togLogMessageReceived, 0U, 0) == osOK) {
					// disable threads
					if (togLogMessageReceived.status == DISABLE_SENSING) {

						masterExitRoutine();

						// break out of first while loop and wait until told to start logging again
						break;
					}
				}

				// add delay to wait for next transmission period
				osDelay(PACKET_SEND_PERIOD - (HAL_GetTick() - startTime));

			}
		}
		else if (logEnabled == 1 && togLogMessageReceived.logStatus == DISABLE_LOG) {
			logEnabled = 0;
			masterExitRoutine();
		}
		else if (togLogMessageReceived.status == LIGHT_LAB_DEMO){
			// if requesting another feature to be enabled but the logging is still enabled
			if(logEnabled == 1){
				logEnabled = 0;
				masterExitRoutine();
			}

			lightLabDemoEnabled = 1;


		}
		else if (lightLabDemoEnabled = 1 && togLogMessageReceived.status == DISABLE_LOG){



			lightLabDemoEnabled = 0;


		}

	}
}

void grabSensorData(void) {
	if (prevLogMessage.blinkEnabled == SENSOR_ENABLE) {
		if (osOK != osMessageQueueGet(blinkMsgQueueHandle, &blinkMsgReceived, 0U, 0)) {
			memcpy(&blinkMsgReceived, &nullBlinkMsg, sizeof(struct blinkData));
		}
	}

	if ((prevLogMessage.tempEnabled == SENSOR_ENABLE)) {
		if (osOK != osMessageQueueGet(interProcessorMsgQueueHandle, &secondaryProcessorMsgReceived, 0U, 0)) {
			memcpy(&secondaryProcessorMsgReceived, &nullSecondaryProcessorMsgReceived,
					sizeof(struct parsedSecondaryProcessorPacket));
		}
	}

	if ((prevLogMessage.positionEnabled == SENSOR_ENABLE)) {
		if (osOK != osMessageQueueGet(viveQueueHandle, &vive_loc, 0U, 0)) {
			memcpy(&vive_loc, &nullViveMsgReceived, sizeof(VIVEVars));
		}
	}

	if ((prevLogMessage.intertialEnabled == SENSOR_ENABLE)) {
		if (osOK != osMessageQueueGet(inertialSensingQueueHandle, &inertialMsgReceived, 0U, 0)) {
			memcpy(&inertialMsgReceived, &nullInertialMsgReceived, sizeof(struct inertialData));
		}
	}
}

void masterEnterRoutine(void) {
	if (prevLogMessage.blinkEnabled == SENSOR_ENABLE) {
		osThreadFlagsSet(blinkTaskHandle, 0x00000001U);
	}

	if (togLogMessageReceived.positionEnabled == SENSOR_ENABLE) {
		// update status queue to notify other threads position is active
		osMessageQueueGet(statusQueueHandle, &statusMessage, 0U, osWaitForever);
		statusMessage.positionEnabled = 1;
		osMessageQueuePut(statusQueueHandle, (void*) &statusMessage, 0U, 0);

		// start timer for 3D position sample to be taken
		osTimerStart(viveTimerHandle, VIVE_SAMPLE_PERIOD);
	}

	if ((prevLogMessage.tempEnabled == SENSOR_ENABLE)) {
		osThreadFlagsSet(interProcTaskHandle, 0x00000001U);
	}

	if ((prevLogMessage.intertialEnabled == SENSOR_ENABLE)) {
		osThreadFlagsSet(inertialTaskHandle, 0x00000001U);
	}
}

void masterExitRoutine(void) {
	if (prevLogMessage.blinkEnabled == SENSOR_ENABLE) {
		osThreadFlagsSet(blinkTaskHandle, 0x00000002U);
	}

	if (prevLogMessage.positionEnabled == SENSOR_ENABLE) {
		// stop timer for 3D position sensing
		osTimerStop(viveTimerHandle);
	}

	if ((prevLogMessage.tempEnabled == SENSOR_ENABLE)) {
		osThreadFlagsSet(interProcTaskHandle, 0x00000002U);
	}

	if ((prevLogMessage.intertialEnabled == SENSOR_ENABLE)) {
		osThreadFlagsSet(inertialTaskHandle, 0x00000002U);
	}

}



void packetizeData(struct LogPacket *packet, struct blinkData *blink,
		struct parsedSecondaryProcessorPacket *processorMsg, struct inertialData *inertialMsg, VIVEVars *posMsg) {
	// get processor tick counts (in terms of ms)
	packet->tick_ms = HAL_GetTick();

	// get epoch time from RTC
	HAL_RTC_GetTime(&hrtc, &RTC_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &RTC_date, RTC_FORMAT_BIN);
	packet->epoch = RTC_ToEpoch(&RTC_time, &RTC_date);

	// add sensor data
	memcpy(&(packet->blink), blink, sizeof(struct blinkData));
	memcpy(&(packet->procData), processorMsg, sizeof(struct parsedSecondaryProcessorPacket));
	memcpy(&(packet->inertial), inertialMsg, sizeof(struct inertialData));
	memcpy(&(packet->pos), posMsg, sizeof(struct VIVEVars));
}

// Convert Date/Time structures to epoch time
//uint32_t RTC_ToEpoch(RTC_TimeTypeDef *time, RTC_DateTypeDef *date) {
//	uint8_t a;
//	uint16_t y;
//	uint8_t m;
//	uint32_t JDN;
//
//	// These hardcore math's are taken from http://en.wikipedia.org/wiki/Julian_day
//
//	// Calculate some coefficients
//	a = (14 - date->Month) / 12;
//	y = (date->Year + 2000) + 4800 - a; // years since 1 March, 4801 BC
//	m = date->Month + (12 * a) - 3; // since 1 March, 4801 BC
//
//	// Gregorian calendar date compute
//	JDN = date->Date;
//	JDN += (153 * m + 2) / 5;
//	JDN += 365 * y;
//	JDN += y / 4;
//	JDN += -y / 100;
//	JDN += y / 400;
//	JDN = JDN - 32045;
//	JDN = JDN - JULIAN_DATE_BASE;    // Calculate from base date
//	JDN *= 86400;                     // Days to seconds
//	JDN += time->Hours * 3600;    // ... and today seconds
//	JDN += time->Minutes * 60;
//	JDN += time->Seconds;
//
//	return JDN;
//}
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
