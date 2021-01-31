/**
 ******************************************************************************
 * @file    app_thread.c
 * @author  MCD Application Team
 * @brief   Thread Application
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                     www.st.com/SLA0044
 *
 ******************************************************************************
 */


/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "utilities_common.h"
#include "app_entry.h"
#include "dbg_trace.h"
#include "app_thread.h"
#include "stm32wbxx_core_interface_def.h"
#include "openthread_api_wb.h"
#include "shci.h"
#include "stm_logging.h"
#include "app_conf.h"
#include "stm32_lpm.h"
#include "stm32_seq.h"
#if (CFG_USB_INTERFACE_ENABLE != 0)
#include "vcp.h"
#include "vcp_conf.h"
#endif /* (CFG_USB_INTERFACE_ENABLE != 0) */

/* Private defines -----------------------------------------------------------*/
#define C_SIZE_CMD_STRING   256U
#define C_PANID             0x2222U
#define C_CHANNEL_NB        12U
#define C_RESSOURCE         "light"

#define COAP_SEND_TIMEOUT                     (1*1000*1000/CFG_TS_TICK_VAL) /**< 1s */
#define THREAD_CHANGE_MODE_TIMEOUT            (1*1000*1000/CFG_TS_TICK_VAL) /**< 1s */

/* Private function prototypes -----------------------------------------------*/
static void APP_THREAD_CheckWirelessFirmwareInfo(void);
static void APP_THREAD_DeviceConfig(void);
static void APP_THREAD_StateNotif(uint32_t NotifFlags, void *pContext);
static void APP_THREAD_TraceError(const char * pMess, uint32_t ErrCode);
#if (CFG_FULL_LOW_POWER == 0)
static void Send_CLI_To_M0(void);
#endif /* (CFG_FULL_LOW_POWER == 0) */
static void Send_CLI_Ack_For_OT(void);
static void HostTxCb( void );
static void Wait_Getting_Ack_From_M0(void);
static void Receive_Ack_From_M0(void);
static void Receive_Notification_From_M0(void);
#if (CFG_USB_INTERFACE_ENABLE != 0)
static uint32_t ProcessCmdString(uint8_t* buf , uint32_t len);
#else
#if (CFG_FULL_LOW_POWER == 0)
static void RxCpltCallback(void);
#endif /* (CFG_FULL_LOW_POWER == 0) */
#endif /* (CFG_USB_INTERFACE_ENABLE != 0) */
static void APP_THREAD_SendCoapMsg(void);
static void APP_THREAD_SendCoapMulticastRequest(uint8_t command);
static void APP_THREAD_DummyReqHandler(void            * p_context,
                       otCoapHeader        * pHeader,
                       otMessage       * pMessage,
                       const otMessageInfo * pMessageInfo);
static void APP_THREAD_CoapRequestHandler(otCoapHeader        * pHeader,
                      otMessage       * pMessage,
                      const otMessageInfo * pMessageInfo);
static void APP_THREAD_SetSleepyEndDeviceMode(void);

static void APP_THREAD_CoapTimingElapsed( void );
static void APP_THREAD_SetThreadMode( void );


/* Private variables -----------------------------------------------*/
#if (CFG_USB_INTERFACE_ENABLE != 0)
static uint8_t TmpString[C_SIZE_CMD_STRING];
static uint8_t VcpRxBuffer[sizeof(TL_CmdSerial_t)];        /* Received Data over USB are stored in this buffer */
static uint8_t VcpTxBuffer[sizeof(TL_EvtPacket_t) + 254U]; /* Transmit buffer over USB */
#else
#if (CFG_FULL_LOW_POWER == 0)
static uint8_t aRxBuffer[C_SIZE_CMD_STRING];
#endif /* (CFG_FULL_LOW_POWER == 0) */
#endif /* (CFG_USB_INTERFACE_ENABLE != 0) */
#if (CFG_FULL_LOW_POWER == 0)
static uint8_t CommandString[C_SIZE_CMD_STRING];
#endif /* (CFG_FULL_LOW_POWER == 0) */
static __IO uint16_t indexReceiveChar = 0;
static __IO uint16_t CptReceiveCmdFromUser = 0;

static TL_CmdPacket_t *p_thread_otcmdbuffer;
static TL_EvtPacket_t *p_thread_notif_M0_to_M4;
static __IO uint32_t  CptReceiveMsgFromM0 = 0; /* Debug counter */

PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_TH_Config_t ThreadConfigBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ThreadOtCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t ThreadNotifRspEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255U];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ThreadCliCmdBuffer;

