/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
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
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "input.h"
#include "pulse_processor.h"
#include "circular_buffer.h"
#include "tsc.h"
#include "touch_detector.h"
#include "touchsensing.h"
#include "captivate_config.h"
#include "usbd_cdc.h"
#include "usb_device.h"
#include "touch_detector.h"
#include "otp.h"
#include "stm32_lpm.h"
#include "stm32_seq.h"
#include "dbg_trace.h"
#include "hw_conf.h"
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

/* USER CODE BEGIN PV */
#ifdef NUCLEO_LED_ACTIVE
GPIO_TypeDef* GPIO_PORT[LEDn] = {LED1_GPIO_PORT, LED2_GPIO_PORT, LED3_GPIO_PORT};
const uint16_t GPIO_PIN[LEDn] = {LED1_PIN, LED2_PIN, LED3_PIN};
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void PeriphClock_Config(void);
static void Reset_Device(void);
static void Reset_IPCC(void);
static void Reset_BackupDomain(void);
static void Init_Exti(void);
static void Config_HSE(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	/**
	 * The OPTVERR flag is wrongly set at power on
	 * It shall be cleared before using any HAL_FLASH_xxx() api
	 */

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */
	Reset_Device();
	Config_HSE();
	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC1_Init();
//  MX_COMP1_Init();
	MX_RF_Init();
	MX_I2C1_Init();
	MX_RTC_Init();
	MX_TIM2_Init();
	MX_TIM16_Init();
	MX_USB_Device_Init();
	/* USER CODE BEGIN 2 */
	MX_TSC_Init();

	/* the below code makes the blink PWM into a GPIO since the QRE cannot be modulated due to the terrible response of the diode */
	MX_BLINK_GPIO_Init();


#ifndef BORDER_ROUTER_NODE
	USBD_Stop(&hUsbDeviceFS);
#endif
	HAL_Delay(100);
	APPE_Init();

	/* USER CODE END 2 */

	/* Init scheduler */
	osKernelInitialize(); /* Call init function for freertos objects (in freertos.c) */
	MX_FREERTOS_Init();
	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		UTIL_SEQ_Run( UTIL_SEQ_DEFAULT);
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };

	/** Macro to configure the PLL multiplication factor
	 */
	__HAL_RCC_PLL_PLLM_CONFIG(RCC_PLLM_DIV2);
	/** Macro to configure the PLL clock source
	 */
	__HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
	/** Configure the main internal regulator output voltage
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI
			| RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4 | RCC_CLOCKTYPE_HCLK2
			| RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1
			| RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the peripherals clocks
	 */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS
			| RCC_PERIPHCLK_RFWAKEUP | RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_I2C1
			| RCC_PERIPHCLK_USB | RCC_PERIPHCLK_ADC;
	PeriphClkInitStruct.PLLSAI1.PLLN = 6;
	PeriphClkInitStruct.PLLSAI1.PLLP = RCC_PLLP_DIV2;
	PeriphClkInitStruct.PLLSAI1.PLLQ = RCC_PLLQ_DIV2;
	PeriphClkInitStruct.PLLSAI1.PLLR = RCC_PLLR_DIV2;
	PeriphClkInitStruct.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_USBCLK
			| RCC_PLLSAI1_ADCCLK;
	PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
	PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLLSAI1;
	PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV32;
	PeriphClkInitStruct.RFWakeUpClockSelection = RCC_RFWKPCLKSOURCE_LSE;
	PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSE;
	PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN Smps */

	/* USER CODE END Smps */
}

/* USER CODE BEGIN 4 */

void PeriphClock_Config(void) {
#if (CFG_USB_INTERFACE_ENABLE != 0)
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };
	RCC_CRSInitTypeDef RCC_CRSInitStruct = { 0 };

	/**
   * This prevents the CPU2 to disable the HSI48 oscillator when
   * it does not use anymore the RNG IP
   */
  LL_HSEM_1StepLock( HSEM, 5 );

  LL_RCC_HSI48_Enable();

	while(!LL_RCC_HSI48_IsReady());

	/* Select HSI48 as USB clock source */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
	PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

	/*Configure the clock recovery system (CRS)**********************************/

	/* Enable CRS Clock */
	__HAL_RCC_CRS_CLK_ENABLE();

	/* Default Synchro Signal division factor (not divided) */
	RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;

	/* Set the SYNCSRC[1:0] bits according to CRS_Source value */
	RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;

	/* HSI48 is synchronized with USB SOF at 1KHz rate */
	RCC_CRSInitStruct.ReloadValue = RCC_CRS_RELOADVALUE_DEFAULT;
	RCC_CRSInitStruct.ErrorLimitValue = RCC_CRS_ERRORLIMIT_DEFAULT;

	RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;

	/* Set the TRIM[5:0] to the default value*/
	RCC_CRSInitStruct.HSI48CalibrationValue = RCC_CRS_HSI48CALIBRATION_DEFAULT;

	/* Start automatic synchronization */
	HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
#endif

	return;
}

