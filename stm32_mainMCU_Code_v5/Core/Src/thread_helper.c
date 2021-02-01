/**
 ******************************************************************************
 * File Name           : thread_helper.c
  * Description        :
  ******************************************************************************

  *
  ******************************************************************************
 */


/* includes -----------------------------------------------------------*/
#include "stm32wbxx_hal.h"

#include "stm32wbxx_core_interface_def.h"
#include "tl_thread_hci.h"

/* Include definition of compilation flags requested for OpenThread configuration */
#include OPENTHREAD_CONFIG_FILE
#include "thread.h"

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
 * @brief Set interval for telling children nodes that router is still alive
 * @param  None
 * @retval None
 */
OTAPI otError OTCALL otThreadSetChildSupervisionCheckTimeout(otInstance *aInstance, uint16_t timeout)
{
  Pre_OtCmdProcessing();
  /* prepare buffer */
  Thread_OT_Cmd_Request_t* p_ot_req = THREAD_Get_OTCmdPayloadBuffer();

  p_ot_req->ID = MSG_M4TOM0_OT_CHILD_SUPERVISION_SET_CHECK_TIMEOUT;

  p_ot_req->Size=1;
  p_ot_req->Data[0] = (uint32_t) timeout;

  Ot_Cmd_Transfer();

  p_ot_req = THREAD_Get_OTCmdRspPayloadBuffer();
  return (otError)p_ot_req->Data[0];
}

/**
 * @brief Set timeout interval for child node to find router
 * @param  None
 * @retval None
 */
OTAPI otError OTCALL otThreadSetChildSupervisionInterval(otInstance *aInstance, uint16_t interval)
{
  Pre_OtCmdProcessing();
  /* prepare buffer */
  Thread_OT_Cmd_Request_t* p_ot_req = THREAD_Get_OTCmdPayloadBuffer();

  p_ot_req->ID = MSG_M4TOM0_OT_CHILD_SUPERVISION_SET_INTERVAL;

  p_ot_req->Size=1;
  p_ot_req->Data[0] = (uint32_t) interval;

  Ot_Cmd_Transfer();

  p_ot_req = THREAD_Get_OTCmdRspPayloadBuffer();
  return (otError)p_ot_req->Data[0];
}

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
*************************************************************/
