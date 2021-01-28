#pragma once
#include "stm32wbxx_hal.h"
#include <stdlib.h>
#define MAX_SIZE 255

typedef struct CircularBuffer{
	void* elems_[256];
	uint32_t read_idx_;
	uint32_t write_idx_;
} CircularBuffer;


uint8_t empty(CircularBuffer *self);
uint32_t size(CircularBuffer *self);
void* front(CircularBuffer *self);
uint8_t pop_front(CircularBuffer *self);
void append_back(CircularBuffer *self, void * elem);
