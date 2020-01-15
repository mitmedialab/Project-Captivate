/**
 ******************************************************************************
 * File Name           : template.c
  * Description        : Header for Lights.
  ******************************************************************************

  *
  ******************************************************************************
 */
#ifdef __cplusplus
extern "C" {
#endif

/* includes -----------------------------------------------------------*/
#include "lp5523.h"
//#include "stm32wbxx_hal_i2c.h"
#include "stm32wbxx_hal.h"
#include "i2c.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "string.h"
#include "config.h"
#include "master_thread.h"

#include "system_settings.h"
/* typedef -----------------------------------------------------------*/


/* defines -----------------------------------------------------------*/
#define I2C_HANDLE_TYPEDEF 	&hi2c1
#define I2C_TIMEOUT			-1


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/


/* Functions Definition ------------------------------------------------------*/


/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

uint8_t led_left_PWM[9] = {0};
uint8_t led_right_PWM[9] = {0};
uint8_t led_current[9] = {100, 100, 100, 100, 100, 100, 100, 100, 100};

//uint8_t led_current[9] = {200, 200, 200, 200, 200, 200, 200, 200, 200};


struct LightConfig
{
   uint8_t current[9];
   uint8_t intensity[9];
};

void setup_LP5523(uint8_t ADDR){
	uint8_t deviceAddress = ADDR << 1;
	uint8_t led_PWM[9] = {0};
	uint8_t packet;

	// enable chip
	osSemaphoreAcquire(messageI2C_LockSem, osWaitForever);
	packet = LP5525_CHIP_EN;
	while(HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_EN_CNTRL1_REG, 1, &packet, 1, I2C_TIMEOUT) != HAL_OK);

	// put charge-pump in auto-mode, serial auto increment, internal clock
	packet = CP_MODE_AUTO | EN_AUTO_INC | INT_CLK_EN;
	while(HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_MISC_REG, 1, &packet, 1, I2C_TIMEOUT) != HAL_OK);

	// set PWM level (0 to 255)
	while(HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_D1_PWM_REG, 1, led_PWM, 9, I2C_TIMEOUT) != HAL_OK);

	// set current control (0 to 25.5 mA) - step size is 100uA
	while(HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_D1_CURRENT_CTRL_REG, 1, led_current, 9, I2C_TIMEOUT) != HAL_OK);

	// enable logarithmic dimming
	packet = LOG_EN;
	uint8_t packet_array[9] = {packet,packet,packet,packet,packet,packet,packet,packet,packet};
	while(HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_D1_CNTRL_REG, 1, packet_array, 9, I2C_TIMEOUT) != HAL_OK);
	osSemaphoreRelease(messageI2C_LockSem);
}
//LP5523::LP5523(uint16_t DevAddress){
//	deviceAddress = DevAddress;
//}
///*!
// *  @brief  Setups the HW (reads coefficients values, etc.)
// *  @param  i2caddr
// *          i2c address (optional, fallback to default)
// *  @param  nWAI
// *          Who Am I register value - defaults to 0x33 (LIS3DH)
// *  @return true if successful
// */
//bool LP5523::begin(void) {
//
//	uint8_t packet;
//
//	// enable chip
//	packet = LP5525_CHIP_EN;
//	HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_EN_CNTRL1_REG, 1, &packet, 1, I2C_TIMEOUT);
//
//	// set PWM level (0 to 255)
//	packet = 100;
//	HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_D1_PWM_REG, 1, &packet, 1, I2C_TIMEOUT);
//
//	// set current control (0 to 25.5 mA) - step size is 100uA
//	packet = 255 * (5/25.5);
//	HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_D1_CURRENT_CTRL_REG, 1, &packet, 1, I2C_TIMEOUT);
//
//	// put charge-pump in auto-mode, serial auto increment, internal clock
//	packet = CP_MODE_AUTO | EN_AUTO_INC | INT_CLK_EN;
//	HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_MISC_REG, 1, &packet, 1, I2C_TIMEOUT);
//
//
//  return true;
//}