//void SystemClock_Config(void)
//{
//  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
//  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
//  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
//
//  /** Configure LSE Drive Capability
//  */
//  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
//  /** Configure the main internal regulator output voltage
//  */
//  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
//  /** Initializes the CPU, AHB and APB busses clocks
//  */
//  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE
//                              |RCC_OSCILLATORTYPE_LSE;
//  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
//  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
//  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
//  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
//  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
//  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
//  */
//  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
//                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
//                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
//  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
//  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
//  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
//  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
//  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
//  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;
//
//  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /** Initializes the peripherals clocks
//  */
//  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS|RCC_PERIPHCLK_RFWAKEUP
//                              |RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
//                              |RCC_PERIPHCLK_LPUART1;
//  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
//  PeriphClkInitStruct.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
//  //PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
//  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV32;
//  PeriphClkInitStruct.RFWakeUpClockSelection = RCC_RFWKPCLKSOURCE_LSE;
//  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSE;
//  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE1;
//
//  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /* USER CODE BEGIN Smps */
//
//#if (CFG_USE_SMPS != 0)
//  /**
//   *  Configure and enable SMPS
//   *
//   *  The SMPS configuration is not yet supported by CubeMx
//   *  when SMPS output voltage is set to 1.4V, the RF output power is limited to 3.7dBm
//   *  the SMPS output voltage shall be increased for higher RF output power
//   */
//  LL_PWR_SMPS_SetStartupCurrent(LL_PWR_SMPS_STARTUP_CURRENT_80MA);
//  LL_PWR_SMPS_SetOutputVoltageLevel(LL_PWR_SMPS_OUTPUT_VOLTAGE_1V40);
//  LL_PWR_SMPS_Enable();
//#endif
//
//  /* USER CODE END Smps */
//}

static void Config_HSE(void) {
	OTP_ID0_t *p_otp;

	/**
	 * Read HSE_Tuning from OTP
	 */
	p_otp = (OTP_ID0_t*) OTP_Read(0);
	if (p_otp) {
		LL_RCC_HSE_SetCapacitorTuning(p_otp->hse_tuning);
	}

	return;
}

static void Reset_Device(void) {
#if ( CFG_HW_RESET_BY_FW == 1 )
	Reset_BackupDomain();

	Reset_IPCC();
#endif

	return;
}

static void Reset_IPCC(void) {
	LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_IPCC);

	LL_C1_IPCC_ClearFlag_CHx(
	IPCC,
			LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3
					| LL_IPCC_CHANNEL_4 | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

	LL_C2_IPCC_ClearFlag_CHx(
	IPCC,
			LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3
					| LL_IPCC_CHANNEL_4 | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

	LL_C1_IPCC_DisableTransmitChannel(
	IPCC,
			LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3
					| LL_IPCC_CHANNEL_4 | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

	LL_C2_IPCC_DisableTransmitChannel(
	IPCC,
			LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3
					| LL_IPCC_CHANNEL_4 | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

	LL_C1_IPCC_DisableReceiveChannel(
	IPCC,
			LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3
					| LL_IPCC_CHANNEL_4 | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

	LL_C2_IPCC_DisableReceiveChannel(
	IPCC,
			LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3
					| LL_IPCC_CHANNEL_4 | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

	return;
}

static void Reset_BackupDomain(void) {
	if ((LL_RCC_IsActiveFlag_PINRST() != FALSE)
			&& (LL_RCC_IsActiveFlag_SFTRST() == FALSE)) {
		HAL_PWR_EnableBkUpAccess(); /**< Enable access to the RTC registers */

		/**
		 *  Write twice the value to flush the APB-AHB bridge
		 *  This bit shall be written in the register before writing the next one
		 */
		HAL_PWR_EnableBkUpAccess();

		__HAL_RCC_BACKUPRESET_FORCE();
		__HAL_RCC_BACKUPRESET_RELEASE();
	}

	return;
}

static void Init_Exti(void) {
	/**< Disable all wakeup interrupt on CPU1  except IPCC(36), HSEM(38) */
	LL_EXTI_DisableIT_0_31(~0);
	LL_EXTI_DisableIT_32_63((~0) & (~(LL_EXTI_LINE_36 | LL_EXTI_LINE_38)));

	return;
}

#ifdef NUCLEO_LED_ACTIVE
void BSP_LED_Init(Led_TypeDef Led)
{
  GPIO_InitTypeDef  gpioinitstruct = {0};

  /* Enable the GPIO_LED Clock */
  LEDx_GPIO_CLK_ENABLE(Led);

  /* Configure the GPIO_LED pin */
  gpioinitstruct.Pin = GPIO_PIN[Led];
  gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
  gpioinitstruct.Pull = GPIO_NOPULL;
  gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(GPIO_PORT[Led], &gpioinitstruct);

  HAL_GPIO_WritePin(GPIO_PORT[Led], GPIO_PIN[Led], GPIO_PIN_RESET);
}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on.
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  *     @arg LED3
  * @retval None
  */
void BSP_LED_On(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(GPIO_PORT[Led], GPIO_PIN[Led], GPIO_PIN_SET);
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off.
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  *     @arg LED3
  * @retval None
  */
void BSP_LED_Off(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(GPIO_PORT[Led], GPIO_PIN[Led], GPIO_PIN_RESET);
}

void BSP_LED_Toggle(Led_TypeDef Led)
{
  HAL_GPIO_TogglePin(GPIO_PORT[Led], GPIO_PIN[Led]);
}
#endif

/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM1 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
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
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
