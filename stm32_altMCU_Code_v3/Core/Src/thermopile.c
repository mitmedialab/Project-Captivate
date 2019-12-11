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
#include "tim.h"
#include "stdint.h"
#include <string.h>

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



//uint32_t ADC_Data[600] = {0};
struct thermopileData tempData = {{0}, 0, {0}, 0};
volatile uint16_t test = 0;

void ThermopileTask(void *argument){

  Setup_LMP91051();

  //HAL_ADC_Start(&hadc1);
//  HAL_ADC_Start(&hadc1);
//  HAL_ADC_Start_DMA(&hadc1, temp_ADC_Data, 600);

//  sensorChoice sensor = nose;
  uint32_t evt = 0;

  while(1){
////	HAL_ADC_Start_IT(&hadc1);
////	  osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
////	  HAL_ADC_Start_DMA(&hadc1, (uint32_t *) ADC_Data, 600);
//	  osDelay(4000);
//	  HAL_ADC_Stop_DMA(&hadc1);
//	  HAL_ADC_Start_DMA(&hadc1, ADC_Data, 600);
//
////    for(int i = 0; i <= 120; i++){
////    	HAL_ADC_PollForConversion(&hadc1,100);
////    	temp_ADC_Data[i] = HAL_ADC_GetValue(&hadc1);
////    }
//
//  }
		evt = osThreadFlagsWait (0x00000001U, osFlagsWaitAny, osWaitForever);

		// if signal was received successfully, start blink task
		if (evt == 0x00000001U)  {

			// start timer for ADC to sample at 200Hz
			osThreadFlagsSet(thermopileTaskHandle, 0x00000008U);
//			HAL_TIM_Base_Start(&htim2);
			HAL_TIM_Base_Start_IT(&htim2);

//			HAL_TIM_Base_Start(&htim16);


			// message passing until told to stop
			//      note: DMA triggers callback where buffers are switched and the full one
			//      is passed by reference via queue to masterThread for packetization
			while(1){
				// stop timer and put thread in idle if signal was reset
	//				osDelay(100); // what I should do is do the queue put calls in the DMA callback and suspend the thread here (the masterThread should resume thread when wanting to stop)
	//				blinkMsgBuffer_1.tick_ms = (blinkMsgBuffer_1.tick_ms) + 1;
	//				buffer_pointer = &blinkMsgBuffer_1;
	//		    	osMessageQueuePut(blinkMsgQueueHandle, &buffer_pointer, 0U, 0U);
				//osMessageQueuePut(blinkMsgQueueHandle, &blinkMsgBuffer_2, 0U, 0U);
	//				osMessageQueuePut(blinkMsgQueueHandle, (void *) &blinkMsgBuffer_1, 0U, 0);

				evt = osThreadFlagsWait (0x0000000AU, osFlagsWaitAny, osWaitForever);

				// sampling period start
				if( (evt & 0x00000008U) == 0x00000008U){

					// start DMA for nose
					SwitchTemperatureSensor(nose);
					HAL_ADC_Start_DMA(&hadc1, (uint32_t*) tempData.noseData, 20);

					// wait for data to be received from nose
					evt = osThreadFlagsWait (0x00000004U, osFlagsWaitAny, osWaitForever);
					tempData.nose_tick_ms = HAL_GetTick();

					// ensure master thread hasn't told this thread to stop
					if( (evt & 0x00000002U) == 0x00000002U) break;

					test = tempData.noseData[5];

					// switch thermopile circuitry to nose
					SwitchTemperatureSensor(temple);

					// start DMA for temple
					HAL_ADC_Start_DMA(&hadc1, (uint32_t*) tempData.templeData, sizeof(tempData.templeData));

					// wait for data to be received from temple
					evt = osThreadFlagsWait (0x00000004U, osFlagsWaitAny, osWaitForever);
					tempData.temple_tick_ms = HAL_GetTick();

					// send data to master thread
					osMessageQueuePut(thermMsgQueueHandle, (void *) &tempData, 0U, 0);

//					}
				}

				// stop timer and put thread in idle if signal was reset
				else if( (evt & 0x00000002U) == 0x00000002U){

					HAL_ADC_Stop_DMA(&hadc1);
	//					while(HAL_ADC_Stop(&hadc1) != HAL_OK)
//					HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
//					HAL_TIM_Base_Stop(&htim16);
//					HAL_TIM_Base_Stop(&htim2);
					HAL_TIM_Base_Stop_IT(&htim2);
					osThreadFlagsClear(0x0000000EU);
					break;
				}
			}
		}
	}
}

volatile uint8_t complete = 0;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	complete++;
	//HAL_ADC_Stop_DMA(&hadc1);
//
//	// notify ThermopileTask that conversion is complete
	osThreadFlagsSet(thermopileTaskHandle, 0x00000004U);

}

volatile uint8_t half = 0;
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
	half++;
//	memcpy(blinkMsgBuffer_1.data, &(blink_buffer), 100);
//	blinkMsgBuffer_1.tick_ms = HAL_GetTick();
//	blink_ptr = &blink_buffer;
//	osThreadFlagsSet(blinkTaskHandle, 0x00000004U);

}

volatile uint8_t i = 0;
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  i++;
}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//	osThreadFlagsSet(thermopileTaskHandle, 0x00000008U);
//}

/*
 * Helper Functions
 */
void SwitchTemperatureSensor(sensorChoice sense){

	uint8_t packet[2] = {0};

	packet[0] = LMP91051_CFG_REG;

	if(sense == nose){
		packet[1] = TP_NOSE_SEL | PGA1_EN | PGA2_EN | GAIN2_8 | GAIN1_250 | CMN_MODE_1_15; //todo: add blocking semaphore so no LED conflict
	}
	else if(sense == temple){
		packet[1] = TP_TEMPLE_SEL | PGA1_EN | PGA2_EN | GAIN2_8 | GAIN1_250 | CMN_MODE_1_15; //todo: add blocking semaphore so no LED conflict
	}

	HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi3, packet, 2, 1);
	HAL_Delay(2);
	HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_SET);
}

void Setup_LMP91051(void){
  uint8_t packet[2];
  packet[0] = LMP91051_CFG_REG;
  packet[1] = TP_NOSE_SEL | PGA1_EN | PGA2_EN | GAIN2_8 | GAIN1_250 | CMN_MODE_1_15; //todo: add blocking semaphore so no LED conflict

  HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi3, packet, 2, 1);
  HAL_Delay(2);
  HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_SET);
}

