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

#define TP_SAMPLING_PERIOD 				100
#define CONTEXT_SWITCH_COMPENSATION 	2
#define SAMPLING_DELAY 					TP_SAMPLING_PERIOD - CONTEXT_SWITCH_COMPENSATION




/*
 * THREADS
 */



//uint32_t ADC_Data[600] = {0};
struct thermopilePackagedData tempData;
//volatile uint16_t test = 0;

struct adcThermopileData buffer = {0};
//volatile uint16_t buffer_2[3] = {0};
//volatile uint32_t avgVal = 0;
volatile uint16_t start;
volatile uint16_t stop;
//uint16_t diff;

volatile uint32_t evt = 0;

void ThermopileTask(void *argument){

  Setup_LMP91051();

  //HAL_ADC_Start(&hadc1);
//  HAL_ADC_Start(&hadc1);
//  HAL_ADC_Start_DMA(&hadc1, temp_ADC_Data, 600);

//  sensorChoice sensor = nose;

  uint8_t index_thermopile = 0;
  uint32_t avgVal = 0;
  int32_t delta_time = 0;

  while(1){

	  	/********* WAIT FOR START CONDITION FROM MASTER THREAD ************************/
		evt = osThreadFlagsWait (0x00000001U, osFlagsWaitAny, osWaitForever);
//	  evt = 0x00000001U;
	    // if signal was received successfully, start blink task
		if (evt == 0x00000001U)  {

			// start timer for ADC to sample at 20Hz (10Hz each channel)
			HAL_TIM_Base_Start(&htim6);

			while(1){
				// start session to grab five samples of nose and temple thermopiles and thermistors
				//   note: the ADC samples in the following order: [selected thermopile, temple thermistor, nose thermistor

				for(int i = 0; i < NUM_THERM_SAMPLES; i++){
					start = HAL_GetTick();

					/********* START DMA FOR ONE NOSE SAMPLE ************************/
					SwitchTemperatureSensor(nose);
					HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &buffer, 12);

					// wait for data to be received from nose
					evt = osThreadFlagsWait (0x00000006U, osFlagsWaitAny, osWaitForever);

					// break if told to stop by master thread
					if( (evt & 0x00000002U) == 0x00000002U){
						break;
					}

					// turn off PGA's to save power
					TurnOff_LMP91051();

					// get avg thermopile value
					avgVal = 0;
					for(index_thermopile = 0; index_thermopile<10; index_thermopile++){
						avgVal += buffer.thermopile[index_thermopile];
					}
					avgVal = avgVal / 10;

					// copy the data from the buffer and continue
					tempData.nose[i].tick_ms = HAL_GetTick();
					tempData.nose[i].thermopile = (uint16_t) avgVal;
					tempData.nose[i].thermistor = buffer.nose_thermistor;

					/********* START DMA FOR ONE TEMPLE SAMPLE ************************/
					SwitchTemperatureSensor(temple);

					HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &buffer, 12);

					// wait for data to be received from temple
					evt = osThreadFlagsWait (0x00000006U, osFlagsWaitAny, osWaitForever);

					// break if told to stop by master thread
					if( (evt & 0x00000002U) == 0x00000002U){
						break;
					}

					// turn off PGA's to save power
					TurnOff_LMP91051();

					// get avg thermopile value
					avgVal = 0;
					for(index_thermopile = 0; index_thermopile<10; index_thermopile++){
						avgVal += buffer.thermopile[index_thermopile];
					}
					avgVal = avgVal / 10;

					// copy the data from the buffer and continue
					tempData.temple[i].tick_ms = HAL_GetTick();
					tempData.temple[i].thermopile = (uint16_t) avgVal;
					tempData.temple[i].thermistor = buffer.temple_thermistor;

					stop = HAL_GetTick() - start;

					// wait for next sampling period
					delta_time = SAMPLING_DELAY - stop;
					if((delta_time > 0) && (delta_time < SAMPLING_DELAY)){
						osDelay(delta_time);
					}
				}

				if( (evt & 0x00000002U) == 0x00000002U){
					break;
				}
				// if not told to shut down sampling
				else{
					// send data to master thread
					osMessageQueuePut(thermMsgQueueHandle, &tempData, 0U, 0);
				}
			}

			// stop timer and put thread in idle if signal was reset
			if( (evt & 0x00000002U) == 0x00000002U){
				HAL_ADC_Stop_DMA(&hadc1);
				HAL_ADC_Stop(&hadc1);
				HAL_TIM_Base_Stop(&htim6);

				// empty queue
				osMessageQueueReset(thermMsgQueueHandle);

				// clear any flags
				osThreadFlagsClear(0x0000000EU);

				// turn off PGA's to save power
				TurnOff_LMP91051();

				// exit and wait for another start condition
//				break;
			}
		}
	}
}

