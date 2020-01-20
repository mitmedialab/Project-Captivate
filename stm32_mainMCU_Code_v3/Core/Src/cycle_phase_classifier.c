#include "cycle_phase_classifier.h"
#include "UART_Print.h"

enum PhaseFixLevels {  // Unscoped enum because we use it more like set of constants.
    kPhaseFixNone = 0,
    kPhaseFixCandidate = 1,
    kPhaseFixAcquired = 4,
    kPhaseFixFinal = 16,
};

//length of pulse_lens is num_base_stations
void process_pulse_lengths(CyclePhaseClassifier * self, uint32_t cycle_idx, const uint16_t *pulse_lens){
	int cur_phase_id = -1;
	if(pulse_lens[0] > 0 && pulse_lens[1] > 0){
		uint8_t cur_more = pulse_lens[0] > pulse_lens[1];
        if (cycle_idx == self->prev_full_cycle_idx_ + 1) {
            // To get current phase, we use simple fact that in phases 0 and 1, first pulse is shorter than the second,
            // and in phases 2, 3 it is longer. This allows us to estimate current phase using comparison between
            // the pair of pulses in current cycle (cur_more) and the previous one.
            self->phase_history_ = (self->phase_history_ << 1) | cur_more;  // phase_history_ keeps a bit for each pulse comparison.
            static const char phases[4] = {1, 2, 0, 3};
            cur_phase_id = phases[self->phase_history_ & 0x3];  // 2 least significant bits give us enough info to get phase.
            //UART_Print_int32_t(cur_phase_id);
        }
        else {
            self->phase_history_ = cur_more;
        }
        self->prev_full_cycle_idx_ = cycle_idx;
	}

    // If we haven't achieved final fix yet, check the cur_phase_id is as expected.
    if (cur_phase_id >= 0 && self->fix_level_ < kPhaseFixFinal) {
        if (self->fix_level_ == kPhaseFixNone) {
            // Use current phase_id as the candidate.
            self->fix_level_ = kPhaseFixCandidate;
            self->phase_shift_ = (cur_phase_id - cycle_idx) & 0x3;

        } else {
            // Either add or remove confidence that the phase_shift_ is correct.
            int expected_phase_id = (cycle_idx + self->phase_shift_) & 0x3;
            self->fix_level_ += (cur_phase_id == expected_phase_id) ? +1 : -1;
        }
    }
}

float expected_pulse_len(CyclePhaseClassifier * self, uint8_t skip, uint8_t data, uint8_t axis) {
    // See https://github.com/nairol/LighthouseRedox/blob/master/docs/Light%20Emissions.md
    return self->pulse_base_len_ + (skip << 2 | data << 1 | axis) * 10.416f;
}

int get_phase(CyclePhaseClassifier * self, uint32_t cycle_idx) {
    if (self->fix_level_ >= kPhaseFixAcquired) {
        return (cycle_idx + self->phase_shift_) & 0x3;
    } else {
        return -1;
    }
}

void reset(CyclePhaseClassifier * self) {
    self->fix_level_ = kPhaseFixNone;
    self->prev_full_cycle_idx_ = -1;
    self->pulse_base_len_ = 62.5f;
    self->phase_history_ = 0;
}
