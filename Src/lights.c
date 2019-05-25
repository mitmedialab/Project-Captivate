/**
  ******************************************************************************
  * File Name          : lights.c
  * Description        : This file provides code RGB and Shift Register Lights
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lights.h"
#include "gpio.h"
#include "stm32wbxx_hal.h"
#include "spi.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"
/*
 * THREADS
 */

#define SHIFT_REG_PERIOD  50
#define RGB_INTERVAL  400

void ShiftRegLightThread(void){

  uint8_t light_sequence = 0xFF;

  while(1){
    if(light_sequence == 0xFF || light_sequence == 0x7F){
          light_sequence = 0xFE;
    }else{
      light_sequence = light_sequence << 1;
      light_sequence = light_sequence | 0x01;
    }

    TransmitShiftRegLight(&light_sequence);

    osDelay(SHIFT_REG_PERIOD);
  }
}

void RGB_LightThread(void){

  while(1){

    RGB_TurnOn_Red();
    osDelay(RGB_INTERVAL);
    RGB_TurnOn_Blue();
    osDelay(RGB_INTERVAL);
    RGB_TurnOn_Green();
    osDelay(RGB_INTERVAL);
    RGB_TurnOff_Red();
    osDelay(RGB_INTERVAL);
    RGB_TurnOff_Blue();
    osDelay(RGB_INTERVAL);
    RGB_TurnOff_Green();
    osDelay(RGB_INTERVAL);

  }
}

/*
 * Helper Functions
 */

void TransmitShiftRegLight(uint8_t *light_sequence){
  HAL_GPIO_WritePin(LED_SS_GPIO_Port, LED_SS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, light_sequence, 1, 1);
  HAL_GPIO_WritePin(LED_SS_GPIO_Port, LED_SS_Pin, GPIO_PIN_SET);
}

void RGB_TurnOn_Red(void){
  HAL_GPIO_WritePin(LED1_RED_GPIO_Port, LED1_RED_Pin, GPIO_PIN_RESET);
}

void RGB_TurnOn_Blue(void){
  HAL_GPIO_WritePin(LED1_BLUE_GPIO_Port, LED1_BLUE_Pin, GPIO_PIN_RESET);
}

void RGB_TurnOn_Green(void){
  HAL_GPIO_WritePin(LED1_GREEN_GPIO_Port, LED1_GREEN_Pin, GPIO_PIN_RESET);
}

void RGB_TurnOff_Red(void){
  HAL_GPIO_WritePin(LED1_RED_GPIO_Port, LED1_RED_Pin, GPIO_PIN_SET);
}

void RGB_TurnOff_Blue(void){
  HAL_GPIO_WritePin(LED1_BLUE_GPIO_Port, LED1_BLUE_Pin, GPIO_PIN_SET);
}

void RGB_TurnOff_Green(void){
  HAL_GPIO_WritePin(LED1_GREEN_GPIO_Port, LED1_GREEN_Pin, GPIO_PIN_SET);
}
