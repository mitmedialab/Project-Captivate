
/**
 ******************************************************************************
 * File Name           : template.c
  * Description        : Header for Lights.
  ******************************************************************************

  *
  ******************************************************************************
 */


/* includes -----------------------------------------------------------*/
#include "camera_detector.h"
#include "stdint.h"
#include <string.h>
//#include "thermopile.h"
#include "adc.h"
#include "math.h"
#include "lp5523.h"
#include "stdlib.h"
#include "i2c.h"
/* typedef -----------------------------------------------------------*/


/* defines -----------------------------------------------------------*/
#define TOTAL_DIODE_SAMPLES		1000
#define HALF_DIODE_SAMPLES		500

#define SAMPLE_CNT				20

#define THRESHOLD				250

#define MAX_BRIGHTNESS			255

#define I2C_TIMEOUT				100
/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/
volatile uint32_t diodeSamples[TOTAL_DIODE_SAMPLES] = {0};
uint32_t* diodeSamplesPtr;

union ColorComplex colorSet;

uint8_t led_left_PWM[9] = {0};
uint8_t led_right_PWM[9] = {0};
//uint8_t led_current[9] = {100, 100, 100, 100, 100, 100, 100, 100, 100};

volatile uint16_t ADCValue[3] = {0};

double diode_left = 0;
double diode_right = 0;
double diode_center = 0;

double vec_x;
double vec_y;

double theta;

uint8_t left_brightness;
uint8_t right_brightness;


/* Functions Definition ------------------------------------------------------*/
void cameraDetectionTask(void *argument){

//	HAL_Delay(100);
	uint32_t evt = 0;
	setup_LP5523(LIS3DH_LEFT_ADDRESS);
	setup_LP5523(LIS3DH_RIGHT_ADDRESS);



	while(1){
//		evt = osThreadFlagsWait (0x00000001U, osFlagsWaitAny, osWaitForever);
		evt = 0x00000001U;
		// if signal was received successfully, start blink task
		if (evt == 0x00000001U)  {
			// wait for start flag

		// start DMA to start sampling 3 channels
//		HAL_ADC_Start_DMA(&hadc1, (uint32_t *) diodeSamples, sizeof(diodeSamples));
//		HAL_ADC_Start_IT(&hadc1);
		// start timer to trigger DMA
//		volatile uint32_t ADCValue = 0;
			while(1){
				// reset
				diode_left = 0;
				diode_right = 0;
				diode_center = 0;

				// grab SAMPLE_CNT number of samples and average it
				for(int i=0; i<SAMPLE_CNT; i++){
					HAL_ADC_Start(&hadc1);

					HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
					ADCValue[0] = HAL_ADC_GetValue(&hadc1);

					HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
					ADCValue[1] = HAL_ADC_GetValue(&hadc1);
//					ADCValue[1] = 0;

					HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
					ADCValue[2] = HAL_ADC_GetValue(&hadc1);

					if(ADCValue[0] >= THRESHOLD){
						diode_right += ADCValue[0];
					}

					if(ADCValue[1] >= THRESHOLD){
						diode_left += ADCValue[1];
					}

					if(ADCValue[2] >= THRESHOLD){
						diode_center += ADCValue[2];
					}

					HAL_ADC_Stop(&hadc1);
					HAL_Delay(1);
				}

				// get average
				diode_right = diode_right / SAMPLE_CNT;
				diode_left = diode_left / SAMPLE_CNT;
				diode_center = diode_center / SAMPLE_CNT;

				// treat values as vector
				vec_x = diode_right - diode_left;
				vec_y = diode_center;

				// get theta of vector if vector exists
				if (vec_y > THRESHOLD || abs(vec_x) > THRESHOLD){

					// avoid dividing by zero if camera seen only on sides
					if(vec_y == 0){
						if (vec_x > 0){
							theta = 0;
						}
						else{
							theta = M_PI;
						}
					}
					// if directly sensed by front diode
					else if(vec_x == 0){
						theta = M_PI_2;
					}
					else if(vec_x > 0){
						theta = tanh(vec_y/vec_x);
					}else{
						theta = M_PI - tanh(vec_y/(-1*vec_x));
					}
				}
				else{
					// turn off lights
					colorSet.loc.left_side_b = 0;
					colorSet.loc.right_side_b = 0;

					colorSet.loc.left_top_b = 0;
					colorSet.loc.right_top_b = 0;

					colorSet.loc.left_front_g = 0;
					colorSet.loc.right_front_g= 0;

//					FrontLightsSet(&colorSet);

					memcpy(led_left_PWM, colorSet.color, 9);
					memcpy(led_right_PWM, &(colorSet.color[9]), 9);

					while(HAL_I2C_Mem_Write(&hi2c1, LIS3DH_LEFT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_left_PWM, 9, I2C_TIMEOUT) != HAL_OK);
					while(HAL_I2C_Mem_Write(&hi2c1, LIS3DH_RIGHT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_right_PWM, 9, I2C_TIMEOUT) != HAL_OK);

					continue;
				}

				// calculate brightness
				if( (theta >= 0) && (theta <= M_PI_2) ){
					right_brightness = MAX_BRIGHTNESS;
					left_brightness = ((float) MAX_BRIGHTNESS) * sin(theta);
				}

				// calculate right brightness
				else{
//					theta = (theta * -1) + M_PI_2; // compensate for arctan negative region
					theta = M_PI_2 + theta;
					left_brightness = MAX_BRIGHTNESS;
					right_brightness = ((float) MAX_BRIGHTNESS ) * sin(theta);
				}

				colorSet.loc.left_side_b = left_brightness;
				colorSet.loc.left_front_g = left_brightness;
				colorSet.loc.left_top_b = left_brightness;

				colorSet.loc.right_side_b = right_brightness;
				colorSet.loc.right_front_g = right_brightness;
				colorSet.loc.right_top_b = right_brightness;


//				FrontLightsSet(&colorSet);
//
				memcpy(led_left_PWM, colorSet.color, 9);
				memcpy(led_right_PWM, &(colorSet.color[9]), 9);

				while(HAL_I2C_Mem_Write(&hi2c1, LIS3DH_LEFT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_left_PWM, 9, I2C_TIMEOUT) != HAL_OK);
				while(HAL_I2C_Mem_Write(&hi2c1, LIS3DH_RIGHT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_right_PWM, 9, I2C_TIMEOUT) != HAL_OK);

				//HAL_Delay(200);

//				HAL_ADC_Start(&hadc1);
//				HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
//				ADCValue = HAL_ADC_GetValue(&hadc1);
//				HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
//				ADCValue = HAL_ADC_GetValue(&hadc1);

				// wait for DMA to finish
//				evt = osThreadFlagsWait (0x00000004U, osFlagsWaitAny, osWaitForever);
//				HAL_ADC_Start_DMA(&hadc1, (uint32_t *) diodeSamples, sizeof(diodeSamples));
				// convert ADC values to respected light triggering
				//convertADC_ToColor(diodeSamplesPtr, &colorSet);

				// set LEDs
//				FrontLightsSet(&colorSet);

				// send LED config and Rotational Vector to other glasses

			}

			// stop timer and put thread in idle if signal was reset
//			evt = osThreadFlagsWait (0x00000002U, osFlagsWaitAny, 0);
//			if( (evt & 0x00000002U) == 0x00000002U){
//
//				HAL_ADC_Stop_DMA(&hadc1);
////					while(HAL_ADC_Stop(&hadc1) != HAL_OK)
////					HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
////					HAL_TIM_Base_Stop(&htim16);
////					HAL_TIM_Base_Stop(&htim2);
////				HAL_TIM_Base_Stop_IT(&htim2);
//				osThreadFlagsClear(0x0000000EU);
//				break;
//			}
		}
	}
}

