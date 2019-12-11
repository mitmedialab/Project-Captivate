#ifndef __thermopile_H
#define __thermopile_H

#include "stdint.h"
#include "stdint.h"
#include "cmsis_os2.h"


#ifdef __cplusplus
 extern "C" {
#endif

struct thermopileData{
	uint16_t			templeData[20];
	uint32_t			temple_tick_ms;

	uint16_t			noseData[20];
	uint32_t			nose_tick_ms;

};

typedef enum{
	temple = 0,
	nose = 1
} sensorChoice;


//osThreadId_t thermopileTaskHandle;
osThreadId_t thermopileTaskHandle;
osMessageQueueId_t thermMsgQueueHandle;

void ThermopileTask(void *argument);
void Setup_LMP91051(void);
void SwitchTemperatureSensor(sensorChoice sense);

#ifdef __cplusplus
 }
#endif
#endif
