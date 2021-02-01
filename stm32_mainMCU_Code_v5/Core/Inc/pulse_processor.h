#pragma once
#include "stm32wbxx_hal.h"
#include "messages.h"
#include "cycle_phase_classifier.h"
#include "geometry.h"
//#include "input.h"

#define MAX_CAPACITY	16
typedef struct vector {
	Pulse items[MAX_CAPACITY];
	uint32_t count;
} vector;

void vector_push(vector *self, Pulse *item);
Pulse vector_pop(vector *self);
Pulse vector_get(vector *self, int index);
void vector_clear(vector *self);

typedef struct PulseProcessor{
	uint32_t num_inputs_;

	// Fix level - increased when everything's right (up to a limit); decreases on errors. See CycleFixLevels
	uint32_t cycle_fix_level_;

	// Current cycle params
	uint16_t cycle_start_time_;  // Current cycle start time.  TODO: We should track cycle period more precisely.
	uint32_t cycle_idx_;          // Index of current cycle.

	// Classified pulses for current cycle: long (x2, by base stations), short, unclassified.
	vector cycle_long_pulses_[NUM_BASE_STATIONS];
	vector cycle_short_pulses_;
	vector unclassified_long_pulses_;

	// Phase classifier - helps determine which of the 4 cycles in we have now.
	CyclePhaseClassifier phase_classifier_;
	// Output data: angles.
	SensorAnglesFrame angles_frame_;
	uint16_t time_from_last_long_pulse_;

	GeometryBuilder *next;
} PulseProcessor;



void _PulseProcessor(PulseProcessor *self, uint32_t num_inputs);
void consume_pulse(PulseProcessor *self, Pulse* p);
void process_long_pulse(PulseProcessor *self, Pulse* p);
void process_short_pulse(PulseProcessor *self, Pulse *p);
void process_cycle_fix(PulseProcessor *self);
void reset_cycle_long_pulses(PulseProcessor *self);
void reset_unclassified_long_pulses(PulseProcessor *self);
void reset_cycle_short_pulses(PulseProcessor *self);
void do_work_pulse_processor(PulseProcessor *self, uint16_t cur_time);

