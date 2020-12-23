/*
 * input.c
 *
 *  Created on: Dec 4, 2019
 *      Author: giand
 */
#include "input.h"
#include "comp.h"
#include "master_thread.h"
#include "captivate_config.h"

#include "string.h"
#include "app_thread.h"

PulseProcessor pulse_processor;
GeometryBuilder geometry_builder;

void _Input(Input *self, uint8_t input_idx){
	_GeometryBuilder(&geometry_builder);
	_PulseProcessor(&pulse_processor, 1);
	pulse_processor.next = &geometry_builder;
	self->next = &pulse_processor;
	self->input_idx_ = input_idx;

}
volatile uint32_t count = 0;
Pulse p;
void PulseHandlerTask(void *argument){
	uint32_t evt = 0;
	while(1){
		osThreadFlagsWait(0x00000001U, osFlagsWaitAny, osWaitForever);
		//Start interrupt
//		HAL_NVIC_EnableIRQ(COMP_IRQn);
		HAL_TIM_Base_Start(&htim16);
		HAL_COMP_Start(&hcomp1);
		_Input(&input0, 0);
		while(1){
			osMessageQueueGet (pulseQueueHandle, (void *) &p, NULL, osWaitForever);
			consume_pulse(input0.next, &p);

			evt = osThreadFlagsWait(0x00000002U, osFlagsWaitAny, 0);
			if((evt & 0x00000002U) == 0x00000002U){
				//Stop interrupt
//				HAL_NVIC_DisableIRQ(COMP_IRQn);
				HAL_COMP_Stop(&hcomp1);
				HAL_TIM_Base_Stop(&htim16);
				osMessageQueueReset(pulseQueueHandle);
				break;
			}
		}
	}

}

VIVEVars vive_vars;
struct LogMessage statusMessage;
struct VIVEVars vive_loc_demo;

void get3D_location(void *arguments){

	uint8_t blinkActive	= 0;
	uint32_t blinkState;

	memcpy(&blinkState, arguments, sizeof(blinkState));

	// ensures semaphores are clear
	osSemaphoreAcquire(locCompleteHandle, 0);
	osSemaphoreAcquire(locNotifyHandle, 0);

	osMessageQueueGet(statusQueueHandle, &statusMessage, 0U, osWaitForever);
	if(statusMessage.blinkEnabled == 1){
		osMessageQueuePut(statusQueueHandle, (void *) &statusMessage, 0U, 0);

		blinkActive = 1;

		// disable blink if active
		osThreadFlagsSet(blinkTaskHandle, 0x00000002U);

		// wait for blink thread to turn off
		//TODO: make safer by making a break condition
		osSemaphoreAcquire(locNotifyHandle, osWaitForever);
	}
	else{
		osMessageQueuePut(statusQueueHandle, (void *) &statusMessage, 0U, 0);
	}

	// stop blink code if running and hold I2C bus from being used
	// 		the timeout is to ensure to turn off the thread if no signal is seen
	//TODO: optimize the timeout
	osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);

	// turn on 3D localization
	osThreadFlagsSet(pulseTaskHandle, 0x00000001U);

	// wait for completion
	osSemaphoreAcquire (locCompleteHandle, GET_3D_LOC_TIMEOUT);

#ifdef VIVE_THREAD_INFINITE_TIMEOUT
	osDelay(osWaitForever);
#endif

	if(blinkState == LIGHT_LAB_DEMO){
		while(osOK != osSemaphoreAcquire(lightingLabDemoEndHandle, 0)){
			if(osOK == osMessageQueueGet(viveQueueHandle, &vive_loc_demo, 0U, 1000)){
				APP_THREAD_SendBorderMessage(&vive_loc_demo, sizeof(VIVEVars), "capLoc");
			}
//			else{
//				vive_loc_demo.pos[2] = 100;
//				APP_THREAD_SendBorderMessage(&vive_loc_demo, sizeof(VIVEVars), "capLoc");
//			}
		}
	}

	// release I2C handle
	osSemaphoreRelease(messageI2C_LockHandle);

	// turn off 3D localization
	osThreadFlagsSet(pulseTaskHandle, 0x00000002U);

	if(blinkActive){
//		// wait for blink thread to turn off
//		//TODO: make safer by making a break condition
//		osSemaphoreAcquire(locNotifyHandle, osWaitForever);

		// enable blink thread
		osThreadFlagsSet(blinkTaskHandle, 0x00000001U);
	}

}


void enqueue_pulse(Input *self, uint16_t start_time, uint16_t len){
	Pulse p_in = {self->input_idx_, start_time, len};
	osMessageQueuePut(pulseQueueHandle, (const void *) &p_in, 0U, 0);
//	count = osMessageQueueGetCount(pulseQueueHandle);
}
