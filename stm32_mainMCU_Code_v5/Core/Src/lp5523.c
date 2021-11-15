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
//#include "main.h"
#include "string.h"
#include "config.h"
#include "master_thread.h"

#include "system_settings.h"

#include "captivate_config.h"

/* typedef -----------------------------------------------------------*/

/* defines -----------------------------------------------------------*/
#define I2C_HANDLE_TYPEDEF 	&hi2c1
#define I2C_TIMEOUT			100

/* macros ------------------------------------------------------------*/

/* function prototypes -----------------------------------------------*/

/* variables -----------------------------------------------*/
#ifdef DONGLE_CODE
GPIO_TypeDef* GPIO_PORT_DONGLE[3] = {LED1_GPIO_Port, LED2_GPIO_Port, LED3_GPIO_Port};
const uint16_t GPIO_PIN_DONGLE[3] = {LED1_Pin, LED2_Pin, LED3_Pin};
#endif
union ColorComplex receivedColor;

/* Functions Definition ------------------------------------------------------*/

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

#define MAX_BRIGHTNESS 20

uint8_t led_left_PWM[9] = { 0 };
uint8_t led_right_PWM[9] = { 0 };
uint8_t led_current[9] = { MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS,
		MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS,
		MAX_BRIGHTNESS, MAX_BRIGHTNESS };
static const union ColorComplex EmptyColorComplex;


//uint8_t led_current[9] = {LED_BRIGHTNESS, 200, 200, 200, 200, 200, 200, 200, 200};

struct LightConfig {
	uint8_t current[9];
	uint8_t intensity[9];
};

const uint8_t packet_array[9] = { LOG_EN, LOG_EN, LOG_EN, LOG_EN, LOG_EN,
		LOG_EN, LOG_EN, LOG_EN, LOG_EN };
uint8_t deviceAddress;
uint8_t led_PWM[9] = { 0 };
uint8_t packet;

void setup_LP5523(uint8_t ADDR) {

#ifndef DONGLE_CODE
	deviceAddress = ADDR << 1;

	// enable chip
	osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
	packet = LP5525_CHIP_EN;
//	while(HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_EN_CNTRL1_REG, 1, &packet, 1, I2C_TIMEOUT) != HAL_OK);
	HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_EN_CNTRL1_REG,
			1, &packet, 1, I2C_TIMEOUT);

	// put charge-pump in auto-mode, serial auto increment, internal clock
	packet = CP_MODE_AUTO | EN_AUTO_INC | INT_CLK_EN;
//	while(HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_MISC_REG, 1, &packet, 1, I2C_TIMEOUT) != HAL_OK);
	HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_MISC_REG, 1,
			&packet, 1, I2C_TIMEOUT);

	// set PWM level (0 to 255)
//	while(HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_D1_PWM_REG, 1, led_PWM, 9, I2C_TIMEOUT) != HAL_OK);
	HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_D1_PWM_REG, 1,
			led_PWM, 9, I2C_TIMEOUT);

	// set current control (0 to 25.5 mA) - step size is 100uA
//	while(HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_D1_CURRENT_CTRL_REG, 1, led_current, 9, I2C_TIMEOUT) != HAL_OK);
	HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress,
			LIS3DH_D1_CURRENT_CTRL_REG, 1, led_current, 9, I2C_TIMEOUT);

	// enable logarithmic dimming
//	packet = LOG_EN;
//	while(HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_D1_CNTRL_REG, 1, packet_array, 9, I2C_TIMEOUT) != HAL_OK);
	HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, deviceAddress, LIS3DH_D1_CNTRL_REG, 1,
			packet_array, 9, I2C_TIMEOUT);

	osSemaphoreRelease(messageI2C_LockHandle);
#else
	BSP_LED_Init(LED_BLUE);
	BSP_LED_Init(LED_GREEN);
	BSP_LED_Init(LED_RED);
#endif
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

