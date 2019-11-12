/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * File Name          : app_thread.c
  * Description        : Thread Application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

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

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
#define C_SIZE_CMD_STRING       256U
#define C_PANID                 0x4444U
#define C_CHANNEL_NB            13U

/* USER CODE BEGIN PD */
#define C_RESSOURCE_BASIC		"test"
#define C_RESSOURCE				"light"
/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
static void APP_THREAD_CheckWirelessFirmwareInfo(void);
static void APP_THREAD_DeviceConfig(void);
static void APP_THREAD_StateNotif(uint32_t NotifFlags, void *pContext);
static void APP_THREAD_TraceError(const char * pMess, uint32_t ErrCode);
static void Send_CLI_To_M0(void);
static void Send_CLI_Ack_For_OT(void);
static void HostTxCb( void );
static void Wait_Getting_Ack_From_M0(void);
static void Receive_Ack_From_M0(void);
static void Receive_Notification_From_M0(void);
#if (CFG_HW_LPUART1_ENABLED == 1)
extern void MX_LPUART1_UART_Init(void);
#endif
#if (CFG_HW_USART1_ENABLED == 1)
extern void MX_USART1_UART_Init(void);
#endif
#if (CFG_USB_INTERFACE_ENABLE != 0)
static uint32_t ProcessCmdString(uint8_t* buf , uint32_t len);
#else
static void RxCpltCallback(void);
#endif

/* USER CODE BEGIN PFP */
static void APP_THREAD_DummyReqHandler(void * p_context,
                  otCoapHeader * pHeader,
                  otMessage * pMessage,
                  const otMessageInfo * pMessageInfo);

static void APP_THREAD_SendCoapUnicastRequest();

static void APP_THREAD_CoapRequestHandler(otCoapHeader        * pHeader,
                                  otMessage           * pMessage,
                                  const otMessageInfo * pMessageInfo);
/* USER CODE END PFP */

/* Private variables -----------------------------------------------*/

#if (CFG_USB_INTERFACE_ENABLE != 0)
static uint8_t TmpString[C_SIZE_CMD_STRING];
static uint8_t VcpRxBuffer[sizeof(TL_CmdSerial_t)];        /* Received Data over USB are stored in this buffer */
static uint8_t VcpTxBuffer[sizeof(TL_EvtPacket_t) + 254U]; /* Transmit buffer over USB */
#else
static uint8_t aRxBuffer[C_SIZE_CMD_STRING];
#endif /* (CFG_USB_INTERFACE_ENABLE != 0) */

static uint8_t CommandString[C_SIZE_CMD_STRING];
static __IO uint16_t indexReceiveChar = 0;
static __IO uint16_t CptReceiveCmdFromUser = 0;

static TL_CmdPacket_t *p_thread_otcmdbuffer;
static TL_EvtPacket_t *p_thread_notif_M0_to_M4;
static __IO uint32_t  CptReceiveMsgFromM0 = 0;
PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_TH_Config_t ThreadConfigBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ThreadOtCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t ThreadNotifRspEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255U];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ThreadCliCmdBuffer;

/* USER CODE BEGIN PV */
static otCoapResource OT_Ressource = {C_RESSOURCE, APP_THREAD_DummyReqHandler, (void*)APP_THREAD_CoapRequestHandler, NULL};
//static otCoapResource OT_RessourceDataTransfer = {C_RESSOURCE_DATA_TRANSFER, APP_THREAD_DummyReqHandler, (void*)APP_THREAD_CoapDataReqHandler, NULL};
//static otCoapResource OT_RessourceProvisionning = {C_RESSOURCE_Provisioning, APP_THREAD_DummyReqHandler, (void*)APP_THREAD_ProvisioningReqHandler, NULL};
static otMessageInfo OT_MessageInfo = {0};
static otCoapHeader  OT_Header = {0};
static uint8_t OT_Command = 0;
static uint16_t OT_BufferIdRead = 1U;
static uint16_t OT_BufferIdSend = 1U;
//static uint8_t OT_BufferSend[COAP_PAYLOAD_MAX_LENGTH] = {0};
//static uint8_t OT_BufferReceived[COAP_PAYLOAD_MAX_LENGTH] = {0};
static otMessage   * pOT_Message = NULL;
static otIp6Address   OT_PeerAddress = { .mFields.m8 = { 0 } };

