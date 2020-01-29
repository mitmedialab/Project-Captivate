/*
 * pulse_processor.c
 *
 *  Created on: Nov 14, 2019
 *      Author: giand
 */

#include "pulse_processor.h"
#include <math.h>
//#include "UART_Print.h"

// Pulse classification parameters.
#define MIN_SHORT_PULSE_LEN 	2 //usec
#define MIN_LONG_PULSE_LEN 		40 //usec
#define MAX_LONG_PULSE_LEN 		300 //usec

#define LONG_PULSE_STARTS_ACCEPTED_RANGE 	30  //usec
const uint16_t long_pulse_starts[NUM_BASE_STATIONS] = {0, 410};

#define CYCLE_PERIOD 			8333 //Total len of 1 cycle (usec)
#define ANGLE_CENTER_LEN 		4000 //usec

#define SHORT_PULSE_MIN_TIME 	ANGLE_CENTER_LEN - (CYCLE_PERIOD / 3)
#define SHORT_PULSE_MAX_TIME 	ANGLE_CENTER_LEN + (CYCLE_PERIOD / 3)
#define CYCLE_PROCESSING_POINT 	SHORT_PULSE_MAX_TIME + 100 // time from start of the cycle.


enum CycleFixLevels {
	kCycleFixNone = 0,
	kCycleFixCandidate = 1,
	kCycleFixAcquired = 5,
	kCycleFixMax = 10
};

void vector_push(vector *self, Pulse *item)
{
    if (self->count < MAX_CAPACITY){
    	self->items[self->count++] = *item;
    }
}

Pulse vector_pop(vector *self){
	if(self->count > 0){
		Pulse last = self->items[self->count - 1];
		self->count--;
		return last;
	}
	return self->items[0];
}

Pulse vector_get(vector *self, int index)
{
    if (index >= 0 && index < self->count){
    	return self->items[index];
    }
    return self->items[0];
}

void vector_clear(vector *self){
	self->count = 0;
}

void _PulseProcessor(PulseProcessor *self, uint32_t num_inputs){
	self->num_inputs_ = num_inputs;
	self->cycle_fix_level_ = 0;
	self->cycle_idx_ = 0;


	for(int i = 0; i < NUM_BASE_STATIONS; i++){
		vector_clear(&self->cycle_long_pulses_[i]);
	}
	vector_clear(&self->cycle_short_pulses_);
	vector_clear(&self->unclassified_long_pulses_);
	self->time_from_last_long_pulse_ = 0;
}

void consume_pulse(PulseProcessor *self, Pulse* p){
	//vector_push(&debug_ar, p);
	if(p->pulse_len >= MAX_LONG_PULSE_LEN){
		//Ignore very long pulses
	}
	else if(p->pulse_len >= MIN_LONG_PULSE_LEN){ // Long pulse - likely sync pulse
		process_long_pulse(self, p);
	}
	else{	//Short pulse -likely laser sweep
		process_short_pulse(self, p);
	}
}

