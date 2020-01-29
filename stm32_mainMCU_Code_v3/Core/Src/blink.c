/**
 ******************************************************************************
 * File Name           : blink.c
  * Description        : C file for blink sensing.
  ******************************************************************************

  *
  ******************************************************************************
 */


/* includes -----------------------------------------------------------*/
#include "blink.h"
#include "gpio.h"
#include "stm32wbxx_hal.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "tim.h"

#include "adc.h"
#include "string.h"

#include "captivate_config.h"


/* typedef -----------------------------------------------------------*/


/* defines -----------------------------------------------------------*/


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/


/* Functions Definition ------------------------------------------------------*/



/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
 * @brief Thread initialization.
 * @param  None
 * @retval None
 */
struct blinkData blinkMsgBuffer_1 = {{0},0,0};
//volatile struct blinkData blinkMsgBuffer_2 = {{0},2};
//
//static struct blinkData *buffer_pointer;

uint8_t blink_buffer[2000] = {0};
uint8_t *blink_ptr;

#define BLINK_HALF_BUFFER_SIZE	1000
#define BLINK_PACKET_SIZE		100
#define BLINK_ITERATOR_COUNT 	BLINK_HALF_BUFFER_SIZE / BLINK_PACKET_SIZE

/* GLOBAL DEFINES */

uint32_t payload_ID = 0;
uint32_t iterator = 0;

float previousTick_ms = 0;
float tick_ms_diff = 0;

void BlinkTask(void *argument){

	uint32_t evt;

	while(1){
		evt = osThreadFlagsWait (0x00000001U, osFlagsWaitAny, osWaitForever);

		// if signal was received successfully, start blink task
		if (evt == 0x00000001U)  {

			// start timer and PWM channel for blink LED
			HAL_TIM_Base_Start(&htim2);
			HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

			// start timer for ADC to sample at 1kHz
			HAL_ADC_Start_DMA(&hadc1, (uint32_t*) blink_buffer, sizeof(blink_buffer));

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

				evt = osThreadFlagsWait (0x00000006U, osFlagsWaitAny, osWaitForever);

				if( (evt & 0x00000004U) == 0x00000004U){

					// interpolate timestamps for blink packets
					if(previousTick_ms == 0){
						previousTick_ms = HAL_GetTick();
					}
					tick_ms_diff = (HAL_GetTick() - previousTick_ms) / ((float) BLINK_ITERATOR_COUNT);

					// because of COAP packet size restrictions, separate blink packet into chunks of size BLINK_PACKET_SIZE
					for(iterator=0; iterator < BLINK_ITERATOR_COUNT; iterator++){

						memcpy(blinkMsgBuffer_1.data, &(blink_buffer[iterator*BLINK_PACKET_SIZE]), BLINK_PACKET_SIZE);
						blinkMsgBuffer_1.tick_ms = previousTick_ms + tick_ms_diff;
						blinkMsgBuffer_1.payload_ID = payload_ID;

						previousTick_ms = blinkMsgBuffer_1.tick_ms;
						payload_ID++;

						osMessageQueuePut(blinkMsgQueueHandle, (void *) &blinkMsgBuffer_1, 0U, 0);
					}
				}

				// stop timer and put thread in idle if signal was reset
				if( (evt & 0x00000002U) == 0x00000002U){

					HAL_ADC_Stop_DMA(&hadc1);
//					while(HAL_ADC_Stop(&hadc1) != HAL_OK)
					HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
					HAL_TIM_Base_Stop(&htim2);
					previousTick_ms = 0;

					// empty queue
					osMessageQueueReset(blinkMsgQueueHandle);

					break;
				}
			}
		}
	}
}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
//}

volatile uint8_t i = 0;
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  i++;
}

//volatile uint8_t complete = 0;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
//	memcpy(blinkMsgBuffer_1.data, &(blink_buffer[100]), 100);
//	blinkMsgBuffer_1.tick_ms = HAL_GetTick();
	blink_ptr = &blink_buffer[BLINK_HALF_BUFFER_SIZE];
	osThreadFlagsSet(blinkTaskHandle, 0x00000004U);

}

//volatile uint8_t half = 0;
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
//	memcpy(blinkMsgBuffer_1.data, &(blink_buffer), 100);
//	blinkMsgBuffer_1.tick_ms = HAL_GetTick();
	blink_ptr = blink_buffer;
	osThreadFlagsSet(blinkTaskHandle, 0x00000004U);

}

//volatile i = 0;
// Called when buffer is half filled
//void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
//	i++;
//}
//
//// Called when buffer is completely filled
//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
//	i++;
//}

/**
 * @brief Setting up blink sensing
 * @param  None
 * @retval None
 */
void SetupBlinkSensing(void){

}
/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
*************************************************************/
