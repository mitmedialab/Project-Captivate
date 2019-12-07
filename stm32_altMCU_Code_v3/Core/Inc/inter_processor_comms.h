/**
 ******************************************************************************
 * File Name          : inter_processor_comms.h
  * Description        : Header for Lights.
  ******************************************************************************

  *
  ******************************************************************************
 */
#ifndef TEMPLATE_H
#define TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes -----------------------------------------------------------*/
#include "cmsis_os.h"


/* typedef -----------------------------------------------------------*/


/* defines -----------------------------------------------------------*/


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/
osMessageQueueId_t 	sendMsgToMainQueueHandle;
osThreadId_t sendMsgToMainTaskHandle;

/* Functions Definition ------------------------------------------------------*/
void SendPacketToMainTask(void *argument);


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





#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