void FrontLightsSet(union ColorComplex *setColors) {

	memcpy(led_left_PWM, setColors, 9);
	memcpy(led_right_PWM, &(setColors->color[9]), 9);
#ifndef DONGLE_CODE
	osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
	HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_LEFT_ADDRESS << 1,
			LIS3DH_D1_PWM_REG, 1, led_left_PWM, 9, I2C_TIMEOUT);
	HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_RIGHT_ADDRESS << 1,
			LIS3DH_D1_PWM_REG, 1, led_right_PWM, 9, I2C_TIMEOUT);
	osSemaphoreRelease(messageI2C_LockHandle);
#endif

#ifdef DONGLE_CODE
	    	if(led_left_PWM[LED_LEFT_TOP_R] > 0)
	    	{
				HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);

	    	}
	    	else
			{
				HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
			}

	    	// if 1
	    	if (led_left_PWM[LED_LEFT_TOP_B] > 0)
	    	{
	    		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
			}
	    	else
			{
				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
			}

	    	//if 2
	    	if (led_left_PWM[LED_LEFT_TOP_G] > 0)
	    	{
				HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
			}
	    	else
	    	{
	    		HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
	    	}
#endif
}

union ColorComplex receivedColors;
void ThreadFrontLightsComplexTask(void *argument){

	setup_LP5523(LIS3DH_LEFT_ADDRESS);
	setup_LP5523(LIS3DH_RIGHT_ADDRESS);

	uint32_t counter = 0;

	while (1) {
		osMessageQueueGet(lightsComplexQueueHandle, &receivedColors,
						0U, osWaitForever);
		memcpy(led_left_PWM, &receivedColors, 9);
		memcpy(led_right_PWM, &(receivedColors.color[9]), 9);
	#ifndef DONGLE_CODE
		osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);

		HAL_I2C_Mem_Write_IT(I2C_HANDLE_TYPEDEF, LIS3DH_LEFT_ADDRESS << 1,
				LIS3DH_D1_PWM_REG, 1, led_left_PWM, 9);

		counter = 0;
		while( (HAL_I2C_GetState(I2C_HANDLE_TYPEDEF) != HAL_I2C_STATE_READY)){
			counter+=20;
			osDelay(20);

			if(counter > 1000){
				HAL_I2C_Master_Abort_IT(I2C_HANDLE_TYPEDEF, LIS3DH_LEFT_ADDRESS << 1);
			}
		}


		HAL_I2C_Mem_Write_IT(I2C_HANDLE_TYPEDEF, LIS3DH_RIGHT_ADDRESS << 1,
				LIS3DH_D1_PWM_REG, 1, led_right_PWM, 9);

		counter = 0;
		while( (HAL_I2C_GetState(I2C_HANDLE_TYPEDEF) != HAL_I2C_STATE_READY)){
			counter+=20;
			osDelay(20);

			if(counter > 1000){
				HAL_I2C_Master_Abort_IT(I2C_HANDLE_TYPEDEF, LIS3DH_LEFT_ADDRESS << 1);
			}
		}

		osSemaphoreRelease(messageI2C_LockHandle);
	#endif
	}
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

