/**
 ******************************************************************************
 * File Name           : blink.c
  * Description        : C file for blink sensing.
  ******************************************************************************

  *
  ******************************************************************************
 */


/* includes -----------------------------------------------------------*/
#include "blink.h"
#include "gpio.h"
#include "stm32wbxx_hal.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "tim.h"


/* typedef -----------------------------------------------------------*/


/* defines -----------------------------------------------------------*/


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/


/* Functions Definition ------------------------------------------------------*/



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
static struct blinkData blinkMsgBuffer_1 = {{0},1};
static struct blinkData blinkMsgBuffer_2 = {{0},2};

static struct blinkData *buffer_pointer;

void BlinkTask(void){
	uint32_t evt;

	while(1){
		evt = osThreadFlagsWait (0x00000001U, osFlagsWaitAny, osWaitForever);

		// if signal was received successfully, start blink task
		if (evt == 0x00000001U)  {

			// start timer and PWM channel for blink LED
			HAL_TIM_Base_Start(&htim2);
			HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

			// start timer for ADC to sample at 1kHz

			// message passing until told to stop
			//      note: DMA triggers callback where buffers are switched and the full one
			//      is passed by reference via queue to masterThread for packetization
			while(1){
				// stop timer and put thread in idle if signal was reset
				osDelay(100); // what I should do is do the queue put calls in the DMA callback and suspend the thread here (the masterThread should resume thread when wanting to stop)
				blinkMsgBuffer_1.tick_ms = (blinkMsgBuffer_1.tick_ms) + 1;
				buffer_pointer = &blinkMsgBuffer_1;
		    	osMessageQueuePut(blinkMsgQueueHandle, &buffer_pointer, 0U, 0U);
				//osMessageQueuePut(blinkMsgQueueHandle, &blinkMsgBuffer_2, 0U, 0U);

				// stop timer and put thread in idle if signal was reset
				evt = osThreadFlagsWait (0x00000002U, osFlagsWaitAny, 0);
				if( evt == 0x00000002U){
					HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
					HAL_TIM_Base_Stop(&htim2);
					break;
				}
			}
		}
	}
}



/**
 * @brief Setting up blink sensing
 * @param  None
 * @retval None
 */
void SetupBlinkSensing(void){

}
/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
*************************************************************/