otMasterKey raw = {0x33, 0x33, 0x44, 0x44, 0x33, 0x33, 0x44, 0x44,
		0x33, 0x33, 0x44, 0x44, 0x33, 0x33, 0x44, 0x44};
//otIp6Address binary_border_ipv6;
static char border_ipv6[100] = "fd11:22::994e:6ed7:263d:6187";
//static char border_ipv6[100] = "FF03::1";
//const char border_ipv6[50] = "fd33:3333:3344:0:0:ff:fe00:e400";
otError   error = OT_ERROR_NONE;
static uint8_t OT_ReceivedCommand = 0;
static char OT_BufferSend[20] = "TEST PAYLOAD";
volatile otIp6Address * myAddress;
volatile otMessageInfo * tempMessageInfo;
volatile uint16_t myRloc16;
volatile otNetifAddress * unicastAddresses;

volatile bool isEnabledIpv6;
volatile otNetifMulticastAddress * multicastAddresses;
volatile otIp6Address *  meshLocalEID;
volatile otIp6Address * linkLocalIPV6;
volatile otMessageInfo * receivedMessage;

//static uint8_t command = 1;
/* USER CODE END PV */

/* Functions Definition ------------------------------------------------------*/

void APP_THREAD_Init( void )
{
  /* USER CODE BEGIN APP_THREAD_INIT_1 */

  /* USER CODE END APP_THREAD_INIT_1 */

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
  
  /* Prevent unused argument(s) compilation warning */
  UNUSED(ThreadInitStatus);

  /* Register task */
  /* Create the different tasks */
  UTIL_SEQ_RegTask( 1<<(uint32_t)CFG_TASK_MSG_FROM_M0_TO_M4, UTIL_SEQ_RFU, APP_THREAD_ProcessMsgM0ToM4);

  /* USER CODE BEGIN INIT TASKS */
//  UTIL_SEQ_RegTask( 1<<(uint32_t)CFG_TASK_COAP_MSG_BUTTON, UTIL_SEQ_RFU, APP_THREAD_SendCoapMsg);
  /* USER CODE END INIT TASKS */

  /* Initialize and configure the Thread device*/
  APP_THREAD_DeviceConfig();

 /* USER CODE BEGIN APP_THREAD_INIT_2 */
//  HW_TS_Create(CFG_TIM_PROC_ID_ISR, &TimerID, hw_ts_SingleShot, APP_THREAD_TimingElapsed);
//
//  APP_THREAD_InitPayloadWrite();
 /* USER CODE END APP_THREAD_INIT_2 */
}

/**
  * @brief  Trace the error or the warning reported.
  * @param  ErrId :
  * @param  ErrCode
  * @retval None
  */