static otCoapResource OT_Ressource = {C_RESSOURCE, APP_THREAD_DummyReqHandler, (void*)APP_THREAD_CoapRequestHandler, NULL};
static otMessageInfo OT_MessageInfo = {0};
static uint8_t OT_Command = 0;
static otCoapHeader  OT_Header = {0};
static uint8_t OT_ReceivedCommand = 0;
static otMessage   * pOT_Message = NULL;
static otLinkModeConfig OT_LinkMode = {0};
static uint32_t sleepyEndDeviceFlag = FALSE;
/*--Debug counters--------------------------------*/
static uint32_t DebugCoapCpt = 0;

static uint8_t sedCoapTimerID;
static uint8_t setThreadModeTimerID;

/* Functions Definition ------------------------------------------------------*/

/**
 * @brief  Main entry point for the Thread Application
 * @param  none
 * @retval None
 */
void APP_THREAD_Init( void )
{
  SHCI_CmdStatus_t ThreadInitStatus;
  
  /* Check the compatibility with the Coprocessor Wireless Firmware loaded */
  APP_THREAD_CheckWirelessFirmwareInfo();

#if (CFG_USB_INTERFACE_ENABLE != 0)
  VCP_Init(&VcpTxBuffer[0], &VcpRxBuffer[0]);
#endif /* (CFG_USB_INTERFACE_ENABLE != 0) */

  /* Register cmdbuffer */
  APP_THREAD_RegisterCmdBuffer(&ThreadOtCmdBuffer);

  /**
   * Do not allow standby in the application
   */
  UTIL_LPM_SetOffMode(1 << CFG_LPM_APP_THREAD, UTIL_LPM_DISABLE);

  /* Init config buffer and call TL_THREAD_Init */
  APP_THREAD_TL_THREAD_INIT();

  /* Configure UART for sending CLI command from M4 */
  APP_THREAD_Init_UART_CLI();

  /* Send Thread start system cmd to M0 */
  ThreadInitStatus = SHCI_C2_THREAD_Init();

  UNUSED(ThreadInitStatus);
  /* Register task */
  /* Create the different tasks */
  UTIL_SEQ_RegTask( 1<<(uint32_t)CFG_TASK_MSG_FROM_M0_TO_M4, UTIL_SEQ_RFU, APP_THREAD_ProcessMsgM0ToM4);
  UTIL_SEQ_RegTask( 1<<(uint32_t)CFG_TASK_COAP_SEND_MSG, UTIL_SEQ_RFU,APP_THREAD_SendCoapMsg);
  UTIL_SEQ_RegTask( 1<<(uint32_t)CFG_TASK_SET_THREAD_MODE, UTIL_SEQ_RFU,APP_THREAD_SetSleepyEndDeviceMode);

  /* Initialize and configure the Thread device*/
  APP_THREAD_DeviceConfig();

  /**
   * Create timer to handle COAP request sending
   */
  HW_TS_Create(CFG_TIM_PROC_ID_ISR, &sedCoapTimerID, hw_ts_Repeated, APP_THREAD_CoapTimingElapsed);

  /**
   * Create timer to change Thread Mode to SED
   */
  HW_TS_Create(CFG_TIM_PROC_ID_ISR, &setThreadModeTimerID, hw_ts_SingleShot, APP_THREAD_SetThreadMode);

}

/**
  * @brief  Trace the error or the warning reported.
  * @param  ErrId :
  * @param  ErrCode :
  * @retval None
  */
