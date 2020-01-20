/*
 * input.c
 *
 *  Created on: Dec 4, 2019
 *      Author: giand
 */
#include "input.h"
#include "UART_Print.h"

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
		HAL_NVIC_EnableIRQ(EXTI0_IRQn);
		_Input(&input0, 0);
		while(1){
			osMessageQueueGet (pulseQueue, (void *) &p, NULL, osWaitForever);
			consume_pulse(input0.next, &p);

			evt = osThreadFlagsWait(0x00000002U, osFlagsWaitAny, 0);
			if((evt & 0x00000002U) == 0x00000002U){
				//Stop interrupt
				HAL_NVIC_DisableIRQ(EXTI0_IRQn);
				osMessageQueueReset(pulseQueue);
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


void get3D_location(vive_vars *vive_vars){
	// start VIVE thread
	osThreadFlagsSet(pulseHandlerTaskHandle, 0x00000001U);

	// grab 3D location estimate
	osMessageQueueGet(viveQueue, (void *) vive_vars, NULL, osWaitForever);

	// TODO : average them and/or error correction

	// turn off 3D localization
	osThreadFlagsSet(pulseHandlerTaskHandle, 0x00000002U);
}


void enqueue_pulse(Input *self, uint16_t start_time, uint16_t len){
	Pulse p_in = {self->input_idx_, start_time, len};
	osMessageQueuePut(pulseQueue, (const void *) &p_in, NULL, 0);
	count = osMessageQueueGetCount(pulseQueue);
}
