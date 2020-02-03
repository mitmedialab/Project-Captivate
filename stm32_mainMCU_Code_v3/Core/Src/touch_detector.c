#include "touch_detector.h"

void process_touches(TouchDetector *self, Debouncer *dbs, uint32_t cur_time){
    if(self->touch_state == Idle){
    	if(dbs[0].clean_out){
    		self->touch_state = OnePressed;
    		self->touches[0].cap_idx = 0;
    		self->touches[0].start_t = cur_time;
    	}
    	else if(dbs[1].clean_out){
    		self->touch_state = OnePressed;
    		self->touches[0].cap_idx = 1;
    		self->touches[0].start_t = cur_time;
    	}
    }
    else if(self->touch_state == OnePressed){
    	if(dbs[1 - self->touches[0].cap_idx].clean_out){
    		self->touch_state = TwoPressed;
    		self->touches[1].cap_idx = 1 - self->touches[0].cap_idx;
    		self->touches[1].start_t = cur_time;
    	}
    	else if(!dbs[self->touches[0].cap_idx].clean_out){
    		self->touch_state = OneReleasedNoPressed;
    		self->touches[0].end_t = cur_time;

    	}
    }
    else if(self->touch_state == TwoPressed){
    	if(!dbs[self->touches[0].cap_idx].clean_out){
    		self->touch_state = FirstReleasedSecondPressed;
    		self->touches[0].end_t = cur_time;
    	}
    	else if(!dbs[self->touches[1].cap_idx].clean_out){
    		self->touch_state = SecondReleasedFirstPressed;
    		self->touches[1].end_t = cur_time;
    	}
    }
    else if(self->touch_state == OneReleasedNoPressed){
    	if(dbs[1 - self->touches[0].cap_idx].clean_out){
    		self->touch_state = FirstReleasedSecondPressed;
    		self->touches[1].cap_idx = (1 - self->touches[0].cap_idx);
    		self->touches[1].start_t = cur_time;
    	}
    	else if(cur_time - self->touches[0].end_t > MIN_SWIPE_GAP){
    		uint32_t touch_duration = self->touches[0].end_t - self->touches[0].start_t;
			if(TAP_MIN_THRESHOLD < touch_duration && touch_duration < TAP_MAX_THRESHOLD){
				//Register a tap
				enum TouchType action = Tap;
			}
			self->touch_state = Idle;
    	}
    }
    else if(self->touch_state == FirstReleasedSecondPressed){
    	if(!dbs[self->touches[1].cap_idx].clean_out){
    		self->touch_state = Idle;
    		self->touches[1].end_t = cur_time;
    		enum TouchType action = None;
    		uint32_t touches_start_diff = self->touches[1].start_t - self->touches[0].start_t;
    		uint32_t touches_end_diff = self->touches[1].end_t - self->touches[0].end_t;
    		uint32_t touch_duration = self->touches[1].end_t - self->touches[0].start_t;

    		//If touch start and touch end differences were small, then it was likely just a tap
    		if((touches_start_diff <= TAP_MIN_THRESHOLD) && (touches_end_diff <= TAP_MIN_THRESHOLD) &&
    				(TAP_MIN_THRESHOLD < touch_duration) && (touch_duration < TAP_MAX_THRESHOLD)){
    			//Register a tap
    			action = Tap;
    		}
    		//Otherwise, it was a swipe. Might have to adjust these thresholds to allow swipe detection
    		else if(touches_start_diff > TAP_MIN_THRESHOLD && touches_end_diff > TAP_MIN_THRESHOLD){
    			//Register a swipe
    			action = (self->touches[0].cap_idx << 1) | self->touches[1].cap_idx;
    		}
    		self->touch_state = Idle;
    	}
    }
    else if(self->touch_state == SecondReleasedFirstPressed){
    	if(!dbs[self->touches[0].cap_idx].clean_out){
    	    self->touch_state = Idle;
    	    self->touches[0].end_t = cur_time;

    	    uint32_t touches_start_diff = self->touches[1].start_t - self->touches[0].start_t;
    	    uint32_t touches_end_diff = self->touches[0].end_t - self->touches[1].end_t;
    	    uint32_t touch_duration = self->touches[0].end_t - self->touches[0].start_t;
    	    enum TouchType action = None;
    	    //If touch start and touch end differences were small, then it was likely just a tap
    	    if((touches_start_diff <= TAP_MIN_THRESHOLD) && (touches_end_diff <= TAP_MIN_THRESHOLD) &&
    	    		(TAP_MIN_THRESHOLD < touch_duration) && (touch_duration < TAP_MAX_THRESHOLD)){
    	    	//Register a tap
    	    	action = Tap;
    	    }
    	    self->touch_state = Idle;
    	}
    }
}

void debounce(Debouncer *self, uint8_t noisy_in, uint32_t cur_time){
	if(noisy_in != self->input){
		self->input = noisy_in;
		self->start_t = cur_time;
	}
	else if(cur_time - self->start_t > DEBOUNCE_TIME){
		self->clean_out = self->input;
	}
}