void APP_THREAD_Error(uint32_t ErrId, uint32_t ErrCode)
{
  switch(ErrId)
  {
    case ERR_REC_MULTI_MSG_FROM_M0 :
      APP_THREAD_TraceError("ERROR : ERR_REC_MULTI_MSG_FROM_M0 ", ErrCode);
      break;
    case ERR_THREAD_SET_STATE_CB :
      APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_STATE_CB ",ErrCode);
      break;
   case ERR_THREAD_SET_CHANNEL :
      APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_CHANNEL ",ErrCode);
      break;
    case ERR_THREAD_SET_PANID :
      APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_PANID ",ErrCode);
      break;
    case ERR_THREAD_IPV6_ENABLE :
      APP_THREAD_TraceError("ERROR : ERR_THREAD_IPV6_ENABLE ",ErrCode);
      break;
    case ERR_THREAD_COAP_START :
      APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_START ",ErrCode);
      break;
    case ERR_THREAD_COAP_ADD_RESSOURCE :
      APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_ADD_RESSOURCE ",ErrCode);
      break;
    case ERR_THREAD_MESSAGE_READ :
      APP_THREAD_TraceError("ERROR : ERR_THREAD_MESSAGE_READ ",ErrCode);
      break;
    case ERR_THREAD_COAP_SEND_RESPONSE :
      APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_SEND_RESPONSE ",ErrCode);
      break;
    case ERR_THREAD_COAP_APPEND :
      APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_APPEND ",ErrCode);
      break;
    case ERR_THREAD_COAP_SEND_REQUEST :
      APP_THREAD_TraceError("ERROR : ERR_THREAD_COAP_SEND_REQUEST ",ErrCode);
      break;
    case ERR_TIMER_INIT :
      APP_THREAD_TraceError("ERROR : ERR_TIMER_INIT ",ErrCode);
      break;
    case ERR_TIMER_START :
      APP_THREAD_TraceError("ERROR : ERR_TIMER_START ",ErrCode);
      break;
    case ERR_THREAD_CHECK_WIRELESS :
      APP_THREAD_TraceError("ERROR : ERR_THREAD_CHECK_WIRELESS ",ErrCode);
      break;
    default :
      APP_THREAD_TraceError("ERROR Unknown\n",0);
      break;
  }
}

/**
 * @brief Process associated to the sending of a COAP request.
 * @param  None
 * @retval None
 */
static void APP_THREAD_SendCoapMsg(void)
{
  APP_THREAD_SendCoapMulticastRequest(1);
}

static void APP_THREAD_CoapTimingElapsed( void )
{
  otDeviceRole role;

  role = otThreadGetDeviceRole(NULL);

  if ((role == OT_DEVICE_ROLE_CHILD) || (role == OT_DEVICE_ROLE_ROUTER))
  {
    if (sleepyEndDeviceFlag == TRUE)
    {
      UTIL_SEQ_SetTask(TASK_COAP_SEND_MSG,CFG_SCH_PRIO_1);
    }
  }
}

static void APP_THREAD_SetThreadMode( void )
{
  UTIL_SEQ_SetTask(TASK_SET_THREAD_MODE,CFG_SCH_PRIO_1);
}

/**
  * @brief Dummy request handler
  * @param
  * @retval None
  */
