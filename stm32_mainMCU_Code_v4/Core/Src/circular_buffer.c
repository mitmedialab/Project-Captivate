/*
 * circular_buffer.c
 *
 *  Created on: Dec 4, 2019
 *      Author: giand
 */
#include "circular_buffer.h"

uint8_t empty(CircularBuffer *self){
	return self->read_idx_ == self->write_idx_;
}

uint32_t size(CircularBuffer *self){
	return self->write_idx_ - self->read_idx_;
}

void* front(CircularBuffer *self){
	return self->elems_[self->read_idx_ & MAX_SIZE];
}

uint8_t pop_front(CircularBuffer *self){
	if(!empty(self)){
		//free(self->elems_[self->read_idx_ & MAX_SIZE]);
		self->elems_[self->read_idx_ & MAX_SIZE] = NULL;
		self->read_idx_ += 1;
		return 1;
	}
	else{
		return 0;
	}
}

void append_back(CircularBuffer *self, void * elem){
	self->elems_[self->write_idx_ & MAX_SIZE] = elem;
	self->write_idx_++;
}