void process_long_pulse(PulseProcessor *self, Pulse* p){
	if(self->cycle_fix_level_ == kCycleFixNone){
        // Bootstrap mode. We keep the previous long pulse in unclassified_long_pulses_ vector.
        // With this algorithm 2 base stations needed for a fix. We search for a situation where the last pulse was
        // second in last cycle, which means (8333-400) us difference in start time.
		if(self->unclassified_long_pulses_.count > 0){
			Pulse last_long_pulse = vector_pop(&self->unclassified_long_pulses_);

			self->time_from_last_long_pulse_ = p->start_time - last_long_pulse.start_time;


			int delta = self->time_from_last_long_pulse_ - (CYCLE_PERIOD - long_pulse_starts[1]);
			if(-LONG_PULSE_STARTS_ACCEPTED_RANGE <= delta && delta < LONG_PULSE_STARTS_ACCEPTED_RANGE){
                // Found candidate first pulse.
                self->cycle_fix_level_ = kCycleFixCandidate;
                self->cycle_start_time_ = p->start_time;
                self->cycle_idx_ = 0;

                reset(&self->phase_classifier_);

                reset_cycle_long_pulses(self);
                reset_unclassified_long_pulses(self);
                reset_cycle_short_pulses(self);
            }
			else{
				reset_unclassified_long_pulses(self);
			}
		}
	}

    // Put the pulse into either one of two buckets, or keep it as unclassified.
    uint8_t pulse_classified = 0;
    if (self->cycle_fix_level_ >= kCycleFixCandidate) {
        // Put pulse into one of two buckets by start time.
    	uint16_t time_from_cycle_start = p->start_time - self->cycle_start_time_;
        for (int i = 0; i < NUM_BASE_STATIONS; i++) {
        	int16_t delta = time_from_cycle_start - long_pulse_starts[i];
        	if(-LONG_PULSE_STARTS_ACCEPTED_RANGE <= delta && delta < LONG_PULSE_STARTS_ACCEPTED_RANGE) {
                vector_push(&self->cycle_long_pulses_[i], p);
                pulse_classified = 1;
                break;
            }
        }
    }
    if (!pulse_classified){
    	vector_push(&self->unclassified_long_pulses_, p);
    }
}

void process_short_pulse(PulseProcessor *self, Pulse *p){
	if (self->cycle_fix_level_ >= kCycleFixCandidate && p->input_idx < self->num_inputs_) {
	    // TODO: Filter out pulses outside of current cycle.
		vector_push(&self->cycle_short_pulses_, p);

		//We can process the cycle now
		process_cycle_fix(self);
	}
}

