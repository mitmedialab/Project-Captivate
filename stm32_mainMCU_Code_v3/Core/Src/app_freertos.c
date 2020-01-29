/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "adc.h"
#include "tim.h"
#include "lp5523.h"
#include "blink.h"
#include "app_entry.h"
#include "master_thread.h"
#include "system_settings.h"
#include "inter_processor_comms.h"
#include "inertial_sensing.h"
#include "input.h"
#include "pulse_processor.h"
#include "messages.h"
#include "captivate_config.h"
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
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512
};
/* Definitions for frontLightsTask */
osThreadId_t frontLightsTaskHandle;
const osThreadAttr_t frontLightsTask_attributes = {
  .name = "frontLightsTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512
};
/* Definitions for masterTask */
osThreadId_t masterTaskHandle;
const osThreadAttr_t masterTask_attributes = {
  .name = "masterTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512
};
/* Definitions for inertialTask */
osThreadId_t inertialTaskHandle;
const osThreadAttr_t inertialTask_attributes = {
  .name = "inertialTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024
};
/* Definitions for pulseTask */
osThreadId_t pulseTaskHandle;
const osThreadAttr_t pulseTask_attributes = {
  .name = "pulseTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512
};
/* Definitions for interProcTask */
osThreadId_t interProcTaskHandle;
const osThreadAttr_t interProcTask_attributes = {
  .name = "interProcTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512
};
/* Definitions for blinkTask */
osThreadId_t blinkTaskHandle;
const osThreadAttr_t blinkTask_attributes = {
  .name = "blinkTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512
};
/* Definitions for blinkMsgQueue */
osMessageQueueId_t blinkMsgQueueHandle;
const osMessageQueueAttr_t blinkMsgQueue_attributes = {
  .name = "blinkMsgQueue"
};
/* Definitions for lightsSimpleQueue */
osMessageQueueId_t lightsSimpleQueueHandle;
const osMessageQueueAttr_t lightsSimpleQueue_attributes = {
  .name = "lightsSimpleQueue"
};
/* Definitions for togLoggingQueue */
osMessageQueueId_t togLoggingQueueHandle;
const osMessageQueueAttr_t togLoggingQueue_attributes = {
  .name = "togLoggingQueue"
};
/* Definitions for interProcessorMsgQueue */
osMessageQueueId_t interProcessorMsgQueueHandle;
const osMessageQueueAttr_t interProcessorMsgQueue_attributes = {
  .name = "interProcessorMsgQueue"
};
/* Definitions for inertialSensingQueue */
osMessageQueueId_t inertialSensingQueueHandle;
const osMessageQueueAttr_t inertialSensingQueue_attributes = {
  .name = "inertialSensingQueue"
};
/* Definitions for activitySampleQueue */
osMessageQueueId_t activitySampleQueueHandle;
const osMessageQueueAttr_t activitySampleQueue_attributes = {
  .name = "activitySampleQueue"
};
/* Definitions for rotationSampleQueue */
osMessageQueueId_t rotationSampleQueueHandle;
const osMessageQueueAttr_t rotationSampleQueue_attributes = {
  .name = "rotationSampleQueue"
};
/* Definitions for pulseQueue */
osMessageQueueId_t pulseQueueHandle;
const osMessageQueueAttr_t pulseQueue_attributes = {
  .name = "pulseQueue"
};
/* Definitions for viveQueue */
osMessageQueueId_t viveQueueHandle;
const osMessageQueueAttr_t viveQueue_attributes = {
  .name = "viveQueue"
};
/* Definitions for messageI2C_Lock */
osSemaphoreId_t messageI2C_LockHandle;
const osSemaphoreAttr_t messageI2C_Lock_attributes = {
  .name = "messageI2C_Lock"
};

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

extern void MX_STM32_WPAN_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
volatile unsigned long ulHighFrequencyTimerTicks;
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return HAL_GetTick();
}
/* USER CODE END 1 */

