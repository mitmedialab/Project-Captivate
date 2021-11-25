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
#include "arm_math.h"
#include "math.h"
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

//volatile uint8_t blink_copy[1000];

//uint32_t payload_ID = 0;
uint32_t iterator = 0;
float previousTick_ms = 0;
float tick_ms_diff = 0;

struct LogMessage statusMessage;
uint8_t diodeState = 0;

CaptivatePacket *captivatePacket;

/**
 * @brief Thread initialization.
 * @param  None
 * @retval None
 */
void BlinkTask(void *argument) {

	uint32_t evt;
	uint8_t diodeSaturatedFlag = 0;
	float rolling_avg = 255;
	uint16_t packetsPerHalfBuffer = ceil( ( (float) BLINK_HALF_BUFFER_SIZE)/BLINK_PACKET_SIZE );
	uint16_t payloadLength = 0;
	uint16_t blinkDataTracker = 0;
	uint32_t payload_ID = 0;
	uint32_t tickCnt;
	uint32_t blinkSampleHalfBuffer_ms = BLINK_HALF_BUFFER_SIZE * (1.0/BLINK_SAMPLE_RATE) * 1000.0;
	uint32_t packetRemainder = BLINK_SAMPLE_RATE % BLINK_PACKET_SIZE;


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

//
//			 start timer
			HAL_TIM_Base_Start(&htim2);

//			 start  PWM channel for blink LED
//			if(HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2) == HAL_OK){
//				diodeState = 1;
//			}

			HAL_GPIO_WritePin(BLINK_PWM_GPIO_Port, BLINK_PWM_Pin,
					GPIO_PIN_SET);
			diodeState = 1;

			// reset external infrared detection flag
			diodeSaturatedFlag = 0;

			// message passing until told to stop
			//      note: DMA triggers callback where buffers are switched and the full one
			//      is passed by reference via queue to masterThread for packetization

			while (1) {

				// wait for data ready flag and/or stop task flags
				evt = osThreadFlagsWait(0x00000006U, osFlagsWaitAny,
						osWaitForever);
				blink_ptr_copy = blink_ptr;
//				memcpy(blink_copy,blink_ptr,1000);

				if ((evt & 0x00000004U) == 0x00000004U) {

					tickCnt = HAL_GetTick() - blinkSampleHalfBuffer_ms;

					// interpolate timestamps for blink packets
					if (previousTick_ms == 0) {
						previousTick_ms = HAL_GetTick();
					}
					tick_ms_diff = (HAL_GetTick() - previousTick_ms)
							/ ((float) BLINK_ITERATOR_COUNT);

					/* check to see if external infrared is saturating
					 *   if so, disable active diode
					 *   otherwise, leave enabled */

					// BLINK_SAMPLE_RATE == size of blink_ptr array
					diodeSaturatedFlag = externalInfraredDetect(blink_ptr_copy, BLINK_SAMPLE_RATE, &rolling_avg);

					if(diodeSaturatedFlag){
						if(diodeState) turnOffDiode();
					}
					else{
						if(!diodeState) turnOnDiode();
					}

					blinkDataTracker = BLINK_HALF_BUFFER_SIZE;
					// because of COAP packet size restrictions, separate blink packet into chunks of size BLINK_PACKET_SIZE
					for (iterator = 0; iterator < packetsPerHalfBuffer;
							iterator++) {
						payload_ID++;

						if(blinkDataTracker > BLINK_PACKET_SIZE){
						    payloadLength = BLINK_PACKET_SIZE;
						    blinkDataTracker -= BLINK_PACKET_SIZE;
						}else if(blinkDataTracker != 0){
						    payloadLength = blinkDataTracker;
						    blinkDataTracker = 0;
						}else{
						    break; //should never happen
						}

						// grab available memory for packet creation
						if(osOK != osMessageQueueGet(capPacketAvail_QueueHandle, &captivatePacket, 0U,
							    300)){
						    continue; //no memory available so drop packet
						}

						// copy payload
						memcpy(captivatePacket->payload,
								&(blink_ptr_copy[iterator * BLINK_PACKET_SIZE]),
								payloadLength);

						tickCnt += payloadLength;
						captivatePacket->header.packetType = BLINK_DATA;
						captivatePacket->header.packetID = payload_ID;
						captivatePacket->header.msFromStart = tickCnt;
						captivatePacket->header.epoch = 0;
						captivatePacket->header.payloadLength = payloadLength;
						captivatePacket->header.reserved[0] = diodeSaturatedFlag;

						// add tick cnt
						previousTick_ms = blinkMsgBuffer_1.tick_ms;

						// put into queue
						osMessageQueuePut(capPacket_QueueHandle,
								&captivatePacket, 0U, 0);

					}
				}

				// stop timer and put thread in idle if signal was reset
				if ((evt & 0x00000002U) == 0x00000002U) {

					HAL_ADC_Stop_DMA(&hadc1);
//					if(HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2) == HAL_OK){
//						diodeState = 0;
//					}
					HAL_GPIO_WritePin(BLINK_PWM_GPIO_Port, BLINK_PWM_Pin,
										GPIO_PIN_RESET);
					diodeState = 0;

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

					payload_ID = 0;

					break;
				}
			}
		}
	}
}

void turnOffDiode(){
	HAL_GPIO_WritePin(BLINK_PWM_GPIO_Port, BLINK_PWM_Pin,
			GPIO_PIN_RESET);
	diodeState = 0;

}

void turnOnDiode(){
	HAL_GPIO_WritePin(BLINK_PWM_GPIO_Port, BLINK_PWM_Pin,
			GPIO_PIN_SET);
	diodeState = 1;
}

#define INFRARED_DETECT 	1
#define NO_INFRARED_DETECT	0
float32_t sample_avg;
uint8_t detect_active = 0;
uint8_t externalInfraredDetect(uint8_t* blink_sample, uint32_t size_of_blink_ptr, float* rolling_avg){

	//todo: convert to q format or float32 and use arm library. uint8_t can't be typecasted to 32-bit float
//	arm_mean_f32((float *) random_array, 2, &sample_avg);

	// temporary brute force average
	sample_avg = blink_sample[0] + blink_sample[100] + blink_sample[200] + blink_sample[300] + blink_sample[400]
               + blink_sample[500] + blink_sample[600] + blink_sample[700] + blink_sample[800] + blink_sample[900];
	sample_avg /= 10;

	*rolling_avg = INFRARED_DETECT_ALPHA * sample_avg + (1.0-INFRARED_DETECT_ALPHA) * (*rolling_avg);

	/* SCHMITT TRIGGER */
	if(detect_active){
		if( (*rolling_avg) > INFRARED_DETECT_UPPER_THRESH ){
			detect_active = NO_INFRARED_DETECT;
		}
	}else{
		if( (*rolling_avg) < INFRARED_DETECT_LOWER_THRESH ){
			detect_active = INFRARED_DETECT;
		}
	}

	return detect_active;
}
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
//}

//volatile uint8_t i = 0;
//void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
//	i++;
//}

volatile uint32_t start_time = 0;
volatile uint32_t end_time = 0;
//
//volatile uint32_t pwm_tracker_s = 0;
//volatile uint32_t pwm_tracker_diff = 0;
//
//volatile uint32_t pwm_tracker = 0;
//
//volatile uint8_t low_adc_sample = 0;
//volatile uint8_t random_sample = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	end_time = HAL_GetTick() - start_time;
//	pwm_tracker_diff = pwm_tracker - pwm_tracker_s;
	start_time = HAL_GetTick();
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
