/**
 ******************************************************************************
 * File Name          : thread_helper.h
  * Description        :
  ******************************************************************************

  *
  ******************************************************************************
 */
#ifndef THREAD_HELPER_H
#define THREAD_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes -----------------------------------------------------------*/
#include <openthread/dataset.h>
#include <openthread/link.h>
#include <openthread/message.h>

/* typedef -----------------------------------------------------------*/


/* defines -----------------------------------------------------------*/


/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/
OTAPI otError OTCALL otThreadSetChildSupervisionCheckTimeout(otInstance *aInstance, uint16_t timeout);
OTAPI otError OTCALL otThreadSetChildSupervisionInterval(otInstance *aInstance, uint16_t interval);

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

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
*************************************************************/





#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
