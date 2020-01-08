#ifndef __thermopile_H
#define __thermopile_H

#include "stdint.h"
#include "stdint.h"
#include "cmsis_os2.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define NUM_THERM_SAMPLES	5

struct thermopileData{
	uint16_t thermopile;
	uint16_t thermistor;
	uint32_t tick_ms;
};

struct thermopilePackagedData{
	struct thermopileData temple[5];
	struct thermopileData nose[5];
};

//struct thermopileData{
//	uint16_t thermopile;
//	uint16_t thermistor;
//};
//
struct adcThermopileData{
	uint16_t thermopile;
	uint16_t temple_thermistor;
	uint16_t nose_thermistor;
};

//struct thermopilePackagedData{
//
//	struct thermopileData temple[5];
//	struct thermopileData nose[5];
//
//	uint32_t			tick_ms;
//};

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
