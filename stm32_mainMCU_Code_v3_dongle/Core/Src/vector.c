#include "vector.h"

/*
void vector_init(vector *self)
{
	self->capacity = VECTOR_INIT_CAPACITY;
	self->count = 0;
    self->items = malloc(sizeof(void *) * self->capacity);
}


void vector_resize(vector *self, int capacity)
{
    #ifdef DEBUG_ON
    printf("vector_resize: %d to %d\n", self->capacity, capacity);
    #endif

    void **items = realloc(self->items, sizeof(void *) * capacity);
    if (items) {
        self->items = items;
        self->capacity = capacity;
    }
}

void vector_push(vector *self, void *item)
{
    if (self->capacity == self->count)
        vector_resize(self, self->capacity * 2);
    self->items[self->count++] = item;
}

void vector_set(vector *self, int index, void *item)
{
    if (index >= 0 && index < self->count)
        self->items[index] = item;
}

void *vector_get(vector *self, int index)
{
    if (index >= 0 && index < self->count)
        return self->items[index];
    return NULL;
}

void vector_delete(vector *self, int index)
{
    if (index < 0 || index >= self->count)
        return;

    self->items[index] = NULL;

    for (int i = index; i < self->count - 1; i++) {
        self->items[i] = self->items[i + 1];
        self->items[i + 1] = NULL;
    }

    self->count--;

    if (self->count > 0 && self->count == self->capacity / 4)
        vector_resize(self, self->capacity / 2);
}

void * vector_pop(vector *self){
	void * last = self->items[self->count - 1];
    self->items[self->count - 1] = NULL;
    self->count--;
    if (self->count > 0 && self->count == self->capacity / 4)
        vector_resize(self, self->capacity / 2);
    return last;
}

void vector_free(vector *self)
{
    free(self->items);
}
*/