void FrontLightsSet(union ColorComplex *setColors){
	memcpy(led_left_PWM, setColors, 9);
	memcpy(led_right_PWM, &(setColors->color[9]), 9);
#ifndef DONGLE_CODE
	osSemaphoreAcquire(messageI2C_LockSem, osWaitForever);
	HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_LEFT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_left_PWM, 9, I2C_TIMEOUT);
	HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_RIGHT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_right_PWM, 9, I2C_TIMEOUT);
	osSemaphoreRelease(messageI2C_LockSem);
#endif

#ifdef DONGLE_CODE
	    	if(led_left_PWM[LED_LEFT_TOP_R] > 0)
	    	{
				HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);

	    	}
	    	else
			{
				HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
			}

	    	// if 1
	    	if (led_left_PWM[LED_LEFT_TOP_B] > 0)
	    	{
	    		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
			}
	    	else
			{
				HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
			}

	    	//if 2
	    	if (led_left_PWM[LED_LEFT_TOP_G] > 0)
	    	{
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
			}
	    	else
	    	{
	    		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
	    	}
#endif
}

struct test_color {
        uint8_t left_front_b;
        uint8_t left_front_g;
        uint8_t left_top_b;
        uint8_t left_top_g;
        uint8_t left_side_b;
        uint8_t left_side_g;
        uint8_t left_front_r;
		uint8_t left_top_r;
		uint8_t left_side_r;

		uint8_t right_front_b;
		uint8_t right_front_g;
		uint8_t right_top_b;
		uint8_t right_top_g;
		uint8_t right_side_b;
		uint8_t right_side_g;
		uint8_t right_front_r;
		uint8_t right_top_r;
		uint8_t right_side_r;
    };

struct test_color tempComplexLight;

void ThreadFrontLightsTask(void *argument)
{
#ifndef DONGLE_CODE
	setup_LP5523(LIS3DH_LEFT_ADDRESS);
	setup_LP5523(LIS3DH_RIGHT_ADDRESS);
#endif

//	LP5523 ledDriver;
//	ledDriver.begin();
	uint8_t index = 0;
	uint8_t flip = 0;
	uint32_t lightsSimpleMessageReceived;
//	for(int i = 0; i<10; i++){
//		led_PWM[i] = 255;
//	}
//	led_PWM[0] = 255;
	while(1){
//		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
//		osDelay(1000);
//		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		lightsSimpleMessageReceived = 0;



//		tempComplexLight.left_front_r = 0;
//		tempComplexLight.left_side_r = 0;
//		tempComplexLight.left_top_r = 0;
//		tempComplexLight.left_front_g = 0;
//		tempComplexLight.left_side_g = 150;
//		tempComplexLight.left_top_g = 150;
//		tempComplexLight.left_front_b = 150;
//		tempComplexLight.left_side_b = 0;
//		tempComplexLight.left_top_b = 0;
//
//
//		tempComplexLight.right_front_g = 0;
//		tempComplexLight.right_side_g = 0;
//		tempComplexLight.right_top_g = 0;
//		tempComplexLight.right_front_b = 0;
//		tempComplexLight.right_side_b = 0;
//		tempComplexLight.right_top_b = 0;
//		tempComplexLight.right_front_r = 150;
//		tempComplexLight.right_side_r = 150;
//		tempComplexLight.right_top_r = 150;
//
//		FrontLightsSet(&tempComplexLight);

		osMessageQueueGet(lightsSimpleQueueHandle, &lightsSimpleMessageReceived, 0U, osWaitForever);
#ifndef DONGLE_CODE
//		for(int i = 0; i<= 9; i++){
//
//		}


//		if(flip==0) led_PWM[index] += 2;
//
//		else led_PWM[index] -= 2;
//
//		if( (flip == 1) && (led_PWM[index] == 0)){
//			flip = 0;
//			index += 1;
//			if(index == 9) index = 0;
//		}
//		if(led_PWM[index] >= 240){
//			flip = 1;
////			led_PWM[index]= 0;
////			index += 1;
////			if(index == 9) index = 0;
//		}
//
////		for(int i=0; i<=9; i++) {
////			led_PWM[i] = i == index? 240 : 0;
////		}
////
////		index = index == 9? 0 : index+1;

//		// REMOVE BELOW
//		lightsSimpleMessageReceived = 0X00005229;

		for(int i = 0; i<= 8; i++){
			led_left_PWM[i] = (lightsSimpleMessageReceived & 0x01) * 255;
			lightsSimpleMessageReceived = lightsSimpleMessageReceived >> 1;
		}

		for(int i = 0; i<= 8; i++){
			led_right_PWM[i] = (lightsSimpleMessageReceived & 0x01) * 255;
			lightsSimpleMessageReceived = lightsSimpleMessageReceived >> 1;
		}

		//while(HAL_I2C_Mem_Write_IT(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_D1_PWM_REG, 1, led_PWM, 9) != HAL_OK);
		//HAL_I2C_Mem_Write_IT(I2C_HANDLE_TYPEDEF, LIS3DH_LEFT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_PWM, 9);
		//osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);


		//HAL_I2C_Mem_Write_IT(I2C_HANDLE_TYPEDEF, LIS3DH_RIGHT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_PWM, 9);
		//osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
		osSemaphoreAcquire(messageI2C_LockSem, osWaitForever);
		HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_LEFT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_left_PWM, 9, I2C_TIMEOUT);
		HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_RIGHT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_right_PWM, 9, I2C_TIMEOUT);
		osSemaphoreRelease(messageI2C_LockSem, osWaitForever);
