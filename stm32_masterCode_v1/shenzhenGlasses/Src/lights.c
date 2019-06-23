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

#define SHIFT_REG_PERIOD  400
#define RGB_INTERVAL  400

volatile uint8_t LED_SETTINGS[2];

void ShiftRegLightThread(void){

//  uint16_t light_sequence = 0xFFFF;

//	uint16_t light_sequence = 0x1249;
	uint16_t light_sequence = 0x0001;

//  while(1){
//    if(light_sequence == 0xFFFF || light_sequence == 0x7FFF){
//          light_sequence = 0xFFFE;
//    }else{
//      light_sequence = light_sequence << 1;
//      light_sequence = light_sequence | 0x0001;
//    }

//    LED_SETTINGS[0] = (light_sequence >> 8);
//	LED_SETTINGS[1] = light_sequence;
  while(1){

    LED_SETTINGS[0] = light_sequence >> 8;
    LED_SETTINGS[1] = light_sequence;

    TransmitShiftRegLight(LED_SETTINGS);
    light_sequence = light_sequence << 1;
    if(light_sequence == 0) light_sequence = 1;
    osDelay(SHIFT_REG_PERIOD);
  }
}

void RGB_LightThread(void){

  while(1){

    RGB_TurnOn_Red();
    osDelay(RGB_INTERVAL);
    RGB_TurnOff_Red();
    RGB_TurnOn_Blue();
    osDelay(RGB_INTERVAL);
    RGB_TurnOn_Red();
    osDelay(RGB_INTERVAL);
    RGB_TurnOff_Red();
    RGB_TurnOff_Blue();
    osDelay(RGB_INTERVAL);


  }
}

/*
 * Helper Functions
 */

void TransmitShiftRegLight(uint8_t *light_sequence){
  HAL_GPIO_WritePin(LED_SS_GPIO_Port, LED_SS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi1, light_sequence, 2, 1);
  HAL_Delay(2);
  HAL_GPIO_WritePin(LED_SS_GPIO_Port, LED_SS_Pin, GPIO_PIN_SET);
}

void RGB_TurnOn_Red(void){
  HAL_GPIO_WritePin(LED1_RED_GPIO_Port, LED1_RED_Pin, GPIO_PIN_RESET);
}

void RGB_TurnOn_Blue(void){
  HAL_GPIO_WritePin(LED1_BLUE_GPIO_Port, LED1_BLUE_Pin, GPIO_PIN_RESET);
}


void RGB_TurnOff_Red(void){
  HAL_GPIO_WritePin(LED1_RED_GPIO_Port, LED1_RED_Pin, GPIO_PIN_SET);
}

void RGB_TurnOff_Blue(void){
  HAL_GPIO_WritePin(LED1_BLUE_GPIO_Port, LED1_BLUE_Pin, GPIO_PIN_SET);
}