void APP_THREAD_Error(uint32_t ErrId, uint32_t ErrCode)
{
  /* USER CODE BEGIN APP_THREAD_Error_1 */

  /* USER CODE END APP_THREAD_Error_1 */
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
  case ERR_THREAD_START :
    APP_THREAD_TraceError("ERROR: ERR_THREAD_START ", ErrCode);
    break;
  case ERR_THREAD_ERASE_PERSISTENT_INFO :
    APP_THREAD_TraceError("ERROR : ERR_THREAD_ERASE_PERSISTENT_INFO ",ErrCode);
    break;
  case ERR_THREAD_CHECK_WIRELESS :
    APP_THREAD_TraceError("ERROR : ERR_THREAD_CHECK_WIRELESS ",ErrCode);
    break;
  /* USER CODE BEGIN APP_THREAD_Error_2 */

  /* USER CODE END APP_THREAD_Error_2 */
  default :
    APP_THREAD_TraceError("ERROR Unknown ", 0);
    break;
    }
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
static void APP_THREAD_DeviceConfig(void)
{
  otError error;
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
  error = otLinkSetChannel(NULL, C_CHANNEL_NB);
  if (error != OT_ERROR_NONE)
  {
    APP_THREAD_Error(ERR_THREAD_SET_CHANNEL,error);
  }
  error = otLinkSetPanId(NULL, C_PANID);
  if (error != OT_ERROR_NONE)
  {
    APP_THREAD_Error(ERR_THREAD_SET_PANID,error);
  }

  //error = otThreadSetMasterKey(NULL, &raw);

  error = otIp6SetEnabled(NULL, true);
  if (error != OT_ERROR_NONE)
  {
    APP_THREAD_Error(ERR_THREAD_IPV6_ENABLE,error);
  }

  error = otThreadSetEnabled(NULL, true);
  if (error != OT_ERROR_NONE)
  {
    APP_THREAD_Error(ERR_THREAD_START,error);
  }

  /* USER CODE BEGIN DEVICECONFIG */
  error = otCoapStart(NULL, OT_DEFAULT_COAP_PORT);
  error = otCoapAddResource(NULL, &OT_Ressource);

  /* Start the COAP server */
//	error = otCoapStart(NULL, OT_DEFAULT_COAP_PORT);
//	if (error != OT_ERROR_NONE)
//	{
//	  APP_THREAD_Error(ERR_THREAD_COAP_START,error);
//	}
//	/* Add COAP resources */
//
//	if (error != OT_ERROR_NONE)
//	{
//	  APP_THREAD_Error(ERR_THREAD_COAP_ADD_RESSOURCE,error);
//	}
//	/* Add APP_THREAD_AskProvisioning resources */
//	error = otCoapAddResource(NULL, &OT_RessourceProvisionning);
//	if (error != OT_ERROR_NONE)
//	{
//	  APP_THREAD_Error(ERR_THREAD_COAP_ADD_RESSOURCE,error);
//	}

//  raw = otThreadGetMasterKey(NULL);
//  otThreadSetMasterKey(NULL, ,);
  /* USER CODE END DEVICECONFIG */
}

/**
 * @brief Thread notification when the state changes.
 * @param  aFlags  : Define the item that has been modified
 *         aContext: Context
 *
 * @retval None
 */
static void APP_THREAD_StateNotif(uint32_t NotifFlags, void *pContext)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(pContext);

  /* USER CODE BEGIN APP_THREAD_STATENOTIF */
  static uint32_t provisioning = 0;
  /* USER CODE END APP_THREAD_STATENOTIF */

  if ((NotifFlags & (uint32_t)OT_CHANGED_THREAD_ROLE) == (uint32_t)OT_CHANGED_THREAD_ROLE)
  {
    switch (otThreadGetDeviceRole(NULL))
    {
    case OT_DEVICE_ROLE_DISABLED:
      /* USER CODE BEGIN OT_DEVICE_ROLE_DISABLED */
    	HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
    	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
      /* USER CODE END OT_DEVICE_ROLE_DISABLED */
      break;
    case OT_DEVICE_ROLE_DETACHED:
      /* USER CODE BEGIN OT_DEVICE_ROLE_DETACHED */
    	HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
    	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
      /* USER CODE END OT_DEVICE_ROLE_DETACHED */
      break;
    case OT_DEVICE_ROLE_CHILD:
      /* USER CODE BEGIN OT_DEVICE_ROLE_CHILD */
    	HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
    	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
      /* USER CODE END OT_DEVICE_ROLE_CHILD */
      break;
    case OT_DEVICE_ROLE_ROUTER :
      /* USER CODE BEGIN OT_DEVICE_ROLE_ROUTER */
    	HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
    	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);

        APP_THREAD_SendCoapUnicastRequest();
      /* USER CODE END OT_DEVICE_ROLE_ROUTER */
      break;
    case OT_DEVICE_ROLE_LEADER :
      /* USER CODE BEGIN OT_DEVICE_ROLE_LEADER */
    	  HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
    	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

    	  APP_THREAD_SendCoapUnicastRequest();
      /* USER CODE END OT_DEVICE_ROLE_LEADER */
      break;
    default:
      /* USER CODE BEGIN DEFAULT */

      /* USER CODE END DEFAULT */
      break;
    }
  }
}

