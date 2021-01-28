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
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#define LH_SIG_Pin GPIO_PIN_1
#define LH_SIG_GPIO_Port GPIOA
#define IMU_INT_Pin GPIO_PIN_5
#define IMU_INT_GPIO_Port GPIOA
#define IMU_INT_EXTI_IRQn EXTI9_5_IRQn
#define EXPANSION_INT_Pin GPIO_PIN_2
#define EXPANSION_INT_GPIO_Port GPIOB
#define EXPANSION_INT_EXTI_IRQn EXTI2_IRQn
/* USER CODE BEGIN Private defines */
#define LED1_GPIO_Port	GPIOA
#define LED2_GPIO_Port	GPIOB
#define LED3_GPIO_Port	GPIOB
#define LED1_Pin	GPIO_PIN_4
#define LED2_Pin	GPIO_PIN_0
#define LED3_Pin	GPIO_PIN_1
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/