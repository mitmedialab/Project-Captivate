#ifndef TOUCH_DETECTOR_H
#define TOUCH_DETECTOR_H

#pragma once
#include "stm32wbxx_hal.h"
#include "tsc.h"

#define TOUCH_TIMEOUT	1000
#define TAP_MAX_THRESHOLD	200
#define TAP_MIN_THRESHOLD	40
#define DEBOUNCE_TIME	30
#define MIN_SWIPE_GAP	10

//.1 MF is ideal for capacitor

enum TouchType {
	None = 0,
	SwipeForward = 1,
	SwipeBackward = 2,
	FrontRelease = 3,
	BackRelease = 4,
	FrontHold = 5,
	BackHold = 6,
	BothHold = 7,
	BothRelease = 8
};

enum TouchState {
	Idle = 0,
	OnePressed = 1,
	TwoPressed = 2,
	OneReleasedNoPressed = 3,
	FirstReleasedSecondPressed = 4,
	SecondReleasedFirstPressed = 5
};

typedef struct Touch {
	uint8_t cap_idx;
	uint32_t start_t;	//ms
	uint32_t end_t;	//ms
} Touch;

typedef struct TouchDetector {
	Touch touches[2];
	uint8_t touch_state;
} TouchDetector;

typedef struct Debouncer {
	uint32_t start_t;
	uint8_t input;
	uint8_t clean_out;
} Debouncer;

void process_touches(TouchDetector *self, Debouncer *dbs, uint32_t cur_time);
void debounce(Debouncer *self, uint8_t noisy_in, uint32_t cur_time);
void HAL_TSC_ConvCpltCallback(TSC_HandleTypeDef *htsc);
void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp);
void touchSensingStart(void);
void touchSensingStop(void);

#endif