/**
  * @brief  Warn the user that an error has occurred.In this case,
  *         the LEDs on the Board will start blinking.
  *
  * @param  pMess  : Message associated to the error.
  * @param  ErrCode: Error code associated to the module (OpenThread or other module if any)
  * @retval None
  */
static void APP_THREAD_TraceError(const char * pMess, uint32_t ErrCode)
{
  /* USER CODE BEGIN TRACE_ERROR */

  /* USER CODE END TRACE_ERROR */
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
/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

static void APP_THREAD_DummyRespHandler(void                * p_context,
									  otCoapHeader        * pHeader,
									  otMessage           * pMessage,
									  const otMessageInfo * pMessageInfo,
									  otError             Result)
{
	/* Prevent unused argument(s) compilation warning */
	UNUSED(p_context);
	UNUSED(pHeader);
	UNUSED(pMessage);
	UNUSED(pMessageInfo);
	UNUSED(Result);
}

/**
  * @brief Dummy request handler
  * @param
  * @retval None
  */
static void APP_THREAD_DummyReqHandler(void            * p_context,
                                   otCoapHeader        * pHeader,
                                   otMessage           * pMessage,
                                   const otMessageInfo * pMessageInfo)
{
    tempMessageInfo = pMessageInfo;
    receivedMessage = (otMessageInfo *) pMessage;
}

static void APP_THREAD_CoapRequestHandler(otCoapHeader * pHeader,
                                  otMessage            * pMessage,
                                  const otMessageInfo  * pMessageInfo)
{
  do
  {
    APP_THREAD_SendCoapUnicastRequest();
    tempMessageInfo = pMessageInfo;
    receivedMessage = (otMessageInfo *) pMessage;

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
      //APP_THREAD_Error(ERR_THREAD_MESSAGE_READ, 0);
    }

    if (OT_ReceivedCommand == 1U)
    {
      //BSP_LED_Toggle(LED1);
    }

  } while (false);
}

/**
 * @brief Initialize CoAP write buffer.
 * @param  None
 * @retval None
 */
//static void APP_THREAD_InitPayloadWrite(void)
//{
//  uint8_t i;
//  for(i = 0; i < COAP_PAYLOAD_LENGTH; i++)
//  {
//    PayloadWrite[i] = 0xFF;
//  }
//}

/**
 * @brief Task associated to the push button.
 * @param  None
 * @retval None
 */
//static void APP_THREAD_SendCoapMsg(void)
//{
//  APP_DBG("********* STEP 1: Send a CoAP NON-CONFIRMABLE PUT Request *********");
//  /* Send a NON-CONFIRMABLE PUT Request */
//  APP_THREAD_CoapSendRequest(&OT_Ressource, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT, MULICAST_FTD_MED, PayloadWrite, sizeof(PayloadWrite));
//
//  /* Insert Delay here using Hw timer server */
//  /* Start the timer */
//  HW_TS_Start(TimerID, (uint32_t)WAIT_TIMEOUT);
//  UTIL_SEQ_WaitEvt(EVENT_TIMER);
//
//  APP_DBG("********* STEP 2: Send a CoAP CONFIRMABLE PUT Request *********");
//  /* Send a CONFIRMABLE PUT Request */
//  APP_THREAD_CoapSendRequest(&OT_Ressource, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_PUT, MULICAST_FTD_MED, PayloadWrite, sizeof(PayloadWrite));
//}
//
//static void APP_THREAD_TimingElapsed(void)
//{
//  APP_DBG("--- APP_THREAD_TimingElapsed ---");
//  UTIL_SEQ_SetEvt(EVENT_TIMER);
//}
//
///**
// * @brief  Compare the message received versus the original message.
// * @param  None
// * @retval None
// */
//static bool APP_THREAD_CheckMsgValidity(void)
//{
//  bool valid = true;
//  uint32_t i;
//
//  for(i = 0; i < COAP_PAYLOAD_LENGTH; i++)
//  {
//    if(PayloadRead[i] != PayloadWrite[i])
//    {
//      valid = false;
//    }
//  }
//
//  if(valid == true)
//  {
//    APP_DBG("PAYLOAD Comparison OK!");
//  }else
//  {
//    APP_THREAD_Error(ERR_THREAD_MSG_COMPARE_FAILED, 0);
//  }
//  return valid;
//}

