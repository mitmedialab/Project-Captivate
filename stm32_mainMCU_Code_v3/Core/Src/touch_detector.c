#include "touch_detector.h"
#include "captivate_config.h"
#include "tim.h"
#include "input.h"
#include "stm32wbxx_hal.h"
#include "tsc.h"


uint16_t timestamp = 0;
uint8_t cap_sensor = 0;
TSC_IOConfigTypeDef IoConfig;
Debouncer dbs[2] = {{0,0,0}, {0,0,0}};
volatile TouchDetector touch_detector;
extern Input input0;

void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp){
	timestamp = __HAL_TIM_GET_COUNTER(&htim16);

//	GPIO_PinState pin_state = HAL_GPIO_ReadPin(LH_SIG_GPIO_Port, LH_SIG_Pin);
	uint32_t pin_state = HAL_COMP_GetOutputLevel(hcomp);
	if (pin_state == COMP_OUTPUT_LEVEL_HIGH) {
		//Rising edge
		input0.rise_time_ = timestamp;
		input0.rise_valid_ = 1;
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	}
	else if (input0.rise_valid_ && pin_state == COMP_OUTPUT_LEVEL_LOW) {
		//Falling edge
		enqueue_pulse(&input0, input0.rise_time_, timestamp - input0.rise_time_);
		input0.rise_valid_ = 0;
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	}
}

void HAL_TSC_ConvCpltCallback(TSC_HandleTypeDef* htsc){
	  /*##-5- Discharge the touch-sensing IOs ####################################*/
	  HAL_TSC_IODischarge(htsc, ENABLE);
	  /* Note: a delay can be added here */

	  uint32_t uhTSCAcquisitionValue;
	  TSC_GroupStatusTypeDef status = HAL_TSC_GroupGetStatus(htsc, TSC_GROUP2_IDX);
	  uint32_t cur_time;
	  /*##-6- Check if the acquisition is correct (no max count) #################*/
	  if (status == TSC_GROUP_COMPLETED)
	  {
	    /*##-7- Read the acquisition value #######################################*/
	    uhTSCAcquisitionValue = HAL_TSC_GroupGetValue(htsc, TSC_GROUP2_IDX);
	    uint8_t touch;
	    if(cap_sensor == 0){
	    	touch = (uhTSCAcquisitionValue >= TSCx_TS1_MINTHRESHOLD) && (uhTSCAcquisitionValue <= TSCx_TS1_MAXTHRESHOLD);
	    }
	    else{
	    	touch = (uhTSCAcquisitionValue >= TSCx_TS2_MINTHRESHOLD) && (uhTSCAcquisitionValue <= TSCx_TS2_MAXTHRESHOLD);
	    }

	    cur_time = HAL_GetTick();
	    debounce(&dbs[cap_sensor], touch, cur_time);
	    process_touches(&touch_detector, dbs, cur_time);
	  }

	  //Switches between the two channels to be acquired
	  if (cap_sensor == 0)
	  {
	    IoConfig.ChannelIOs = TSC_GROUP2_IO4; /* TS4 touchkey */
	    cap_sensor = 1;
	  }
	  else
	  {
	    IoConfig.ChannelIOs = TSC_GROUP2_IO3; /* TS3 touchkey */
	    cap_sensor = 0;
	  }


	  if (HAL_TSC_IOConfig(htsc, &IoConfig) != HAL_OK)
	  {
	    /* Initialization Error */
	    Error_Handler();
	  }

	  /*##-9- Re-start the acquisition process ###################################*/
	  if (HAL_TSC_Start_IT(htsc) != HAL_OK)
	  {
	    /* Acquisition Error */
	    Error_Handler();
	  }
}

void touchSensingStart(void){
	  IoConfig.ChannelIOs  = TSC_GROUP2_IO3; /* Start with the first channel */
	  IoConfig.SamplingIOs = TSC_GROUP2_IO1;
	  IoConfig.ShieldIOs = 0;

	  if (HAL_TSC_IOConfig(&htsc, &IoConfig) != HAL_OK)
	  {
	    /* Initialization Error */
	    Error_Handler();
	  }

	  HAL_TSC_IODischarge(&htsc, ENABLE);
	  HAL_Delay(5); /* 1 ms is more than enough to discharge all capacitors */
	  if (HAL_TSC_Start_IT(&htsc) != HAL_OK)
		{
			/* Acquisition Error */
			Error_Handler();
		}
}

void touchSensingStop(void){
	  if (HAL_TSC_Stop_IT(&htsc) != HAL_OK)
		{
			/* Acquisition Error */
			Error_Handler();
		}
}

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
