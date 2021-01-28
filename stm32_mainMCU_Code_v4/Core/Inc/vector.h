#pragma once
#include "stm32wbxx_hal.h"
#include <stdlib.h>

#define VECTOR_INIT_CAPACITY 4

typedef struct vector {
	void **items;
	uint32_t capacity;
	uint32_t count;
} vector;

void vector_init(vector *self);
void vector_resize(vector *self, int capacity);
void vector_push(vector *self, void *item);
void vector_set(vector *self, int index, void *item);
void *vector_get(vector *self, int index);
void vector_delete(vector *self, int index);
void * vector_pop(vector *self);
void vector_free(vector *self);

