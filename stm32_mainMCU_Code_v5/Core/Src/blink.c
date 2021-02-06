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
#include "master_thread.h"
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

/* GLOBAL DEFINES */
struct blinkData blinkMsgBuffer_1 = { { 0 }, 0, 0 };
uint8_t blink_buffer[2000] = { 0 };

//uint32_t blink_float_buffer[2000] = { 0 };
//uint8_t blink_buffer_neg[2000] = { 0 };

volatile uint8_t *blink_ptr;
volatile uint8_t *blink_ptr_copy;

uint32_t payload_ID = 0;
uint32_t iterator = 0;
float previousTick_ms = 0;
float tick_ms_diff = 0;

struct LogMessage statusMessage;

/**
 * @brief Thread initialization.
 * @param  None
 * @retval None
 */
void BlinkTask(void *argument) {

	uint32_t evt;

	while (1) {
		evt = osThreadFlagsWait(0x00000001U, osFlagsWaitAny, osWaitForever);

		// if signal was received successfully, start blink task
		if ((evt & 0x00000001U) == 0x00000001U) {

			// tell other threads that blink has been activated
//todo: the oswaitforever option of getting the queue handle should be used or else the system could get messed up down the line
			//not sure why its not working....
//			osMessageQueueGet(statusQueueHandle, &statusMessage, 0U, osWaitForever);

			osMessageQueueGet(statusQueueHandle, &statusMessage, 0U, 1000);
			statusMessage.blinkEnabled = 1;
			osMessageQueuePut(statusQueueHandle, (void*) &statusMessage, 0U, 0);

			// start timer for ADC to sample at 1kHz
			HAL_ADC_Start_DMA(&hadc1, (uint32_t*) blink_buffer,
					sizeof(blink_buffer));

			// start timer for ADC to sample at 1kHz
//			HAL_ADC_Start_DMA(&hadc1, (uint32_t*) blink_float_buffer, sizeof(blink_float_buffer));

			// start  PWM channel for blink LED
			HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

			// start timer
			HAL_TIM_Base_Start(&htim2);

			// message passing until told to stop
			//      note: DMA triggers callback where buffers are switched and the full one
			//      is passed by reference via queue to masterThread for packetization

			while (1) {

				// wait for data ready flag and/or stop task flasg
				evt = osThreadFlagsWait(0x00000006U, osFlagsWaitAny,
						osWaitForever);
				blink_ptr_copy = blink_ptr;

				if ((evt & 0x00000004U) == 0x00000004U) {

					// interpolate timestamps for blink packets
					if (previousTick_ms == 0) {
						previousTick_ms = HAL_GetTick();
					}
					tick_ms_diff = (HAL_GetTick() - previousTick_ms)
							/ ((float) BLINK_ITERATOR_COUNT);

					// because of COAP packet size restrictions, separate blink packet into chunks of size BLINK_PACKET_SIZE
					for (iterator = 0; iterator < BLINK_ITERATOR_COUNT;
							iterator++) {

						// grab packet of size BLINK_PACKET_SIZE
						memcpy(blinkMsgBuffer_1.data,
								&(blink_ptr_copy[iterator * BLINK_PACKET_SIZE]),
								BLINK_PACKET_SIZE);
						blinkMsgBuffer_1.tick_ms = previousTick_ms
								+ tick_ms_diff;
						blinkMsgBuffer_1.payload_ID = payload_ID;

						// add tick cnt
						previousTick_ms = blinkMsgBuffer_1.tick_ms;
						payload_ID++;

						// put into queue
						osMessageQueuePut(blinkMsgQueueHandle,
								(void*) &blinkMsgBuffer_1, 0U, 0);
					}
				}

				// stop timer and put thread in idle if signal was reset
				if ((evt & 0x00000002U) == 0x00000002U) {

					HAL_ADC_Stop_DMA(&hadc1);
					HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
					HAL_TIM_Base_Stop(&htim2);
					previousTick_ms = 0;

					/* tell threads that blink is disabled */
					osMessageQueueGet(statusQueueHandle, &statusMessage, 0U,
							osWaitForever);
					statusMessage.blinkEnabled = 0;
					// notify 3D localization thread that blink is deactivating if active
					if (statusMessage.positionEnabled == 1) {
						osSemaphoreRelease(locNotifyHandle);
					}
					osMessageQueuePut(statusQueueHandle, (void*) &statusMessage,
							0U, 0);

					// empty queue
					osMessageQueueReset(blinkMsgQueueHandle);

					// clear any flags
					osThreadFlagsClear(0x0000000EU);

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

//volatile uint8_t i = 0;
//void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
//	i++;
//}

//volatile uint32_t start_time = 0;
//volatile uint32_t end_time = 0;
//
//volatile uint32_t pwm_tracker_s = 0;
//volatile uint32_t pwm_tracker_diff = 0;
//
//volatile uint32_t pwm_tracker = 0;
//
//volatile uint8_t low_adc_sample = 0;
//volatile uint8_t random_sample = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
//	end_time = HAL_GetTick() - start_time;
//	pwm_tracker_diff = pwm_tracker - pwm_tracker_s;
//	start_time = HAL_GetTick();
//	pwm_tracker_s = pwm_tracker;
//	if(low_adc_sample){
//		HAL_ADC_Stop(&hadc1);
//		low_adc_sample = 0;
//		return;
//	}
	blink_ptr = &blink_buffer[BLINK_HALF_BUFFER_SIZE];
	osThreadFlagsSet(blinkTaskHandle, 0x00000004U);

}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
	blink_ptr = blink_buffer;
	osThreadFlagsSet(blinkTaskHandle, 0x00000004U);

}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
//	if(htim->Instance == TIM2){
//		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
//		HAL_ADC_Start(&hadc1);
//	}
//	pwm_tracker++;

//	low_adc_sample = 1;
//	HAL_ADC_Start(&hadc1);
}

/**
 * @brief Setting up blink sensing
 * @param  None
 * @retval None
 */

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
 *************************************************************/