/* USER CODE BEGIN 3 */
__weak void vApplicationTickHook( void )
{
   /* This function will be called by each tick interrupt if
   configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be
   added here, but the tick hook is called from an interrupt context, so
   code must not attempt to block, and only the interrupt safe FreeRTOS API
   functions can be used (those that end in FromISR()). */
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
volatile uint8_t test=0;
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{

   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
	test += 1;
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
__weak void vApplicationMallocFailedHook(void)
{
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
/* USER CODE END 5 */

/* USER CODE BEGIN VPORT_SUPPORT_TICKS_AND_SLEEP */
__weak void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
  // Generated when configUSE_TICKLESS_IDLE == 2.
  // Function called in tasks.c (in portTASK_FUNCTION).
  // TO BE COMPLETED or TO BE REPLACED by a user one, overriding that weak one.
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

  /* USER CODE BEGIN RTOS_SEMAPHORES */
//  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
//  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of blinkMsgQueue */
  blinkMsgQueueHandle = osMessageQueueNew (10, 108, &blinkMsgQueue_attributes);

  /* creation of lightsSimpleQueue */
  lightsSimpleQueueHandle = osMessageQueueNew (3, 4, &lightsSimpleQueue_attributes);

  /* creation of togLoggingQueue */
  togLoggingQueueHandle = osMessageQueueNew (4, 6, &togLoggingQueue_attributes);

  /* creation of interProcessorMsgQueue */
  interProcessorMsgQueueHandle = osMessageQueueNew (10, 24, &interProcessorMsgQueue_attributes);

  /* creation of inertialSensingQueue */
  inertialSensingQueueHandle = osMessageQueueNew (10, 40, &inertialSensingQueue_attributes);

  /* creation of activitySampleQueue */
  activitySampleQueueHandle = osMessageQueueNew (10, 16, &activitySampleQueue_attributes);

  /* creation of rotationSampleQueue */
  rotationSampleQueueHandle = osMessageQueueNew (3, 24, &rotationSampleQueue_attributes);

  /* creation of pulseQueue */
  pulseQueueHandle = osMessageQueueNew (10, 6, &pulseQueue_attributes);

  /* creation of viveQueue */
  viveQueueHandle = osMessageQueueNew (10, 24, &viveQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(DefaultTask, NULL, &defaultTask_attributes);

  /* creation of frontLightsTask */
  frontLightsTaskHandle = osThreadNew(ThreadFrontLightsTask, NULL, &frontLightsTask_attributes);

  /* creation of masterTask */
  masterTaskHandle = osThreadNew(MasterThreadTask, NULL, &masterTask_attributes);

  /* creation of inertialTask */
  inertialTaskHandle = osThreadNew(InertialSensingTask, NULL, &inertialTask_attributes);

  /* creation of pulseTask */
  pulseTaskHandle = osThreadNew(PulseHandlerTask, NULL, &pulseTask_attributes);

  /* creation of interProcTask */
  interProcTaskHandle = osThreadNew(InterProcessorTask, NULL, &interProcTask_attributes);

  /* creation of blinkTask */
  blinkTaskHandle = osThreadNew(BlinkTask, NULL, &blinkTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */

   /* add threads, ... */


	/* Init code for STM32_WPAN */
	APPE_Init();


  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_DefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_DefaultTask */
__weak void DefaultTask(void *argument)
{
  /* USER CODE BEGIN DefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END DefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void startSensorThreads(void){
	/* creation of messageI2C_Lock */
	messageI2C_LockHandle = osSemaphoreNew(1, 1, &messageI2C_Lock_attributes);

	/* creation of blinkMsgQueue */
	blinkMsgQueueHandle = osMessageQueueNew (10, 108, &blinkMsgQueue_attributes);

	/* creation of lightsSimpleQueue */
	lightsSimpleQueueHandle = osMessageQueueNew (3, 4, &lightsSimpleQueue_attributes);

	/* creation of togLoggingQueue */
	togLoggingQueueHandle = osMessageQueueNew (4, 6, &togLoggingQueue_attributes);

	/* creation of interProcessorMsgQueue */
	interProcessorMsgQueueHandle = osMessageQueueNew (10, 24, &interProcessorMsgQueue_attributes);

	/* creation of inertialSensingQueue */
	inertialSensingQueueHandle = osMessageQueueNew (10, 40, &inertialSensingQueue_attributes);

	/* creation of activitySampleQueue */
	activitySampleQueueHandle = osMessageQueueNew (10, 16, &activitySampleQueue_attributes);

	/* creation of rotationSampleQueue */
	rotationSampleQueueHandle = osMessageQueueNew (3, 24, &rotationSampleQueue_attributes);

	/* creation of pulseQueue */
	pulseQueueHandle = osMessageQueueNew (10, 6, &pulseQueue_attributes);

	/* creation of viveQueue */
	viveQueueHandle = osMessageQueueNew (10, 24, &viveQueue_attributes);

	/* creation of inertialTask */
//	inertialTaskHandle = osThreadNew(InertialSensingTask, NULL, &inertialTask_attributes);
//
//	/* creation of pulseTask */
//	pulseTaskHandle = osThreadNew(PulseHandlerTask, NULL, &pulseTask_attributes);
//
//	/* creation of interProcTask */
//	interProcTaskHandle = osThreadNew(InterProcessorTask, NULL, &interProcTask_attributes);

	/* creation of blinkTask */
//	blinkTaskHandle = osThreadNew(BlinkTask, NULL, &blinkTask_attributes);

//	/* creation of masterTask */
//	masterTaskHandle = osThreadNew(MasterThreadTask, NULL, &masterTask_attributes);

	/* creation of frontLightsTask */
//	frontLightsTaskHandle = osThreadNew(ThreadFrontLightsTask, NULL, &frontLightsTask_attributes);
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