//void convertADC_ToColor(uint16_t* diodeSamples, union ColorComplex *setColors){
//
////
////	uint8_t left_front_b;
////	        uint8_t left_front_g;
////	        uint8_t left_top_b;
////	        uint8_t left_top_g;
////	        uint8_t left_side_b;
////	        uint8_t left_side_g;
////	        uint8_t left_front_r;
////			uint8_t left_top_r;
////			uint8_t left_side_r;
////
////			uint8_t right_front_b;
////			uint8_t right_front_g;
////			uint8_t right_top_b;
////			uint8_t right_top_g;
////			uint8_t right_side_b;
////			uint8_t right_side_g;
////			uint8_t right_front_r;
////			uint8_t right_top_r;
////			uint8_t right_side_r;
//}


volatile uint8_t complete = 0;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
//	HAL_ADC_Stop_DMA(&hadc1);
	complete++;
//	HAL_ADC_Stop_DMA(&hadc1);
//
//	// notify ThermopileTask that conversion is complete
//	diodeSamplesPtr = &(diodeSamples[HALF_DIODE_SAMPLES]);
//	osThreadFlagsSet(cameraDetectionTaskHandle, 0x00000004U);

}

volatile uint8_t half = 0;
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
	half++;
//	memcpy(blinkMsgBuffer_1.data, &(blink_buffer), 100);
//	blinkMsgBuffer_1.tick_ms = HAL_GetTick();
//	blink_ptr = &blink_buffer;
//	osThreadFlagsSet(blinkTaskHandle, 0x00000004U);

//	diodeSamplesPtr = diodeSamples;
//	osThreadFlagsSet(cameraDetectionTaskHandle, 0x00000004U);

}

volatile uint8_t i = 0;
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  i++;
}
/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
 * @brief Thread initialization.
 * @param  None
 * @retval None
 */

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
*************************************************************/
