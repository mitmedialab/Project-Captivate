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
//#include "UART_Print.h"

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
			//VIVEVars v;
			//v.pos[0] = 4.0;
			//v.pos[1] = 2.0;
			//v.pos[3] = 0.0;
			//osMessageQueuePut(viveQueue, (void *) &v, NULL, 0);
		}
	}

}

VIVEVars vive_vars;
//void MasterTask(void *argument){
//	while(1){
//		osThreadFlagsSet(pulseHandlerTaskHandle, 0x00000001U);
//		uint32_t m_count = 0;
//		while(1){
//			osMessageQueueGet(viveQueue, (void *) &vive_vars, NULL, osWaitForever);
//
//			if(m_count == 5){
//				UART_Print_3DCoords(vive_vars.pos);
//				osThreadFlagsSet(pulseHandlerTaskHandle, 0x00000002U);
//				osDelay(10);
//				break;
//			}
//			m_count += 1;
//		}
//	}
//}

struct LogMessage statusMessage;


void get3D_location(void *arguments){

	uint8_t blinkActive	= 0;
//	uint32_t startTime = HAL_GetTick();
//	volatile uint32_t deltaTime;

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
//	osSemaphoreAcquire (locCompleteHandle, osWaitForever);
	osSemaphoreAcquire (locCompleteHandle, GET_3D_LOC_TIMEOUT);

	osDelay(osWaitForever);

	// release I2C handle
	osSemaphoreRelease(messageI2C_LockHandle);

	// turn off 3D localization
	osThreadFlagsSet(pulseTaskHandle, 0x00000002U);

//	// empty queue
//	osMessageQueueReset(viveQueueHandle);

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
	osMessageQueuePut(pulseQueueHandle, (const void *) &p_in, NULL, 0);
	count = osMessageQueueGetCount(pulseQueueHandle);
}
