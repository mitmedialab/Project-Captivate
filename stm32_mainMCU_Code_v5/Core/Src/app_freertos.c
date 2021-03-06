/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : app_freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_entry.h"
#include "master_thread.h"
#include "app_freertos.h"
#include "adc.h"
#include "tim.h"
#include "lp5523.h"
#include "blink.h"
#include "system_settings.h"
#include "inter_processor_comms.h"
#include "inertial_sensing.h"
#include "input.h"
#include "pulse_processor.h"
#include "messages.h"
#include "captivate_config.h"
#include "input.h"
//#include "iwdg.h"
#include "usbd_cdc_if.h"
#include "coap.h"
#include "app_thread.h"

#ifdef NETWORK_TEST
#include "network_test.h"
#endif
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
#ifdef NETWORK_TEST
osThreadId_t networkTestTaskHandle;
const osThreadAttr_t networkTestTask_attributes = { .name = "networkTestTask",
		.priority = (osPriority_t) osPriorityLow, .stack_size = 512 * 2 };
#endif
//osThreadId_t blinkTaskHandle;
////osMessageQueueId_t	blinkMsgQueueHandle;
//
//osThreadId_t 		frontLightsTaskHandle;
//osMessageQueueId_t	lightsSimpleQueueHandle;
//
//osMessageQueueId_t	togLoggingQueueHandle;
//osThreadId_t masterTaskHandle;
//
//osSemaphoreId_t messageI2C_LockSem;
//
//osThreadId_t inertialTaskHandle;
//osMessageQueueId_t inertialSensingQueueHandle;
//
//osMessageQueueId_t activitySampleQueueHandle;
//osMessageQueueId_t rotationSampleQueueHandle;
//
//osThreadId_t interProcTaskHandle;
//osMessageQueueId_t	 interProcessorMsgQueueHandle;
//
//osThreadId_t pulseTaskHandle;

//uint8_t temp[2048] = {0};

/* Definitions for rotationSampleQueue */
//#ifdef INERTIAL_ACC_GYRO_EN
osMessageQueueId_t accSampleQueueHandle;
const osMessageQueueAttr_t accSampleQueue_attributes = { .name =
		"accSampleQueue"};

osMessageQueueId_t gyroSampleQueueHandle;
const osMessageQueueAttr_t gyroSampleQueue_attributes = { .name =
		"gyroSampleQueue"};
//#endif

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = { .name = "defaultTask",
		.priority = (osPriority_t) osPriorityNormal, .stack_size = 512 };
/* Definitions for frontLightsTask */
osThreadId_t frontLightsTaskHandle;
const osThreadAttr_t frontLightsTask_attributes = { .name = "frontLightsTask",
		.priority = (osPriority_t) osPriorityLow, .stack_size = 512 * 2};
/* Definitions for frontLightsComplexTask */
osThreadId_t frontLightsComplexTaskHandle;
const osThreadAttr_t frontLightsComplexTask_attributes = { .name = "frontLightsComplexTask",
		.priority = (osPriority_t) osPriorityLow, .stack_size = 512 * 2 };
/* Definitions for masterTask */
osThreadId_t masterTaskHandle;
const osThreadAttr_t masterTask_attributes = { .name = "masterTask", .priority =
		(osPriority_t) osPriorityNormal, .stack_size = 512 * 3 };
/* Definitions for inertialTask */
osThreadId_t inertialTaskHandle;
const osThreadAttr_t inertialTask_attributes = { .name = "inertialTask",
		.priority = (osPriority_t) osPriorityNormal, .stack_size = 512*3 };
/* Definitions for pulseTask */
osThreadId_t pulseTaskHandle;
const osThreadAttr_t pulseTask_attributes = { .name = "pulseTask", .priority =
		(osPriority_t) osPriorityNormal, .stack_size = 512 };
/* Definitions for interProcTask */
osThreadId_t interProcTaskHandle;
const osThreadAttr_t interProcTask_attributes = { .name = "interProcTask",
		.priority = (osPriority_t) osPriorityNormal, .stack_size = 512 };
/* Definitions for blinkTask */
osThreadId_t blinkTaskHandle;
const osThreadAttr_t blinkTask_attributes = { .name = "blinkTask", .priority =
		(osPriority_t) osPriorityNormal, .stack_size = 512 * 4 };