static void APP_THREAD_SendCoapUnicastRequest()
{
  //otError   error = OT_ERROR_NONE;

//  if (error != OT_ERROR_NONE)
//  {
//    APP_THREAD_Error(ERR_APEND_URI,error);
//  }
//  if (pOT_Message == NULL)
//  {
//    APP_THREAD_Error(ERR_ALLOC_MSG,error);
//  }
  //error = otMessageAppend(pOT_Message, &OT_BufferSend, sizeof(OT_BufferSend));
//  if (error != OT_ERROR_NONE)
//  {
//    APP_THREAD_Error(ERR_THREAD_COAP_APPEND,error);
//  }

//  memcpy(&OT_MessageInfo.mPeerAddr, &OT_PeerAddress, sizeof(OT_MessageInfo.mPeerAddr));
//  error = otCoapSendRequest(NULL,
//          pOT_Message,
//          &OT_MessageInfo,
//          &APP_THREAD_DummyRespHandler,
//          (void*)&APP_THREAD_DataRespHandler);

  /************ SET MESSAGE INFO (WHERE THE PACKET GOES) ************/
  // https://openthread.io/reference/struct/ot-message-info.html#structot_message_info

//	do{
//	  myAddress = otThreadGetMeshLocalEid(NULL);
//
//
//	  // clear info
//	  memset(&OT_MessageInfo, 0, sizeof(OT_MessageInfo));
//
//	  // set border IP address
//	  // error = otIp6AddressFromString("FF03::01", &OT_MessageInfo.mPeerAddr);
//	  error = otIp6AddressFromString("fd11:22::1f48:3934:3802:dede", &OT_MessageInfo.mPeerAddr);
//	  //error = otIp6AddressFromString("fdde:ad00:beef:0:0:ff:fe00:3800", &OT_MessageInfo.mPeerAddr);
//	  memset(&OT_MessageInfo.mSockAddr, otThreadGetMeshLocalEid(NULL), sizeof(OT_MessageInfo.mSockAddr));
//	  OT_MessageInfo.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
//	  OT_MessageInfo.mPeerPort = OT_DEFAULT_COAP_PORT;
//	  //OT_MessageInfo.mHopLimit = 20;
//
//	  /************** CREATE NEW MESSAGE **********************/
//
//	  // create header
//	  otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_PUT);
//	  //otCoapHeaderSetMessageId(&OT_Header,OT_BufferIdSend); //may not need since sendRequest should set to 0
//	  otCoapHeaderGenerateToken(&OT_Header, 2U); //This function sets the Token length and randomizes its value.
//
//	  // the name of the resource
//	  //error = otCoapHeaderAppendUriPathOptions(&OT_Header,C_RESSOURCE_BASIC);
//	  error = otCoapHeaderAppendUriPathOptions(&OT_Header,"basic");
//
//	  // This function adds Payload Marker indicating beginning of the payload to the CoAP header
//	  otCoapHeaderSetPayloadMarker(&OT_Header);
//
//	  // creates new message with headers but with empty payload
//	  pOT_Message = otCoapNewMessage(NULL, &OT_Header);
//	  if (pOT_Message == NULL) while(1);
//	  // Append bytes to a message (this is where the payload gets added)
//
//	  error = otMessageAppend(pOT_Message, OT_BufferSend, sizeof(OT_BufferSend));
//	  if (error != OT_ERROR_NONE) while(1);
//
//	  error = otCoapSendRequest(NULL,
//				pOT_Message,
//				&OT_MessageInfo,
//				&APP_THREAD_DummyRespHandler,
//				(void*) NULL);
//
//	  // if error: free allocated message buffer if one was allocated
//	  if (error != OT_ERROR_NONE && pOT_Message != NULL)
//	  {
//		otMessageFree(pOT_Message);
//	  }
//
//	  //HAL_Delay(10000);
//	}while(false);
//
//	HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
//
//	do{
//		  // clear info
//		  memset(&OT_MessageInfo, 0, sizeof(OT_MessageInfo));
//
//		  // set border IP address
//		   error = otIp6AddressFromString("fd33:3333:3344:0:ab24:af3d:a3e0:17a4", &OT_MessageInfo.mPeerAddr);
//		  // error = otIp6AddressFromString("fd11:22::994e:6ed7:263d:6187", &OT_MessageInfo.mPeerAddr);
//		  //error = otIp6AddressFromString("fdde:ad00:beef:0:0:ff:fe00:3800", &OT_MessageInfo.mPeerAddr);
//
//		  OT_MessageInfo.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
//		  OT_MessageInfo.mPeerPort = OT_DEFAULT_COAP_PORT;
//		  //OT_MessageInfo.mHopLimit = 20;
//
//		  /************** CREATE NEW MESSAGE ********************ifco*/
//
//		  // create header
//		  otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_PUT);
//		  //otCoapHeaderSetMessageId(&OT_Header,OT_BufferIdSend); //may not need since sendRequest should set to 0
//		  otCoapHeaderGenerateToken(&OT_Header, 2U); //This function sets the Token length and randomizes its value.
//
//		  // the name of the resource
//		  //error = otCoapHeaderAppendUriPathOptions(&OT_Header,C_RESSOURCE_BASIC);
//		  error = otCoapHeaderAppendUriPathOptions(&OT_Header,"basic");
//
//		  // This function adds Payload Marker indicating beginning of the payload to the CoAP header
//		  otCoapHeaderSetPayloadMarker(&OT_Header);
//
//		  // creates new message with headers but with empty payload
//		  pOT_Message = otCoapNewMessage(NULL, &OT_Header);
//		  if (pOT_Message == NULL) while(1);
//		  // Append bytes to a message (this is where the payload gets added)
//
//		  error = otMessageAppend(pOT_Message, OT_BufferSend, sizeof(OT_BufferSend));
//		  if (error != OT_ERROR_NONE) while(1);
//
//		  error = otCoapSendRequest(NULL,
//					pOT_Message,
//					&OT_MessageInfo,
//					NULL,
//					(void*) NULL);
//
//		  // if error: free allocated message buffer if one was allocated
//		  if (error != OT_ERROR_NONE && pOT_Message != NULL)
//		  {
//			otMessageFree(pOT_Message);
//		  }
//
//		  //HAL_Delay(10000);
//		}while(false);

	do{
//			  myRloc16 = otThreadGetRloc16(NULL);
//			  unicastAddresses = otIp6GetUnicastAddresses(NULL);
//			  isEnabledIpv6 = otIp6IsEnabled(NULL);
//			  multicastAddresses = otIp6GetMulticastAddresses(NULL);
//			  meshLocalEID =  otThreadGetMeshLocalEid(NULL);
//			  linkLocalIPV6 = otThreadGetLinkLocalIp6Address(NULL);


			  // clear info
			  memset(&OT_MessageInfo, 0, sizeof(OT_MessageInfo));

			  // set border IP address
			   error = otIp6AddressFromString("ff03::1", &OT_MessageInfo.mPeerAddr);

			   memcpy(&OT_MessageInfo.mSockAddr, otThreadGetMeshLocalEid(NULL), sizeof(OT_MessageInfo.mSockAddr));

			   // error = otIp6AddressFromString("fd11:22::994e:6ed7:263d:6187", &OT_MessageInfo.mPeerAddr);
			  //error = otIp6AddressFromString("fdde:ad00:beef:0:0:ff:fe00:3800", &OT_MessageInfo.mPeerAddr);

			  OT_MessageInfo.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
			  OT_MessageInfo.mPeerPort = OT_DEFAULT_COAP_PORT;
			  //OT_MessageInfo.mHopLimit = 20;

			  /************** CREATE NEW MESSAGE ********************ifco*/

			  // create header
			  otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);
			  //otCoapHeaderSetMessageId(&OT_Header,OT_BufferIdSend); //may not need since sendRequest should set to 0
			  otCoapHeaderGenerateToken(&OT_Header, 2U); //This function sets the Token length and randomizes its value.

			  // the name of the resource
			  //error = otCoapHeaderAppendUriPathOptions(&OT_Header,C_RESSOURCE_BASIC);
			  error = otCoapHeaderAppendUriPathOptions(&OT_Header,"light");

			  // This function adds Payload Marker indicating beginning of the payload to the CoAP header
			  otCoapHeaderSetPayloadMarker(&OT_Header);

			  // creates new message with headers but with empty payload
			  pOT_Message = otCoapNewMessage(NULL, &OT_Header);
			  if (pOT_Message == NULL) while(1);
			  // Append bytes to a message (this is where the payload gets added)

			  //error = otMessageAppend(pOT_Message, OT_BufferSend, sizeof(OT_BufferSend));
			  error = otMessageAppend(pOT_Message, &OT_MessageInfo, sizeof(OT_MessageInfo));

			  if (error != OT_ERROR_NONE) while(1);

			  error = otCoapSendRequest(NULL,
						pOT_Message,
						&OT_MessageInfo,
						NULL,
						(void*) NULL);

			  // if error: free allocated message buffer if one was allocated
			  if (error != OT_ERROR_NONE && pOT_Message != NULL)
			  {
				otMessageFree(pOT_Message);
			  }

			  //HAL_Delay(10000);
			}while(false);