void ThreadFrontLightsTask(void *argument) {

//	osDelay(1); // added delay because it seems that semaphores arent fully initialized and code stalls when releasing semaphore
//#ifndef DONGLE_CODE
	setup_LP5523(LIS3DH_LEFT_ADDRESS);
	setup_LP5523(LIS3DH_RIGHT_ADDRESS);
//#endif

	uint32_t lightsSimpleMessageReceived;

#ifdef LED_TEST
	uint8_t led_test = 1;
#endif

	while (1) {

		lightsSimpleMessageReceived = 0;

#ifdef LED_TEST
		if (led_test == 1){
			while(1){
				tempComplexLight.left_front_r = 0;
				tempComplexLight.left_side_r = 0;
				tempComplexLight.left_top_r = 0;
				tempComplexLight.left_front_g = 0;
				tempComplexLight.left_side_g = 0;
				tempComplexLight.left_top_g = 0;
				tempComplexLight.left_front_b = 0;
				tempComplexLight.left_side_b = 255;
				tempComplexLight.left_top_b = 0;


				tempComplexLight.right_front_g = 0;
				tempComplexLight.right_side_g = 0;
				tempComplexLight.right_top_g = 0;
				tempComplexLight.right_front_b = 0;
				tempComplexLight.right_side_b = 255;
				tempComplexLight.right_top_b = 0;
				tempComplexLight.right_front_r = 0;
				tempComplexLight.right_side_r = 0;
				tempComplexLight.right_top_r = 0;

				FrontLightsSet(&tempComplexLight);

				osDelay(5000);


//				tempComplexLight.left_front_r = 0;
//				tempComplexLight.left_side_r = 0;
//				tempComplexLight.left_top_r = 0;
//				tempComplexLight.left_front_g = 255;
//				tempComplexLight.left_side_g = 255;
//				tempComplexLight.left_top_g = 255;
//				tempComplexLight.left_front_b = 0;
//				tempComplexLight.left_side_b = 0;
//				tempComplexLight.left_top_b = 0;
//
//
//				tempComplexLight.right_front_g = 255;
//				tempComplexLight.right_side_g = 255;
//				tempComplexLight.right_top_g = 255;
//				tempComplexLight.right_front_b = 0;
//				tempComplexLight.right_side_b = 0;
//				tempComplexLight.right_top_b = 0;
//				tempComplexLight.right_front_r = 0;
//				tempComplexLight.right_side_r = 0;
//				tempComplexLight.right_top_r = 0;
//
//				FrontLightsSet(&tempComplexLight);
//
//				osDelay(5000);
//
//				tempComplexLight.left_front_r = 0;
//				tempComplexLight.left_side_r = 0;
//				tempComplexLight.left_top_r = 0;
//				tempComplexLight.left_front_g = 0;
//				tempComplexLight.left_side_g = 0;
//				tempComplexLight.left_top_g = 0;
//				tempComplexLight.left_front_b = 255;
//				tempComplexLight.left_side_b = 255;
//				tempComplexLight.left_top_b = 255;
//
//
//				tempComplexLight.right_front_g = 0;
//				tempComplexLight.right_side_g = 0;
//				tempComplexLight.right_top_g = 0;
//				tempComplexLight.right_front_b = 255;
//				tempComplexLight.right_side_b = 255;
//				tempComplexLight.right_top_b = 255;
//				tempComplexLight.right_front_r = 0;
//				tempComplexLight.right_side_r = 0;
//				tempComplexLight.right_top_r = 0;
//
//				FrontLightsSet(&tempComplexLight);
//
//				osDelay(5000);
//
//				tempComplexLight.left_front_r = 255;
//				tempComplexLight.left_side_r = 255;
//				tempComplexLight.left_top_r = 255;
//				tempComplexLight.left_front_g = 0;
//				tempComplexLight.left_side_g = 0;
//				tempComplexLight.left_top_g = 0;
//				tempComplexLight.left_front_b = 0;
//				tempComplexLight.left_side_b = 0;
//				tempComplexLight.left_top_b = 0;
//
//
//				tempComplexLight.right_front_g = 0;
//				tempComplexLight.right_side_g = 0;
//				tempComplexLight.right_top_g = 0;
//				tempComplexLight.right_front_b = 0;
//				tempComplexLight.right_side_b = 0;
//				tempComplexLight.right_top_b = 0;
//				tempComplexLight.right_front_r = 255;
//				tempComplexLight.right_side_r = 255;
//				tempComplexLight.right_top_r = 255;
//
//				FrontLightsSet(&tempComplexLight);
//
//				osDelay(5000);
			}


			led_test = 0;

		}
#endif

		osDelay(1);

		osMessageQueueGet(lightsSimpleQueueHandle, &lightsSimpleMessageReceived,
				0U, osWaitForever);

		for (int i = 0; i <= 8; i++) {
			led_left_PWM[i] = (lightsSimpleMessageReceived & 0x01) * 255;
			lightsSimpleMessageReceived = lightsSimpleMessageReceived >> 1;
		}

		for (int i = 0; i <= 8; i++) {
			led_right_PWM[i] = (lightsSimpleMessageReceived & 0x01) * 255;
			lightsSimpleMessageReceived = lightsSimpleMessageReceived >> 1;
		}

		osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
		HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_LEFT_ADDRESS << 1,
				LIS3DH_D1_PWM_REG, 1, led_left_PWM, 9, I2C_TIMEOUT);
		HAL_I2C_Mem_Write(I2C_HANDLE_TYPEDEF, LIS3DH_RIGHT_ADDRESS << 1,
				LIS3DH_D1_PWM_REG, 1, led_right_PWM, 9, I2C_TIMEOUT);
		osSemaphoreRelease(messageI2C_LockHandle);

	}
}