static void APP_THREAD_DummyReqHandler(void    * p_context,
                           otCoapHeader        * pHeader,
                           otMessage           * pMessage,
                           const otMessageInfo * pMessageInfo)
{
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
 * @brief Thread initialization.
 *        This function configure the Thread mesh network.
 *
 *        It sets the following parameters :
 *        - call back (used to notify state change)
 *        - channel Id
 *        - panId
 *        and enable
 *        - IPV6
 *        - Thread
 *        - Coap
 *
 * @param  None
 * @retval None
 */
static void APP_THREAD_DeviceConfig(void)
{
  otError error;


  /* Configure the call-back in order to be notified when the device change its state */
  error = otInstanceErasePersistentInfo(NULL);
  if (error != OT_ERROR_NONE)
  {
    APP_THREAD_Error(ERR_THREAD_ERASE_PERSISTENT_INFO,error);
  }
  otInstanceFinalize(NULL);
  otInstanceInitSingle();
  error = otSetStateChangedCallback(NULL, APP_THREAD_StateNotif, NULL);
  if (error != OT_ERROR_NONE)
  {
    APP_THREAD_Error(ERR_THREAD_SET_STATE_CB,error);
  }
  /* Set the channel */
  error = otLinkSetChannel(NULL, C_CHANNEL_NB);
  if (error != OT_ERROR_NONE)
  {
    APP_THREAD_Error(ERR_THREAD_SET_CHANNEL,error);
  }
  /* Set the pandId */
  error = otLinkSetPanId(NULL, C_PANID);
  if (error != OT_ERROR_NONE)
  {
    APP_THREAD_Error(ERR_THREAD_SET_PANID,error);
  }
  /* Activate IPV6 */
  error = otIp6SetEnabled(NULL, true);
  if (error != OT_ERROR_NONE)
  {
   APP_THREAD_Error(ERR_THREAD_IPV6_ENABLE,error);
  }

  /* Start Thread */
  error = otThreadSetEnabled(NULL, true);
  if (error != OT_ERROR_NONE)
  {
    APP_THREAD_Error(ERR_THREAD_START,error);
  }
  /* Start the COAP server */
  error = otCoapStart(NULL, OT_DEFAULT_COAP_PORT);
  if (error != OT_ERROR_NONE)
  {
    APP_THREAD_Error(ERR_THREAD_COAP_START,error);
  }
  /* Add COAP resources */
  error = otCoapAddResource(NULL, &OT_Ressource);
  if (error != OT_ERROR_NONE)
  {
    APP_THREAD_Error(ERR_THREAD_COAP_ADD_RESSOURCE,error);
  }
}

/**
 * @brief This function is used to set the sleepyEndDevice mode
 *        and configure its pool period.
 *
 * @param None
 * @retval None
 */
static void APP_THREAD_SetSleepyEndDeviceMode(void)
{
  otError   error = OT_ERROR_NONE;

  /* Set the pool period to 5 sec. It means that when the device will enter
   * in 'sleepy end device' mode, it will send an ACK_Request every 5 sec.
   * This message will act as keep alive message.
   */
  otLinkSetPollPeriod(NULL,5000U);

  /* Set the sleepy end device mode */
  OT_LinkMode.mRxOnWhenIdle = 0;
  OT_LinkMode.mSecureDataRequests = 1U;
  OT_LinkMode.mDeviceType = 0;
  OT_LinkMode.mNetworkData = 1U;

  error = otThreadSetLinkMode(NULL,OT_LinkMode);
  if (error != OT_ERROR_NONE)
    APP_THREAD_Error(ERR_THREAD_LINK_MODE,error);

  /* After reaching the child or router state, the system
   *   a) sets the 'sleepy end device' mode
   *   b) perform a Thread stop
   *   c) perform a Thread start.
   *
   *  NOTE : According to the Thread specification, it is necessary to set the
   *         mode before starting Thread.
   *
   * A Child that has attached to its Parent indicating it is an FTD MUST NOT use Child UpdateRequest
   * to modify its mode to MTD.
   * As a result, you need to first detach from the network before switching from FTD to MTD at runtime,
   * then reattach.
   *
   */
  if (sleepyEndDeviceFlag == FALSE)
  {
    error = otThreadSetEnabled(NULL, false);
    if (error != OT_ERROR_NONE)
      APP_THREAD_Error(ERR_THREAD_LINK_MODE,error);
    error = otThreadSetEnabled(NULL, true);
    if (error != OT_ERROR_NONE)
      APP_THREAD_Error(ERR_THREAD_LINK_MODE,error);
    sleepyEndDeviceFlag = TRUE;
  }

  /* Start the timer */
  HW_TS_Start(sedCoapTimerID, (uint32_t)COAP_SEND_TIMEOUT);

}

/**
  * @brief Handler called when the server receives a COAP request.
  * @param pHeader : Header
  * @param pMessage : Message
  * @param pMessageInfo : Message information
  * @retval None
  */
static void APP_THREAD_CoapRequestHandler(otCoapHeader        * pHeader,
                          otMessage       * pMessage,
                          const otMessageInfo * pMessageInfo)
{
  do
  {
    if (otCoapHeaderGetType(pHeader) != OT_COAP_TYPE_NON_CONFIRMABLE)
    {
      break;
    }

    if (otCoapHeaderGetCode(pHeader) != OT_COAP_CODE_PUT)
    {
      break;
    }

    if (otMessageRead(pMessage, otMessageGetOffset(pMessage), &OT_ReceivedCommand, 1U) != 1U)
    {
      APP_THREAD_Error(ERR_THREAD_MESSAGE_READ, 0);
    }

    if (OT_ReceivedCommand == 1U)
    {
      BSP_LED_Toggle(LED1);
      APP_DBG("**** Recept COAP nb **** %d ",DebugCoapCpt++);
    }

  } while (false);
}

/**
  * @brief Send a COAP multicast request to all the devices which are connected
  *        on the Thread network
  * @param command : Command associated to the COAP request.
  * @retval None
  */
static void APP_THREAD_SendCoapMulticastRequest(uint8_t command)
{
  otError error = OT_ERROR_NONE;
  OT_Command = command;


  /* Forbid the 800_15_4 IP to enter in low power mode.
   *
   * Reason for that...
   * As soon as the M0 is receiving an ot cmd, it wake up the IP 802_15_4,
   * send the command and put back the IP 802_15_4 in sleep mode when possible.
   * If the application send too much ot_cmd commands sequentially, the IP 802_15_4
   * will wake up and sleep at a non reasonable speed. It is why it is advised
   * to control when the IP 802_15_4 radio is allowed to enter in low power.
   */
  SHCI_C2_RADIO_AllowLowPower(THREAD_IP,FALSE);

  /* Send the COAP request */
  do
  {
    otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);
    otCoapHeaderAppendUriPathOptions(&OT_Header,C_RESSOURCE);
    otCoapHeaderSetPayloadMarker(&OT_Header);

    pOT_Message = otCoapNewMessage(NULL, &OT_Header);
    if (pOT_Message == NULL)
    {
      break;
    }

    error = otMessageAppend(pOT_Message, &OT_Command, sizeof(command));
    if (error != OT_ERROR_NONE)
    {
      APP_THREAD_Error(ERR_THREAD_COAP_APPEND,error);
      break;
    }

    memset(&OT_MessageInfo, 0, sizeof(OT_MessageInfo));
    OT_MessageInfo.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
    OT_MessageInfo.mPeerPort = OT_DEFAULT_COAP_PORT;
    otIp6AddressFromString("FF03::1", &OT_MessageInfo.mPeerAddr);

    error = otCoapSendRequest(NULL,
                  pOT_Message,
                  &OT_MessageInfo,
                  NULL,
                  NULL);
  } while (false);

  APP_DBG("*** Send COAP nb **** %d",DebugCoapCpt++);

  if (error != OT_ERROR_NONE && pOT_Message != NULL)
  {
    otMessageFree(pOT_Message);
    APP_THREAD_Error(ERR_THREAD_COAP_SEND_REQUEST,error);
  }

  /* Allow the 800_15_4 IP to enter in low power mode */
  SHCI_C2_RADIO_AllowLowPower(THREAD_IP,TRUE);
}

