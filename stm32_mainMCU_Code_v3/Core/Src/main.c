/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "comp.h"
#include "dma.h"
#include "i2c.h"
#include "rf.h"
#include "rtc.h"
#include "app_entry.h"
#include "app_common.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "input.h"
#include "pulse_processor.h"
#include "circular_buffer.h"
#include "tsc.h"
#include "touch_detector.h"
#include "touchsensing.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TSCx_TS1_MINTHRESHOLD			0
#define TSCx_TS1_MAXTHRESHOLD			3100
#define TSCx_TS2_MINTHRESHOLD			0
#define TSCx_TS2_MAXTHRESHOLD			6150
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t timestamp = 0;

uint8_t cap_sensor = 0;
TSC_IOConfigTypeDef IoConfig;
Debouncer dbs[2] = {{0,0,0}, {0,0,0}};
volatile TouchDetector touch_detector;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_RTC_Init();
  MX_TIM2_Init();
  MX_RF_Init();
  MX_I2C1_Init();
  MX_COMP1_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */
  MX_TSC_Init();

//  IoConfig.ChannelIOs  = TSC_GROUP2_IO3; /* Start with the first channel */
//  IoConfig.SamplingIOs = TSC_GROUP2_IO1;
//  IoConfig.ShieldIOs = 0;

//  if (HAL_TSC_IOConfig(&htsc, &IoConfig) != HAL_OK)
//  {
//    /* Initialization Error */
//    Error_Handler();
//  }
//
//  HAL_TSC_IODischarge(&htsc, ENABLE);
//  HAL_Delay(5); /* 1 ms is more than enough to discharge all capacitors */
//  if (HAL_TSC_Start_IT(&htsc) != HAL_OK)
//	{
//		/* Acquisition Error */
//		Error_Handler();
//	}

  /* USER CODE END 2 */
  /* Init scheduler */
  osKernelInitialize();

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Macro to configure the PLL multiplication factor 
  */
  __HAL_RCC_PLL_PLLM_CONFIG(RCC_PLLM_DIV2);
  /** Macro to configure the PLL clock source 
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
  /** Configure LSE Drive Capability 
  */
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Configure the main internal regulator output voltage 
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI1
                              |RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks 
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS|RCC_PERIPHCLK_RFWAKEUP
                              |RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLLSAI1.PLLN = 6;
  PeriphClkInitStruct.PLLSAI1.PLLP = RCC_PLLP_DIV2;
  PeriphClkInitStruct.PLLSAI1.PLLQ = RCC_PLLQ_DIV2;
  PeriphClkInitStruct.PLLSAI1.PLLR = RCC_PLLR_DIV2;
  PeriphClkInitStruct.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADCCLK;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInitStruct.RFWakeUpClockSelection = RCC_RFWKPCLKSOURCE_LSI;
  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSE;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE0;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
}

/* USER CODE BEGIN 4 */
void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp){
	timestamp = __HAL_TIM_GET_COUNTER(&htim16);

//	GPIO_PinState pin_state = HAL_GPIO_ReadPin(LH_SIG_GPIO_Port, LH_SIG_Pin);
	uint32_t pin_state = HAL_COMP_GetOutputLevel(hcomp);
	if (pin_state == COMP_OUTPUT_LEVEL_HIGH) {
		//Rising edge
		input0.rise_time_ = timestamp;
		input0.rise_valid_ = 1;
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	}
	else if (input0.rise_valid_ && pin_state == COMP_OUTPUT_LEVEL_LOW) {
		//Falling edge
		enqueue_pulse(&input0, input0.rise_time_, timestamp - input0.rise_time_);
		input0.rise_valid_ = 0;
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	}
}

void HAL_TSC_ConvCpltCallback(TSC_HandleTypeDef* htsc){
	  /*##-5- Discharge the touch-sensing IOs ####################################*/
	  HAL_TSC_IODischarge(htsc, ENABLE);
	  /* Note: a delay can be added here */

	  uint32_t uhTSCAcquisitionValue;
	  TSC_GroupStatusTypeDef status = HAL_TSC_GroupGetStatus(htsc, TSC_GROUP2_IDX);
	  uint32_t cur_time;
	  /*##-6- Check if the acquisition is correct (no max count) #################*/
	  if (status == TSC_GROUP_COMPLETED)
	  {
	    /*##-7- Read the acquisition value #######################################*/
	    uhTSCAcquisitionValue = HAL_TSC_GroupGetValue(htsc, TSC_GROUP2_IDX);
	    uint8_t touch;
	    if(cap_sensor == 0){
	    	touch = (uhTSCAcquisitionValue >= TSCx_TS1_MINTHRESHOLD) && (uhTSCAcquisitionValue <= TSCx_TS1_MAXTHRESHOLD);
	    }
	    else{
	    	touch = (uhTSCAcquisitionValue >= TSCx_TS2_MINTHRESHOLD) && (uhTSCAcquisitionValue <= TSCx_TS2_MAXTHRESHOLD);
	    }

	    cur_time = HAL_GetTick();
	    debounce(&dbs[cap_sensor], touch, cur_time);
	    process_touches(&touch_detector, dbs, cur_time);
	  }

	  //Switches between the two channels to be acquired
	  if (cap_sensor == 0)
	  {
	    IoConfig.ChannelIOs = TSC_GROUP2_IO4; /* TS4 touchkey */
	    cap_sensor = 1;
	  }
	  else
	  {
	    IoConfig.ChannelIOs = TSC_GROUP2_IO3; /* TS3 touchkey */
	    cap_sensor = 0;
	  }


	  if (HAL_TSC_IOConfig(htsc, &IoConfig) != HAL_OK)
	  {
	    /* Initialization Error */
	    Error_Handler();
	  }

	  /*##-9- Re-start the acquisition process ###################################*/
	  if (HAL_TSC_Start_IT(htsc) != HAL_OK)
	  {
	    /* Acquisition Error */
	    Error_Handler();
	  }
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
