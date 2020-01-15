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
//uint8_t temp[2048] = {0};
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_STM32_WPAN_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

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

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  const osThreadAttr_t threadFrontLightsTask_attributes = {
         .name = "threadFrontLightsTask",
         .priority = (osPriority_t) osPriorityNormal,
         .stack_size = 256
       };
   threadFrontLightsTaskHandle = osThreadNew(ThreadFrontLightsTask, NULL, &threadFrontLightsTask_attributes);

 //#ifndef DONGLE_CODE
   const osThreadAttr_t blinkTask_attributes = {
         .name = "blinkTask",
         .priority = (osPriority_t) osPriorityNormal,
         .stack_size = 256
       };
   blinkTaskHandle = osThreadNew(BlinkTask, NULL, &blinkTask_attributes);

   lightsSimpleQueueHandle = osMessageQueueNew (MAX_LIGHT_SIMPLE_QUEUE_SIZE, sizeof(lightsSimpleMessage), NULL);
 //  lightsSimpleQueueHandle = osMessageQueueNew (2, sizeof(LogMessage), NULL);
   blinkMsgQueueHandle = osMessageQueueNew (10, sizeof(struct blinkData), NULL);
   togLoggingQueueHandle = osMessageQueueNew (2, sizeof(struct LogMessage), NULL);

   const osThreadAttr_t masterThreadTask_attributes = {
           .name = "masterThreadTask",
           .priority = (osPriority_t) osPriorityNormal,
           .stack_size = 256
         };
    masterThreadTaskHandle = osThreadNew(MasterThreadTask, NULL, &masterThreadTask_attributes);

	const osThreadAttr_t inertialSensingTask_attributes = {
			 .name = "inertialSensingTask",
			 .priority = (osPriority_t) osPriorityNormal,
			 .stack_size = 128
		   };
	inertialSensingTaskHandle = osThreadNew(InertialSensingTask, NULL, &inertialSensingTask_attributes);

    const osThreadAttr_t interProcessorTask_attributes = {
               .name = "interProcessorTask",
               .priority = (osPriority_t) osPriorityNormal,
               .stack_size = 128
             };
    interProcessorTaskHandle = osThreadNew(InterProcessorTask, NULL, &interProcessorTask_attributes);

    interProcessorMsgQueueHandle = osMessageQueueNew (15, sizeof(struct parsedSecondaryProcessorPacket), NULL);

    inertialSensingQueueHandle = osMessageQueueNew (3, sizeof(struct inertialData), NULL);
    activitySampleQueueHandle = osMessageQueueNew (10, sizeof(struct activityData), NULL);
    rotationSampleQueueHandle = osMessageQueueNew (3, sizeof(struct rotationData), NULL);
//    stepSampleQueueHandle = osMessageQueueNew (3, sizeof(struct stepData), NULL);
//    stabilitySampleQueueHandle = osMessageQueueNew (3, sizeof(struct stabilityData), NULL);

    messageI2C_LockSem = osSemaphoreNew (1, 1, NULL);


 //#endif

   /* add threads, ... */

 	/* Init code for STM32_WPAN */
   APPE_Init();
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
//  HAL_TIM_Base_Start(&htim2);
//  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
//  HAL_ADC_Start_DMA(&hadc1, (uint32_t *) temp, sizeof(temp));
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