/**
 * @brief Thread notification when the state changes.
 *        When the Thread device change state, a specific LED
 *        color is being displayed.
 *        LED2 On (Green) means that the device is in "Leader" mode.
 *        LED3 On (Red) means that the device is in "Child: mode or
 *         in "Router" mode.
 *        LED2 and LED3 off means that the device is in "Disabled"
 *         or "Detached" mode.
 *
 * @param  aFlags  : Define the item that has been modified
 *         aContext: Context
 *
 * @retval None
 */
static void APP_THREAD_StateNotif(uint32_t NotifFlags, void *pContext)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(pContext);
  if ((NotifFlags & (uint32_t)OT_CHANGED_THREAD_ROLE) == (uint32_t)OT_CHANGED_THREAD_ROLE)
  {
    switch (otThreadGetDeviceRole(NULL))
    {
      case OT_DEVICE_ROLE_DISABLED:
        BSP_LED_Off(LED2);
        BSP_LED_Off(LED3);
        break;
      case OT_DEVICE_ROLE_DETACHED:
        BSP_LED_Off(LED2);
        BSP_LED_Off(LED3);
        break;
      case OT_DEVICE_ROLE_CHILD:
        BSP_LED_Off(LED2);
        BSP_LED_On(LED3);
        /* Set the mode sleepy end device */
        /* Start the timer */
        HW_TS_Start(setThreadModeTimerID, (uint32_t)THREAD_CHANGE_MODE_TIMEOUT);
        break;
      case OT_DEVICE_ROLE_ROUTER :
        BSP_LED_Off(LED2);
        BSP_LED_On(LED3);
        /* Set the mode sleepy end device */
        /* Start the timer */
        HW_TS_Start(setThreadModeTimerID, (uint32_t)THREAD_CHANGE_MODE_TIMEOUT);
        break;
      case OT_DEVICE_ROLE_LEADER :
        BSP_LED_On(LED2);
        BSP_LED_Off(LED3);
        break;
      default:
        BSP_LED_Off(LED2);
        BSP_LED_Off(LED3);
        break;
    }
  }
}

/**
  * @brief  Warn the user that an error has occurred.In this case,
  *         the LEDs on the Board will start blinking.
  *
  * @param  mess  : Message associated to the error.
 * @param  ErrCode: Error code associated to the module (OpenThread or other module if any)
  * @retval None
  */
static void APP_THREAD_TraceError(const char * pMess, uint32_t ErrCode)
{
  APP_DBG("**** Fatal error = %s (Err = %d)",pMess,ErrCode);

  while(1U == 1U)
  {
    /* Note : Can be replace HAL_Delay with timer to toggle LEDs */
    BSP_LED_On(LED1);
    BSP_LED_On(LED2);
    BSP_LED_On(LED3);
  }
}

/**
 * @brief Check if the Coprocessor Wireless Firmware loaded supports Thread
 *        and display associated informations
 * @param  None
 * @retval None
 */
