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
#include "stm32f3xx_hal.h"

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
#define DIODE_RIGHT_Pin GPIO_PIN_2
#define DIODE_RIGHT_GPIO_Port GPIOA
#define DIODE_LEFT_Pin GPIO_PIN_3
#define DIODE_LEFT_GPIO_Port GPIOA
#define EXPANSION_INT_Pin GPIO_PIN_4
#define EXPANSION_INT_GPIO_Port GPIOA
#define IMU_INT_Pin GPIO_PIN_5
#define IMU_INT_GPIO_Port GPIOA
#define TP_SS_Pin GPIO_PIN_6
#define TP_SS_GPIO_Port GPIOA
#define DIODE_CENTER_Pin GPIO_PIN_0
#define DIODE_CENTER_GPIO_Port GPIOB
#define TP25_Pin GPIO_PIN_8
#define TP25_GPIO_Port GPIOA
#define TP24_Pin GPIO_PIN_9
#define TP24_GPIO_Port GPIOA
#define TP22_Pin GPIO_PIN_10
#define TP22_GPIO_Port GPIOA
#define TP20_Pin GPIO_PIN_12
#define TP20_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