volatile uint8_t complete = 0;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	complete++;

	// notify ThermopileTask that conversion is complete
	while( HAL_ADC_Stop(&hadc1) != HAL_OK);
	osThreadFlagsSet(thermopileTaskHandle, 0x00000004U);

}

//volatile uint8_t half = 0;
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
	//half++;
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
//		packet[1] = TP_NOSE_SEL | PGA1_EN | PGA2_EN | GAIN2_4| GAIN1_42 | CMN_MODE_1_15; //todo: add blocking semaphore so no LED conflict
		packet[1] = TP_NOSE_SEL | PGA1_EN | PGA2_EN | GAIN2_4 | GAIN1_250 | CMN_MODE_1_15 | EXT_FILT_EN; //todo: add blocking semaphore so no LED conflict

		//		packet[1] = TP_NOSE_SEL | PGA1_EN | PGA2_EN | GAIN2_32| GAIN1_42 | CMN_MODE_1_15; //todo: add blocking semaphore so no LED conflict

//		packet[1] = TP_NOSE_SEL | PGA1_EN | PGA2_EN | GAIN2_16 | GAIN1_42 | CMN_MODE_1_15; //todo: add blocking semaphore so no LED conflict
	}
	else if(sense == temple){
		packet[1] = TP_TEMPLE_SEL | PGA1_EN | PGA2_EN | GAIN2_4 | GAIN1_250 | CMN_MODE_1_15 | EXT_FILT_EN; //todo: add blocking semaphore so no LED conflict

//		packet[1] = TP_TEMPLE_SEL | PGA1_EN | PGA2_EN | GAIN2_4 | GAIN1_250 | CMN_MODE_1_15; //todo: add blocking semaphore so no LED conflict
	}

	HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi3, packet, 2, 1);
	HAL_Delay(1);
	HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_SET);

//	packet[0] = LMP91051_DAC_REG;
//	if(sense == nose){
//		packet[1] = 128; // shift signal down by -33.8mV (134 - 128) during 2nd stage amp
//	}
//	else if(sense == temple){
//		packet[1] = 134; // shift signal down by -33.8mV (128 - 128) during 2nd stage amp
//	}
//
//	HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_RESET);
//	HAL_SPI_Transmit(&hspi3, packet, 2, 1);
//	HAL_Delay(2);
//	HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_SET);
}

void Setup_LMP91051(void){
  uint8_t packet[2];
//  packet[0] = LMP91051_CFG_REG;
//  packet[1] = TP_NOSE_SEL | PGA1_EN | PGA2_EN | GAIN2_32 | GAIN1_42 | CMN_MODE_1_15 | EXT_FILT_EN; //todo: add blocking semaphore so no LED conflict
//  HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_RESET);
//  HAL_SPI_Transmit(&hspi3, packet, 2, 1);
//  HAL_Delay(2);
//  HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_SET);

  packet[0] = LMP91051_DAC_REG;
//  packet[1] = 105; // shift signal down by -33.8mV (NDAC - 128) during 2nd stage amp
//  packet[1] = 130; // shift signal down by -33.8mV (NDAC - 128) during 2nd stage amp
  packet[1] = 110; // shift signal down by -33.8mV (NDAC - 128) during 2nd stage amp
//  packet[1] = 140; // shift signal down by -33.8mV (159 - 128) during 2nd stage amp
  HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi3, packet, 2, 1);
  HAL_Delay(2);
  HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_SET);

  TurnOff_LMP91051();
}

void TurnOff_LMP91051(void){
  uint8_t packet[2];
  packet[0] = LMP91051_CFG_REG;
  packet[1] = 0; //todo: add blocking semaphore so no LED conflict
  HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi3, packet, 2, 1);
  HAL_Delay(2);
  HAL_GPIO_WritePin(TP_SS_GPIO_Port, TP_SS_Pin, GPIO_PIN_SET);
}


