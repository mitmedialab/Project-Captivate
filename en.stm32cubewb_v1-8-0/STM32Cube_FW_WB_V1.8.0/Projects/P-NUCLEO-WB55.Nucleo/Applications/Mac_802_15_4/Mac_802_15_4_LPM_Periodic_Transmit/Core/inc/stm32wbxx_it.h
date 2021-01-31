/**
  ******************************************************************************
  * @file    stm32wbxx_it.h
  * @author  MCD Application Team
  * @brief   This file contains the headers of the interrupt handlers.
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32WB_IT_H
#define STM32WB_IT_H

#include "stm32wbxx_hal.h"
#include "hw_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* External variable ----------------------------------------------------------*/
#ifdef USAGE_OF_VCP
extern PCD_HandleTypeDef hpcd; /* Defined in the Thread Middleware             */
#endif /* USAGE_OF_VCP */

/* Exported functions ------------------------------------------------------- */

void NMI_Handler(void);
void HardFault_Handler(void);
void SysTick_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void IPCC_C1_RX_IRQHandler(void);
void IPCC_C1_TX_IRQHandler(void);

void LPTIM1_IRQHandler(void);

/* USER CODE BEGIN EFP */
void EXTI4_IRQHandler(void);
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
/* USER CODE END EFP */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* STM32WB_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
