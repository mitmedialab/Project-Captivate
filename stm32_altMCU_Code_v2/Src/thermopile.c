/**
  ******************************************************************************
  * File Name          : thermopile.c
  * Description        : This file provides code to command the thermopile IC
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "thermopile.h"
#include "gpio.h"
#include "stm32f3xx_hal.h"
#include "spi.h"
#include "i2c.h"
#include "adc.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"

// CONFIGURATION REGISTER
#define LMP91051_CFG_REG      0x0
#define TP_NOSE_SEL           0x00
#define TP_TEMPLE_SEL         0x80
#define PGA1_EN               0x40
#define PGA2_EN               0x20
#define EXT_FILT_EN           0x10
#define CMN_MODE_2_59         0x08
#define CMN_MODE_1_15         0x00
#define GAIN2_4               0x00
#define GAIN2_8               0x02
#define GAIN2_16              0x04
#define GAIN2_32              0x06
#define GAIN1_250             0x00
#define GAIN1_42              0x01

// DAC REGISTER
//   The output DC level will shift according to the formula Vout_shift = -33.8mV * (NDAC - 128)
#define LMP91051_DAC_REG      0x1

//SDIO ENABLE REGISTER
//  To enter SDIO Mode, write the successive sequence 0xFE and 0xED.
//  Write anything other than this sequence to get out of mode.
#define LMP91051_SDIO_EN_REG  0xF

/*
 * THREADS
 */
volatile uint32_t ADC_Data[600] = {0};
volatile uint16_t temp_ADC_Data[120] = {0};

void ThermopileTask(void){

  Setup_LMP91051();

  //HAL_ADC_Start(&hadc1);
  HAL_ADC_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1, ADC_Data, 600);

  while(1){
//	HAL_ADC_Start_IT(&hadc1);
//	  osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
//	  HAL_ADC_Start_DMA(&hadc1, (uint32_t *) ADC_Data, 600);
	  osDelay(4000);
	  HAL_ADC_Stop_DMA(&hadc1);
	  HAL_ADC_Start_DMA(&hadc1, ADC_Data, 600);

//    for(int i = 0; i <= 120; i++){
//    	HAL_ADC_PollForConversion(&hadc1,100);
//    	temp_ADC_Data[i] = HAL_ADC_GetValue(&hadc1);
//    }

  }
}


/*
 * Helper Functions
 */

void Setup_LMP91051(void){
  uint8_t packet[2];
  packet[0] = LMP91051_CFG_REG;
  packet[1] = TP_NOSE_SEL | PGA1_EN | PGA2_EN | GAIN2_8 | GAIN1_250 | CMN_MODE_1_15; //todo: add blocking semaphore so no LED conflict

  HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi3, packet, 2, 1);
  HAL_Delay(2);
  HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_SET);
}

