#include "watchdogTask.h"
#include "driver_BNO080.h"
#include "math.h"
#include "stm32wbxx_hal.h"
#include "main.h"
#include "captivate_config.h"
#include "String.h"

#define osFlagsErrorTimeout   0xFFFFFFFEU

void watchdogTask(void *argument){

	uint32_t value = 0;
	osThreadFlagsWait(0x00000001U, osFlagsWaitAny, osWaitForever);
	osDelay(10000);

	while(1){
		if(osThreadFlagsWait(0x00000001U, osFlagsWaitAny, pdMS_TO_TICKS(5000)) == osFlagsErrorTimeout){
			taskENTER_CRITICAL();
			NVIC_SystemReset();
			taskEXIT_CRITICAL();
		}
		osThreadFlagsClear(0x00000001U);
		osDelay(20000);
	}
}