void process_cycle_fix(PulseProcessor *self) {
    int pulse_start_corrections[NUM_BASE_STATIONS] = {};
    uint16_t pulse_lens[NUM_BASE_STATIONS] = {};
    // Check if we have long pulses from at least one base station.
    if (self->cycle_long_pulses_[0].count > 0 || self->cycle_long_pulses_[1].count > 0) {
        // Increase fix level if we have pulses from both stations.
        if (self->cycle_fix_level_ < kCycleFixMax && self->cycle_long_pulses_[0].count > 0 && self->cycle_long_pulses_[1].count > 0){
        	self->cycle_fix_level_++;
        }

        // Average out long pulse lengths and start times for each base station across sensors.
        // pulse_start_corrections is the delta between actual start time and expected start time.
        // TODO: Take into account previous cycles as well, i.e. adjust slowly.
        for (int b = 0; b < NUM_BASE_STATIONS; b++){
        	uint32_t num_pulses = self->cycle_long_pulses_[b].count;
            if (num_pulses) {
            	uint16_t expected_start_time = self->cycle_start_time_ + long_pulse_starts[b];
                for (uint32_t i = 0; i < num_pulses; i++) {
                    const Pulse pulse = vector_get(&self->cycle_long_pulses_[b], i);
                    pulse_start_corrections[b] += pulse.start_time - expected_start_time;
                    pulse_lens[b] += pulse.pulse_len;
                }
                if (num_pulses > 1) {
                    pulse_start_corrections[b] /= num_pulses;
                    pulse_lens[b] /= num_pulses;
                }
            }
        }
        // Send pulse lengths to phase classifier.
        process_pulse_lengths(&self->phase_classifier_, self->cycle_idx_, pulse_lens);
    }
    else {
        // No long pulses this cycle. We can survive several of such cycles, but our confidence in timing sinks.
        //self->cycle_fix_level_--;
    	if(self->cycle_fix_level_ > 0){
    		self->cycle_fix_level_--;
    	}
    }

    // Given the cycle phase, we can put the angle timings to a correct bucket.

    int cycle_phase = get_phase(&self->phase_classifier_, self->cycle_idx_);
    if (cycle_phase >= 0) {
    	//UART_Print_int32_t(cycle_phase);
        // From (potentially several) short pulses for the same input, we choose the longest one.
        Pulse *short_pulses[MAX_NUM_INPUTS] = {};
        uint16_t short_pulse_timings[MAX_NUM_INPUTS] = {};

        //Base 0 is emitting during cycle phases 0 and 1. Base 1 is emitting during cycle phases 2 and 3.
        uint32_t emitting_base = cycle_phase >> 1;
        uint16_t base_pulse_start = self->cycle_start_time_ + long_pulse_starts[emitting_base] + pulse_start_corrections[emitting_base];
        for (uint32_t i = 0; i < self->cycle_short_pulses_.count; i++) {
            Pulse p = vector_get(&self->cycle_short_pulses_, i);
            uint32_t input_idx = p.input_idx;

            // To get better precision, we calculate pulse timing based on the long pulse from the same base station.
            uint16_t pulse_timing = p.start_time + p.pulse_len / 2 - base_pulse_start;

            // Get longest laser pulse.
            if (SHORT_PULSE_MIN_TIME < pulse_timing && pulse_timing < SHORT_PULSE_MAX_TIME){
            	if (!short_pulses[input_idx] || short_pulses[input_idx]->pulse_len < p.pulse_len) {
					short_pulses[input_idx] = &p;
					short_pulse_timings[input_idx] = pulse_timing;
            	}
            }
        }

        // Calculate the angles for inputs where we saw short pulses.
        for (uint32_t i = 0; i < self->num_inputs_; i++){
            if (short_pulses[i]) {
                SensorAngles *angles = &self->angles_frame_.sensors[i];
                angles->angles[cycle_phase] = (float)M_PI * ((int)(short_pulse_timings[i] - ANGLE_CENTER_LEN)) / (float)CYCLE_PERIOD;
                angles->updated_cycles[cycle_phase] = self->cycle_idx_;
                //UART_Print_float(angles->angles[0]);
            }
        }
    }

    // Send the data down the pipeline every 4th cycle (30Hz). Can be increased to 120Hz if needed.

    if ((cycle_phase >= 0) ? (cycle_phase == 3) : (self->cycle_idx_ % 4 == 0)) {
        self->angles_frame_.time = self->cycle_start_time_;
        self->angles_frame_.fix_level = (cycle_phase >= 0 && self->cycle_fix_level_ >= kCycleFixAcquired)
                                        ? kCycleSynced : kCycleSyncing;
        self->angles_frame_.cycle_idx = self->cycle_idx_;
        self->angles_frame_.phase_id = cycle_phase;

        //UART_Print_float(self->angles_frame_.sensors[0].angles[0]);
        consume_angles(self->next, &self->angles_frame_);

    }

    // Prepare for the next cycle.
    reset_cycle_long_pulses(self);
    reset_unclassified_long_pulses(self);
    reset_cycle_short_pulses(self);

    self->cycle_start_time_ += CYCLE_PERIOD + pulse_start_corrections[0];
    self->cycle_idx_++;
}

void reset_cycle_long_pulses(PulseProcessor *self){
	for (int i = 0; i < NUM_BASE_STATIONS; i++){
	    vector_clear(&self->cycle_long_pulses_[i]);
	}
}

void reset_unclassified_long_pulses(PulseProcessor *self){
	vector_clear(&self->unclassified_long_pulses_);
}

void reset_cycle_short_pulses(PulseProcessor *self){
	vector_clear(&self->cycle_short_pulses_);
}

/*
void do_work_pulse_processor(PulseProcessor *self, uint16_t cur_time) {
    if (self->cycle_fix_level_ >= kCycleFixCandidate) {
    	//CYCLE_PROCESSING_POINT = 6877
    	uint16_t temp;
    	if(((uint16_t)(cur_time - self->cycle_start_time_)) < ((uint16_t)(self->cycle_start_time_ - cur_time))){
    		temp = (uint16_t)(cur_time - self->cycle_start_time_);
    	}
    	else{
    		temp = (uint16_t)(self->cycle_start_time_ - cur_time);
    	}
    	if(temp > ((uint16_t) CYCLE_PROCESSING_POINT)){
    		//UART_Print_uint16_t(self->cycle_start_time_);
    		process_cycle_fix(self, cur_time);
    	}

    } else {
    	// No fix
    }
}
*/