//	OT_Command = command;
//	  do
//	  {
//	    otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);
//	    otCoapHeaderAppendUriPathOptions(&OT_Header,"test");
//	    otCoapHeaderSetPayloadMarker(&OT_Header);
//
//	    pOT_Message = otCoapNewMessage(NULL, &OT_Header);
//	    if (pOT_Message == NULL)
//	    {
//	      break;
//	    }
//
//	    error = otMessageAppend(pOT_Message, &OT_Command, sizeof(command));
//	    if (error != OT_ERROR_NONE)
//	    {
//	      //APP_THREAD_Error(ERR_THREAD_COAP_APPEND,error);
//	      break;
//	    }
//
//	    memset(&OT_MessageInfo, 0, sizeof(OT_MessageInfo));
//	    OT_MessageInfo.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
//	    OT_MessageInfo.mPeerPort = OT_DEFAULT_COAP_PORT;
//	    otIp6AddressFromString("FF03::1", &OT_MessageInfo.mPeerAddr);
//
//	    error = otCoapSendRequest(NULL,
//	                              pOT_Message,
//	                              &OT_MessageInfo,
//	                              NULL,
//	                              NULL);
//	  } while (false);
}

/* USER CODE END FD_LOCAL_FUNCTIONS */

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
    UTIL_SEQ_WaitEvt(EVENT_SYNCHRO_BYPASS_IDLE);
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
  //HW_UART_Receive_IT(CFG_CLI_UART, aRxBuffer, 1U, RxCpltCallback);
}
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
  UTIL_SEQ_RegTask( 1<<CFG_TASK_SEND_CLI_TO_M0, UTIL_SEQ_RFU,Send_CLI_To_M0);
  #if (CFG_USB_INTERFACE_ENABLE != 0)
  #else
  //HW_UART_Receive_IT(CFG_CLI_UART, aRxBuffer, 1, RxCpltCallback);
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
   // HW_UART_Transmit_IT(CFG_CLI_UART, l_CliBuffer->cmdserial.cmd.payload, l_size, HostTxCb);
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

/* USER CODE BEGIN FD_WRAP_FUNCTIONS */

/* USER CODE END FD_WRAP_FUNCTIONS */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