/* Definitions for msgPassingUSB_T */
osThreadId_t msgPassingUSB_THandle;
const osThreadAttr_t msgPassingUSB_T_attributes = { .name = "msgPassingUSB_T",
		.priority = (osPriority_t) osPriorityNormal, .stack_size = 512 };
/* Definitions for blinkMsgQueue */
osMessageQueueId_t blinkMsgQueueHandle;
const osMessageQueueAttr_t blinkMsgQueue_attributes =
		{ .name = "blinkMsgQueue" };
/* Definitions for lightsSimpleQueue */
osMessageQueueId_t lightsSimpleQueueHandle;
const osMessageQueueAttr_t lightsSimpleQueue_attributes = { .name =
		"lightsSimpleQueue" };
/* Definitions for lightsComplexQueue */
osMessageQueueId_t lightsComplexQueueHandle;
const osMessageQueueAttr_t lightsComplexQueue_attributes = { .name =
		"lightsComplexQueue" };
/* Definitions for togLoggingQueue */
osMessageQueueId_t togLoggingQueueHandle;
const osMessageQueueAttr_t togLoggingQueue_attributes = { .name =
		"togLoggingQueue" };
/* Definitions for interProcessorMsgQueue */
osMessageQueueId_t interProcessorMsgQueueHandle;
const osMessageQueueAttr_t interProcessorMsgQueue_attributes = { .name =
		"interProcessorMsgQueue" };
/* Definitions for inertialSensingQueue */
osMessageQueueId_t inertialSensingQueueHandle;
const osMessageQueueAttr_t inertialSensingQueue_attributes = { .name =
		"inertialSensingQueue" };
/* Definitions for activitySampleQueue */
osMessageQueueId_t activitySampleQueueHandle;
const osMessageQueueAttr_t activitySampleQueue_attributes = { .name =
		"activitySampleQueue" };
/* Definitions for rotationSampleQueue */
osMessageQueueId_t rotationSampleQueueHandle;
const osMessageQueueAttr_t rotationSampleQueue_attributes = { .name =
		"rotationSampleQueue" };
/* Definitions for pulseQueue */
osMessageQueueId_t pulseQueueHandle;
const osMessageQueueAttr_t pulseQueue_attributes = { .name = "pulseQueue" };
/* Definitions for viveQueue */
osMessageQueueId_t viveQueueHandle;
const osMessageQueueAttr_t viveQueue_attributes = { .name = "viveQueue" };
/* Definitions for statusQueue */
osMessageQueueId_t statusQueueHandle;
const osMessageQueueAttr_t statusQueue_attributes = { .name = "statusQueue" };
/* Definitions for msgPasssingUSB_Queue */
osMessageQueueId_t msgPasssingUSB_QueueHandle;
const osMessageQueueAttr_t msgPasssingUSB_Queue_attributes = { .name =
		"msgPasssingUSB_Queue" };
/* Definitions for viveTimer */
osTimerId_t viveTimerHandle;
const osTimerAttr_t viveTimer_attributes = { .name = "viveTimer" };
/* Definitions for watchDogTimer */
osTimerId_t watchDogTimerHandle;
const osTimerAttr_t watchDogTimer_attributes = { .name = "watchDogTimer" };
/* Definitions for messageI2C_Lock */
osSemaphoreId_t messageI2C_LockHandle;
const osSemaphoreAttr_t messageI2C_Lock_attributes =
		{ .name = "messageI2C_Lock" };
/* Definitions for locNotify */
osSemaphoreId_t locNotifyHandle;
const osSemaphoreAttr_t locNotify_attributes = { .name = "locNotify" };
/* Definitions for locComplete */
osSemaphoreId_t locCompleteHandle;
const osSemaphoreAttr_t locComplete_attributes = { .name = "locComplete" };
/* Definitions for lightingLabDemoEnd */
osSemaphoreId_t lightingLabDemoEndHandle;
const osSemaphoreAttr_t lightingLabDemoEnd_attributes = { .name =
		"lightingLabDemoEnd" };

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void DefaultTask(void *argument);
extern void ThreadFrontLightsTask(void *argument);
extern void MasterThreadTask(void *argument);
extern void InertialSensingTask(void *argument);
extern void PulseHandlerTask(void *argument);
extern void InterProcessorTask(void *argument);
extern void BlinkTask(void *argument);
void msgPassingUSB(void *argument);
extern void get3D_location(void *argument);
void watchDogReset(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);
/* USER CODE BEGIN VPORT_SUPPORT_TICKS_AND_SLEEP */
__weak void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime) {
	// Generated when configUSE_TICKLESS_IDLE == 2.
	// Function called in tasks.c (in portTASK_FUNCTION).
	// TO BE COMPLETED or TO BE REPLACED by a user one, overriding that weak one.
}