static void APP_THREAD_CheckWirelessFirmwareInfo(void)
{
  WirelessFwInfo_t wireless_info_instance;
  WirelessFwInfo_t* p_wireless_info = &wireless_info_instance;

  if (SHCI_GetWirelessFwInfo(p_wireless_info) != SHCI_Success)
  {
    APP_THREAD_Error((uint32_t)ERR_THREAD_CHECK_WIRELESS, (uint32_t)ERR_INTERFACE_FATAL);
  }
  else
  {
    APP_DBG("**********************************************************");
    APP_DBG("WIRELESS COPROCESSOR FW:");
    /* Print version */
    APP_DBG("VERSION ID = %d.%d.%d", p_wireless_info->VersionMajor, p_wireless_info->VersionMinor, p_wireless_info->VersionSub);

    switch(p_wireless_info->StackType)
    {
    case INFO_STACK_TYPE_THREAD_FTD :
      APP_DBG("FW Type : Thread FTD");
      break;
    case INFO_STACK_TYPE_THREAD_MTD :
      APP_DBG("FW Type : Thread MTD");
      break;
    case INFO_STACK_TYPE_BLE_THREAD_FTD_STATIC :
      APP_DBG("FW Type : Static Concurrent Mode BLE/Thread");
      break;
    default :
      /* No Thread device supported ! */
      APP_THREAD_Error((uint32_t)ERR_THREAD_CHECK_WIRELESS, (uint32_t)ERR_INTERFACE_FATAL);
      break;
    }
    APP_DBG("**********************************************************");
  }
}

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/

void APP_THREAD_RegisterCmdBuffer(TL_CmdPacket_t* p_buffer)
{
  p_thread_otcmdbuffer = p_buffer;
}

Thread_OT_Cmd_Request_t* THREAD_Get_OTCmdPayloadBuffer(void)
{
  return (Thread_OT_Cmd_Request_t*)p_thread_otcmdbuffer->cmdserial.cmd.payload;
}

Thread_OT_Cmd_Request_t* THREAD_Get_OTCmdRspPayloadBuffer(void)
{
  return (Thread_OT_Cmd_Request_t*)((TL_EvtPacket_t *)p_thread_otcmdbuffer)->evtserial.evt.payload;
}

Thread_OT_Cmd_Request_t* THREAD_Get_NotificationPayloadBuffer(void)
{
  return (Thread_OT_Cmd_Request_t*)(p_thread_notif_M0_to_M4)->evtserial.evt.payload;
}

/**
 * @brief  This function is used to transfer the Ot commands from the
 *         M4 to the M0.
 *
 * @param   None
 * @return  None
 */
void Ot_Cmd_Transfer(void)
{
  /* OpenThread OT command cmdcode range 0x280 .. 0x3DF = 352 */
  p_thread_otcmdbuffer->cmdserial.cmd.cmdcode = 0x280U;
  /* Size = otCmdBuffer->Size (Number of OT cmd arguments : 1 arg = 32bits so multiply by 4 to get size in bytes)
   * + ID (4 bytes) + Size (4 bytes) */
  uint32_t l_size = ((Thread_OT_Cmd_Request_t*)(p_thread_otcmdbuffer->cmdserial.cmd.payload))->Size * 4U + 8U;
  p_thread_otcmdbuffer->cmdserial.cmd.plen = l_size;

  TL_OT_SendCmd();

  /* Wait completion of cmd */
  Wait_Getting_Ack_From_M0();
}

/**
 * @brief  This function is called when acknowledge from OT command is received from the M0+.
 *
 * @param   Otbuffer : a pointer to TL_EvtPacket_t
 * @return  None
 */
void TL_OT_CmdEvtReceived( TL_EvtPacket_t * Otbuffer )
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Otbuffer);

  Receive_Ack_From_M0();
}

/**
 * @brief  This function is called when notification from M0+ is received.
 *
 * @param   Notbuffer : a pointer to TL_EvtPacket_t
 * @return  None
 */
void TL_THREAD_NotReceived( TL_EvtPacket_t * Notbuffer )
{
  p_thread_notif_M0_to_M4 = Notbuffer;

  Receive_Notification_From_M0();
}

/**
  * @brief  This function is called before sending any ot command to the M0
  *         core. The purpose of this function is to be able to check if
  *         there are no notifications coming from the M0 core which are
  *         pending before sending a new ot command.
  * @param  None
  * @retval None
  */
void Pre_OtCmdProcessing(void)
{
#ifdef CFG_LPM_CORE
#else
    UTIL_SEQ_WaitEvt(EVENT_SYNCHRO_BYPASS_IDLE);
#endif /* CFG_LPM_CORE */
}

/**
  * @brief  This function waits for getting an acknowledgment from the M0.
  *
  * @param  None
  * @retval None
  */
static void Wait_Getting_Ack_From_M0(void)
{
  UTIL_SEQ_WaitEvt(EVENT_ACK_FROM_M0_EVT);
}