#ifdef DONGLE_CODE
/**
  * @brief  Configures LED GPIO.
  * @param  Led: LED to be configured.
  *          This parameter can be one of the following values:
  *     @arg LED2
  * @retval None
  */
void BSP_LED_Init(Led_TypeDef Led)
{
  GPIO_InitTypeDef  gpioinitstruct = {0};

  /* Enable the GPIO_LED Clock */
  LEDx_GPIO_CLK_ENABLE(Led);

  /* Configure the GPIO_LED pin */
  gpioinitstruct.Pin = GPIO_PIN_DONGLE[Led];
  gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
  gpioinitstruct.Pull = GPIO_NOPULL;
  gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(GPIO_PORT_DONGLE[Led], &gpioinitstruct);

  HAL_GPIO_WritePin(GPIO_PORT_DONGLE[Led], GPIO_PIN_DONGLE[Led], GPIO_PIN_RESET);
}

/**
  * @brief  DeInit LEDs.
  * @param  Led: LED to be de-init.
  *   This parameter can be one of the following values:
  *     @arg  LED2
  * @note Led DeInit does not disable the GPIO clock nor disable the Mfx
  * @retval None
  */
void BSP_LED_DeInit(Led_TypeDef Led)
{
  GPIO_InitTypeDef  gpio_init_structure;

  /* Turn off LED */
  HAL_GPIO_WritePin(GPIO_PORT_DONGLE[Led], GPIO_PIN_DONGLE[Led], GPIO_PIN_RESET);
  /* DeInit the GPIO_LED pin */
  gpio_init_structure.Pin = GPIO_PIN_DONGLE[Led];
  HAL_GPIO_DeInit(GPIO_PORT_DONGLE[Led], gpio_init_structure.Pin);
}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on.
  *   This parameter can be one of following parameters:
  *     @arg LED2
  * @retval None
  */
void BSP_LED_On(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(GPIO_PORT_DONGLE[Led], GPIO_PIN_DONGLE[Led], GPIO_PIN_SET);
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off.
  *   This parameter can be one of following parameters:
  *     @arg LED2
  * @retval None
  */
void BSP_LED_Off(Led_TypeDef Led)
{
  HAL_GPIO_WritePin(GPIO_PORT_DONGLE[Led], GPIO_PIN_DONGLE[Led], GPIO_PIN_RESET);
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled.
  *   This parameter can be one of following parameters:
  *     @arg LED2
  * @retval None
  */
void BSP_LED_Toggle(Led_TypeDef Led)
{
  HAL_GPIO_TogglePin(GPIO_PORT_DONGLE[Led], GPIO_PIN_DONGLE[Led]);
}
#endif

void ledStartupSequence(void){
	resetColor(&receivedColor);

	receivedColor.colors_indiv.left_front_b = 255;
	osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);
//	FrontLightsSet(&receivedColor););
	osDelay(LED_START_SEQ_INTERVAL);

	receivedColor.colors_indiv.left_front_b = 0;
	receivedColor.colors_indiv.left_front_g = 255;

	receivedColor.colors_indiv.left_top_b = 255;
	osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);
//	FrontLightsSet(&receivedColor);
	osDelay(LED_START_SEQ_INTERVAL);

	receivedColor.colors_indiv.left_front_g = 0;
	receivedColor.colors_indiv.left_front_r = 255;

	receivedColor.colors_indiv.left_top_b = 0;
	receivedColor.colors_indiv.left_top_g = 255;

	receivedColor.colors_indiv.left_side_b = 255;
	osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);