/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
volatile unsigned long ulHighFrequencyTimerTicks;
__weak void configureTimerForRunTimeStats(void) {

}

__weak unsigned long getRunTimeCounterValue(void) {
	return HAL_GetTick();
}

volatile uint8_t test = 0;
__weak void vApplicationStackOverflowHook(xTaskHandle xTask,
		signed char *pcTaskName) {

	/* Run time stack overflow checking is performed if
	 configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
	 called if a stack overflow is detected. */
	test += 1;
}

__weak void vApplicationMallocFailedHook(void) {
	/* vApplicationMallocFailedHook() will only be called if
	 configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
	 function that will get called if a call to pvPortMalloc() fails.
	 pvPortMalloc() is called internally by the kernel whenever a task, queue,
	 timer or semaphore is created. It is also called by various parts of the
	 demo application. If heap_1.c or heap_2.c are used, then the size of the
	 heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	 FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	 to query the size of free heap space that remains (although it does not
	 provide information on how the remaining heap might be fragmented). */
	test += 1;

}
/* USER CODE END VPORT_SUPPORT_TICKS_AND_SLEEP */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */

	/* USER CODE END RTOS_MUTEX */

	/* Create the semaphores(s) */
	/* creation of messageI2C_Lock */
	messageI2C_LockHandle = osSemaphoreNew(1, 1, &messageI2C_Lock_attributes);

	/* creation of locNotify */
	locNotifyHandle = osSemaphoreNew(1, 1, &locNotify_attributes);

	/* creation of locComplete */
	locCompleteHandle = osSemaphoreNew(1, 1, &locComplete_attributes);

	/* creation of lightingLabDemoEnd */
	lightingLabDemoEndHandle = osSemaphoreNew(1, 1,
			&lightingLabDemoEnd_attributes);

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* Create the timer(s) */
	/* creation of viveTimer */
	viveTimerHandle = osTimerNew(get3D_location, osTimerPeriodic,
			(void*) &viveStateVar, &viveTimer_attributes);

	/* creation of watchDogTimer */
	watchDogTimerHandle = osTimerNew(watchDogReset, osTimerPeriodic, NULL,
			&watchDogTimer_attributes);

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the queue(s) */
	/* creation of blinkMsgQueue */
	blinkMsgQueueHandle = osMessageQueueNew(10, 108, &blinkMsgQueue_attributes);

	/* creation of lightsSimpleQueue */
	lightsSimpleQueueHandle = osMessageQueueNew(3, 4,
			&lightsSimpleQueue_attributes);

	/* creation of lightsSimpleQueue */
	lightsComplexQueueHandle = osMessageQueueNew(3, sizeof(union ColorComplex),
			&lightsComplexQueue_attributes);

	/* creation of togLoggingQueue */
	togLoggingQueueHandle = osMessageQueueNew(4, 6,
			&togLoggingQueue_attributes);

	/* creation of interProcessorMsgQueue */
	interProcessorMsgQueueHandle = osMessageQueueNew(10, 24,
			&interProcessorMsgQueue_attributes);

	/* creation of inertialSensingQueue */
	inertialSensingQueueHandle = osMessageQueueNew(10, 40,
			&inertialSensingQueue_attributes);

	/* creation of activitySampleQueue */
	activitySampleQueueHandle = osMessageQueueNew(10, 16,
			&activitySampleQueue_attributes);

	/* creation of rotationSampleQueue */
	rotationSampleQueueHandle = osMessageQueueNew(3, 24,
			&rotationSampleQueue_attributes);

	/* creation of pulseQueue */
	pulseQueueHandle = osMessageQueueNew(10, 6, &pulseQueue_attributes);

	/* creation of viveQueue */
	viveQueueHandle = osMessageQueueNew(10, 24, &viveQueue_attributes);

	/* creation of statusQueue */
	statusQueueHandle = osMessageQueueNew(1, sizeof(uint32_t),
			&statusQueue_attributes);

	/* creation of msgPasssingUSB_Queue */
	msgPasssingUSB_QueueHandle = osMessageQueueNew(5, 11,
			&msgPasssingUSB_Queue_attributes);

	/* USER CODE BEGIN RTOS_QUEUES */
