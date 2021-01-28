#pragma once
#include "stm32wbxx_hal.h"
#include "messages.h"

// Reference to a pair of DataFrameBit-s

typedef struct CyclePhaseClassifier{
	uint32_t prev_full_cycle_idx_;
	uint32_t phase_history_;

	int fix_level_;
	uint32_t phase_shift_;

	float pulse_base_len_;

	float average_error_;
} CyclePhaseClassifier;

void process_pulse_lengths(CyclePhaseClassifier * self, uint32_t cycle_idx, const uint16_t *pulse_lens);
float expected_pulse_len(CyclePhaseClassifier * self, uint8_t skip, uint8_t data, uint8_t axis);
int get_phase(CyclePhaseClassifier * self, uint32_t cycle_idx);
void reset(CyclePhaseClassifier * self);