//	FrontLightsSet(&receivedColor);
	osDelay(LED_START_SEQ_INTERVAL);

	receivedColor.colors_indiv.left_front_r = 0;

	receivedColor.colors_indiv.left_top_g = 0;
	receivedColor.colors_indiv.left_top_r = 255;

	receivedColor.colors_indiv.left_side_b = 0;
	receivedColor.colors_indiv.left_side_g = 255;

	receivedColor.colors_indiv.right_side_b = 255;
	osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);
//	FrontLightsSet(&receivedColor);
	osDelay(LED_START_SEQ_INTERVAL);

	receivedColor.colors_indiv.left_top_r = 0;

	receivedColor.colors_indiv.left_side_g = 0;
	receivedColor.colors_indiv.left_side_r = 255;

	receivedColor.colors_indiv.right_side_b = 0;
	receivedColor.colors_indiv.right_side_g = 255;

	receivedColor.colors_indiv.right_top_b = 255;
	osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);
//	FrontLightsSet(&receivedColor);
	osDelay(LED_START_SEQ_INTERVAL);

	receivedColor.colors_indiv.left_side_r = 0;

	receivedColor.colors_indiv.right_side_g = 0;
	receivedColor.colors_indiv.right_side_r = 255;

	receivedColor.colors_indiv.right_top_b = 0;
	receivedColor.colors_indiv.right_top_g = 255;

	receivedColor.colors_indiv.right_front_b = 255;
	osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);
//	FrontLightsSet(&receivedColor);
	osDelay(LED_START_SEQ_INTERVAL);

	receivedColor.colors_indiv.right_side_r = 0;

	receivedColor.colors_indiv.right_top_g = 0;
	receivedColor.colors_indiv.right_top_r = 255;

	receivedColor.colors_indiv.right_front_b = 0;
	receivedColor.colors_indiv.right_front_g = 255;
	osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);
//	FrontLightsSet(&receivedColor);
	osDelay(LED_START_SEQ_INTERVAL);

	receivedColor.colors_indiv.right_top_r = 0;

	receivedColor.colors_indiv.right_front_g = 0;
	receivedColor.colors_indiv.right_front_r = 255;
	osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);
//	FrontLightsSet(&receivedColor);
	osDelay(LED_START_SEQ_INTERVAL);

	receivedColor.colors_indiv.right_front_r = 0;
	osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);
//	FrontLightsSet(&receivedColor);

	ledDisconnectNotification();
}

void ledDisconnectNotification(void){
	resetColor(&receivedColor);

	receivedColor.colors_indiv.left_side_g = 0;
	receivedColor.colors_indiv.right_side_g = 0;
	receivedColor.colors_indiv.left_side_b = 50;
	receivedColor.colors_indiv.right_side_b = 50;
	osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);
	osDelay(10);
//	FrontLightsSet(&receivedColor);
}

void ledConnectNotification(void){
	resetColor(&receivedColor);

	receivedColor.colors_indiv.left_side_b = 0;
	receivedColor.colors_indiv.right_side_b = 0;
	receivedColor.colors_indiv.left_side_g = 80;
	receivedColor.colors_indiv.right_side_g = 80;
	osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);
//	FrontLightsSet(&receivedColor);
	osDelay(1000);
	receivedColor.colors_indiv.left_side_g = 0;
	receivedColor.colors_indiv.right_side_g = 0;
	osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);
//	FrontLightsSet(&receivedColor);
}

void ledAllRed(void){
	resetColor(&receivedColor);

	receivedColor.colors_indiv.left_side_r = 255;
	receivedColor.colors_indiv.right_side_r = 255;
	receivedColor.colors_indiv.left_top_r = 255;
	receivedColor.colors_indiv.right_top_r = 255;
	receivedColor.colors_indiv.left_front_r = 255;
	receivedColor.colors_indiv.right_front_r = 255;
	osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);
}

void resetColor(union ColorComplex * colorComplex){
	memcpy(colorComplex,&EmptyColorComplex,sizeof(union ColorComplex));;
}

#ifdef __cplusplus
}
#endif

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
 *************************************************************/