/**
  * @brief  Receive an acknowledgment from the M0+ core.
  *         Each command send by the M4 to the M0 are acknowledged.
  *         This function is called under interrupt.
  * @param  None
  * @retval None
  */
static void Receive_Ack_From_M0(void)
{
  UTIL_SEQ_SetEvt(EVENT_ACK_FROM_M0_EVT);
}

/**
  * @brief  Receive a notification from the M0+ through the IPCC.
  *         This function is called under interrupt.
  * @param  None
  * @retval None
  */
static void Receive_Notification_From_M0(void)
{
  CptReceiveMsgFromM0++;
  UTIL_SEQ_SetTask(TASK_MSG_FROM_M0_TO_M4,CFG_SCH_PRIO_0);
}

#if (CFG_USB_INTERFACE_ENABLE != 0)
#else
#if (CFG_FULL_LOW_POWER == 0)
static void RxCpltCallback(void)
{
  /* Filling buffer and wait for '\r' char */
  if (indexReceiveChar < C_SIZE_CMD_STRING)
  {
    CommandString[indexReceiveChar++] = aRxBuffer[0];
    if (aRxBuffer[0] == '\r')
    {
      CptReceiveCmdFromUser = 1U;

      /* UART task scheduling*/
      UTIL_SEQ_SetTask(1U << CFG_TASK_SEND_CLI_TO_M0, CFG_SCH_PRIO_0);
    }
  }

  /* Once a character has been sent, put back the device in reception mode */
  HW_UART_Receive_IT(CFG_CLI_UART, aRxBuffer, 1U, RxCpltCallback);
}
#endif /* (CFG_FULL_LOW_POWER == 0) */
#endif /* (CFG_USB_INTERFACE_ENABLE != 0) */

#if (CFG_USB_INTERFACE_ENABLE != 0)
/**
 * @brief Process the command strings.
 *        As soon as a complete command string has been received, the task
 *        in charge of sending the command to the M0 is scheduled
 * @param  None
 * @retval None
 */
static uint32_t  ProcessCmdString( uint8_t* buf , uint32_t len )
{
  uint32_t i,j,tmp_start;
  tmp_start = 0;
  uint32_t res = 0;

  i= 0;
  while ((buf[i] != '\r') && (i < len))
  {
    i++;
  }

  if (i != len)
  {
    memcpy(CommandString, buf,(i+1));
    indexReceiveChar = i + 1U; /* Length of the buffer containing the command string */
    UTIL_SEQ_SetTask(1U << CFG_TASK_SEND_CLI_TO_M0, CFG_SCH_PRIO_0);
    tmp_start = i;
    for (j = 0; j < (len - tmp_start - 1U) ; j++)
    {
      buf[j] = buf[tmp_start + j + 1U];
    }
    res = len - tmp_start - 1U;
  }
  else
  {
    res = len;
  }
  return res; /* Remaining characters in the temporary buffer */
}
#endif/* (CFG_USB_INTERFACE_ENABLE != 0) */

#if (CFG_FULL_LOW_POWER == 0)
/**
 * @brief Process sends receive CLI command to M0.
 * @param  None
 * @retval None
 */
static void Send_CLI_To_M0(void)
{
  memset(ThreadCliCmdBuffer.cmdserial.cmd.payload, 0x0U, 255U);
  memcpy(ThreadCliCmdBuffer.cmdserial.cmd.payload, CommandString, indexReceiveChar);
  ThreadCliCmdBuffer.cmdserial.cmd.plen = indexReceiveChar;
  ThreadCliCmdBuffer.cmdserial.cmd.cmdcode = 0x0;

  /* Clear receive buffer, character counter and command complete */
  CptReceiveCmdFromUser = 0;
  indexReceiveChar = 0;
  memset(CommandString, 0, C_SIZE_CMD_STRING);

  TL_CLI_SendCmd();
}
#endif /* (CFG_FULL_LOW_POWER == 0) */

/**
 * @brief Send notification for CLI TL Channel.
 * @param  None
 * @retval None
 */
static void Send_CLI_Ack_For_OT(void)
{

  /* Notify M0 that characters have been sent to UART */
  TL_THREAD_CliSendAck();
}

/**
 * @brief Perform initialization of CLI UART interface.
 * @param  None
 * @retval None
 */