//		osDelay(1000);


//		// REMOVE BELOW
//		osMessageQueueGet(lightsSimpleQueueHandle, &lightsSimpleMessageReceived, 0U, osWaitForever);
//		osDelay(2000);
//
//		lightsSimpleMessageReceived = 0X000381C0;
//
//		for(int i = 0; i<= 8; i++){
//			led_left_PWM[i] = (lightsSimpleMessageReceived & 0x01) * 255;
//			lightsSimpleMessageReceived = lightsSimpleMessageReceived >> 1;
//		}
//
//		for(int i = 0; i<= 8; i++){
//			led_right_PWM[i] = (lightsSimpleMessageReceived & 0x01) * 255;
//			lightsSimpleMessageReceived = lightsSimpleMessageReceived >> 1;
//		}
//
//		HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_LEFT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_left_PWM, 9, I2C_TIMEOUT);
//		HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_RIGHT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_right_PWM, 9, I2C_TIMEOUT);
//
//		osDelay(2000);
//
//		lightsSimpleMessageReceived = 0X00002C16;
//
//		for(int i = 0; i<= 8; i++){
//			led_left_PWM[i] = (lightsSimpleMessageReceived & 0x01) * 255;
//			lightsSimpleMessageReceived = lightsSimpleMessageReceived >> 1;
//		}
//
//		for(int i = 0; i<= 8; i++){
//			led_right_PWM[i] = (lightsSimpleMessageReceived & 0x01) * 255;
//			lightsSimpleMessageReceived = lightsSimpleMessageReceived >> 1;
//		}
//		HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_LEFT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_left_PWM, 9, I2C_TIMEOUT);
//		HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_RIGHT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_right_PWM, 9, I2C_TIMEOUT);
//
//		osDelay(2000);
//
//		lightsSimpleMessageReceived = 0X0003CDCD;
//
//		for(int i = 0; i<= 8; i++){
//			led_left_PWM[i] = (lightsSimpleMessageReceived & 0x01) * 255;
//			lightsSimpleMessageReceived = lightsSimpleMessageReceived >> 1;
//		}
//
//		for(int i = 0; i<= 8; i++){
//			led_right_PWM[i] = (lightsSimpleMessageReceived & 0x01) * 255;
//			lightsSimpleMessageReceived = lightsSimpleMessageReceived >> 1;
//		}
//		HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_LEFT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_left_PWM, 9, I2C_TIMEOUT);
//		HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_RIGHT_ADDRESS << 1, LIS3DH_D1_PWM_REG, 1, led_right_PWM, 9, I2C_TIMEOUT);
//
//		osDelay(2000);
#endif
	}
}

#ifdef __cplusplus
}
#endif

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
*************************************************************/
