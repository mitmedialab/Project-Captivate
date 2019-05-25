/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wbxx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LH_SIG_Pin GPIO_PIN_5
#define LH_SIG_GPIO_Port GPIOA
#define BATT_MEAS_Pin GPIO_PIN_6
#define BATT_MEAS_GPIO_Port GPIOA
#define LED_MOSI_Pin GPIO_PIN_7
#define LED_MOSI_GPIO_Port GPIOA
#define LED_SS_Pin GPIO_PIN_8
#define LED_SS_GPIO_Port GPIOA
#define LED_PWM_Pin GPIO_PIN_9
#define LED_PWM_GPIO_Port GPIOA
#define BATT_MEAS_CTRL_Pin GPIO_PIN_2
#define BATT_MEAS_CTRL_GPIO_Port GPIOB
#define LED1_RED_Pin GPIO_PIN_0
#define LED1_RED_GPIO_Port GPIOB
#define LED1_GREEN_Pin GPIO_PIN_1
#define LED1_GREEN_GPIO_Port GPIOB
#define LED1_BLUE_Pin GPIO_PIN_4
#define LED1_BLUE_GPIO_Port GPIOE
#define USB_DN_Pin GPIO_PIN_11
#define USB_DN_GPIO_Port GPIOA
#define BUZZER_Pin GPIO_PIN_15
#define BUZZER_GPIO_Port GPIOA
#define LED_SCK_Pin GPIO_PIN_3
#define LED_SCK_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