void APP_THREAD_Init_UART_CLI(void)
{
#if (CFG_FULL_LOW_POWER == 0)
  UTIL_SEQ_RegTask( 1<<CFG_TASK_SEND_CLI_TO_M0, UTIL_SEQ_RFU,Send_CLI_To_M0);
#endif /* (CFG_FULL_LOW_POWER == 0) */

#if (CFG_USB_INTERFACE_ENABLE != 0)
#else
  HW_UART_Init(CFG_CLI_UART);
  HW_UART_Receive_IT(CFG_CLI_UART, aRxBuffer, 1, RxCpltCallback);
#endif /* (CFG_USB_INTERFACE_ENABLE != 0) */
}

/**
 * @brief Perform initialization of TL for THREAD.
 * @param  None
 * @retval None
 */
void APP_THREAD_TL_THREAD_INIT(void)
{
  ThreadConfigBuffer.p_ThreadOtCmdRspBuffer = (uint8_t*)&ThreadOtCmdBuffer;
  ThreadConfigBuffer.p_ThreadNotAckBuffer = (uint8_t*)ThreadNotifRspEvtBuffer;
  ThreadConfigBuffer.p_ThreadCliRspBuffer = (uint8_t*)&ThreadCliCmdBuffer;

  TL_THREAD_Init( &ThreadConfigBuffer );
}

/**
 * @brief  This function is called when notification on CLI TL Channel from M0+ is received.
 *
 * @param   Notbuffer : a pointer to TL_EvtPacket_t
 * @return  None
 */
void TL_THREAD_CliNotReceived( TL_EvtPacket_t * Notbuffer )
{
  TL_CmdPacket_t* l_CliBuffer = (TL_CmdPacket_t*)Notbuffer;
  uint8_t l_size = l_CliBuffer->cmdserial.cmd.plen;

  /* WORKAROUND: if string to output is "> " then respond directly to M0 and do not output it */
  if (strcmp((const char *)l_CliBuffer->cmdserial.cmd.payload, "> ") != 0)
  {
    /* Write to CLI UART */
#if (CFG_USB_INTERFACE_ENABLE != 0)
    VCP_SendData( l_CliBuffer->cmdserial.cmd.payload, l_size, HostTxCb);
#else
    HW_UART_Transmit_IT(CFG_CLI_UART, l_CliBuffer->cmdserial.cmd.payload, l_size, HostTxCb);
#endif /*USAGE_OF_VCP */
  }
  else
  {
    Send_CLI_Ack_For_OT();
  }
}

/**
 * @brief  End of transfer callback for CLI UART sending.
 *
 * @param   Notbuffer : a pointer to TL_EvtPacket_t
 * @return  None
 */
void HostTxCb(void)
{
  Send_CLI_Ack_For_OT();
}

/**
 * @brief Process the messages coming from the M0.
 * @param  None
 * @retval None
 */
void APP_THREAD_ProcessMsgM0ToM4(void)
{
  if (CptReceiveMsgFromM0 != 0)
  {
    /* If CptReceiveMsgFromM0 is > 1. it means that we did not serve all the events from the radio */
    if (CptReceiveMsgFromM0 > 1U)
    {
      APP_THREAD_Error(ERR_REC_MULTI_MSG_FROM_M0, 0);
    }
    else
    {
      OpenThread_CallBack_Processing();
    }
    /* Reset counter */
    CptReceiveMsgFromM0 = 0;
  }
}

#if (CFG_USB_INTERFACE_ENABLE != 0)
/**
 * @brief  This function is called when thereare some data coming
 *         from the Hyperterminal via the USB port
 *         Data received over USB OUT endpoint are sent over CDC interface
 *         through this function.
 * @param  Buf: Buffer of data received
 * @param  Len: Number of data received (in bytes)
 * @retval Number of characters remaining in the buffer and not yet processed
 */
void VCP_DataReceived(uint8_t* Buf , uint32_t *Len)
{
  uint32_t i,flag_continue_checking = TRUE;
  uint32_t char_remaining = 0;
  static uint32_t len_total = 0;

  /* Copy the characteres in the temporary buffer */
  for (i = 0; i < *Len; i++)
  {
    TmpString[len_total++] = Buf[i];
  }

  /* Process the buffer commands one by one     */
  /* A command is limited by a \r caracaters    */
  while (flag_continue_checking == TRUE)
  {
    char_remaining = ProcessCmdString(TmpString,len_total);
    /* If char_remaining is equal to len_total, it means that the command string is not yet
     * completed.
     * If char_remaining is equal to 0, it means that the command string has
     * been entirely processed.
     */
    if ((char_remaining == 0) || (char_remaining == len_total))
    {
      flag_continue_checking = FALSE;
    }
    len_total = char_remaining;
  }
}
#endif /* (CFG_USB_INTERFACE_ENABLE != 0) */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