//#ifdef INERTIAL_ACC_GYRO_EN
	accSampleQueueHandle = osMessageQueueNew(ACC_GYRO_QUEUE_SIZE, sizeof(struct genericThreeAxisData *),
			&accSampleQueue_attributes);
	gyroSampleQueueHandle = osMessageQueueNew(ACC_GYRO_QUEUE_SIZE, sizeof(struct genericThreeAxisData *),
			&gyroSampleQueue_attributes);
//#endif
	/* add queues, ... */
//	/* creation of rotationSampleQueue */
//	accSampleQueueHandle = osMessageQueueNew(2, sizeof(struct genericThreeAxisData)*ACC_GYRO_PACKET_SIZE,
//			&accSampleQueue_attributes);
//	/* creation of rotationSampleQueue */
//	gyroSampleQueueHandle = osMessageQueueNew(2, sizeof(struct genericThreeAxisData)*ACC_GYRO_PACKET_SIZE,
//			&gyroSampleQueue_attributes);

	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
//  defaultTaskHandle = osThreadNew(DefaultTask, NULL, &defaultTask_attributes);
//	/* creation of frontLightsTask */
//	frontLightsTaskHandle = osThreadNew(ThreadFrontLightsTask, NULL,
//			&frontLightsTask_attributes);

//	frontLightsComplexTaskHandle = osThreadNew(ThreadFrontLightsComplexTask, NULL,
//				&frontLightsComplexTask_attributes);

//	/* creation of masterTask */
//	masterTaskHandle = osThreadNew(MasterThreadTask, NULL,
//			&masterTask_attributes);
//
//	/* creation of inertialTask */
//	inertialTaskHandle = osThreadNew(InertialSensingTask, NULL,
//			&inertialTask_attributes);
//
//	/* creation of pulseTask */
//	pulseTaskHandle = osThreadNew(PulseHandlerTask, NULL,
//			&pulseTask_attributes);
//
//	/* creation of interProcTask */
//	interProcTaskHandle = osThreadNew(InterProcessorTask, NULL,
//			&interProcTask_attributes);
//
//	/* creation of blinkTask */
//	blinkTaskHandle = osThreadNew(BlinkTask, NULL, &blinkTask_attributes);

//  /* creation of msgPassingUSB_T */
//  msgPassingUSB_THandle = osThreadNew(msgPassingUSB, NULL, &msgPassingUSB_T_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_DefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_DefaultTask */
__weak void DefaultTask(void *argument) {
	/* USER CODE BEGIN DefaultTask */
	/* Infinite loop */
	for (;;) {
		osDelay(1);
	}
	/* USER CODE END DefaultTask */
}

/* USER CODE BEGIN Header_msgPassingUSB */
/**
 * @brief Function implementing the msgPassingUSB_T thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_msgPassingUSB */
__weak void msgPassingUSB(void *argument) {
	/* USER CODE BEGIN msgPassingUSB */
	/* Infinite loop */
	for (;;) {
		osDelay(1);
	}
	/* USER CODE END msgPassingUSB */
}

/* watchDogReset function */
void watchDogReset(void *argument) {
	/* USER CODE BEGIN watchDogReset */

	/* USER CODE END watchDogReset */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void startApplicationThreads(void){
	/* creation of masterTask */
	masterTaskHandle = osThreadNew(MasterThreadTask, NULL,
			&masterTask_attributes);

	/* creation of frontLightsTask */
	frontLightsComplexTaskHandle = osThreadNew(ThreadFrontLightsComplexTask, NULL,
				&frontLightsComplexTask_attributes);

	/* creation of inertialTask */
#ifndef INERTIAL_ACC_GYRO_EN
	inertialTaskHandle = osThreadNew(InertialSensingTask, NULL,
			&inertialTask_attributes);

	pulseTaskHandle = osThreadNew(PulseHandlerTask, NULL,
			&pulseTask_attributes);

	/* creation of interProcTask */
	interProcTaskHandle = osThreadNew(InterProcessorTask, NULL,
			&interProcTask_attributes);

	/* creation of blinkTask */
	blinkTaskHandle = osThreadNew(BlinkTask, NULL, &blinkTask_attributes);
#else
	inertialTaskHandle = osThreadNew(InertialSensingTask_Accel_Gyro, NULL,
			&inertialTask_attributes);

#endif
	/* creation of pulseTask */


#ifdef NETWORK_TEST
	networkTestTaskHandle = osThreadNew(NetworkTestTask, NULL, &networkTestTask_attributes);
#endif

}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
