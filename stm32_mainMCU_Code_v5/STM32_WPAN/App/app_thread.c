/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : App/app_thread.c
 * Description        : Thread Application.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "cmsis_os.h"
#if (CFG_USB_INTERFACE_ENABLE != 0)
#include "vcp.h"
#include "vcp_conf.h"
#endif /* (CFG_USB_INTERFACE_ENABLE != 0) */

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "coap.h"
//#include "main.h"
#include "stm32wbxx_hal_gpio.h"
#include "lp5523.h"
#include "captivate_config.h"
#include "child_supervision.h"
#include "ip6.h"
#include "rtc.h"

#include "task.h"
#include "usbd_cdc_if.h"

#ifdef NETWORK_TEST
#include "network_test.h"
#endif

// TODO: fix the below include call to be relative
// C:\ST\STM32CubeIDE_1.5.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.7-2018-q2-update.win32_1.5.0.202011040924\tools\arm-none-eabi\include
//#include <C:\ST\STM32CubeIDE_1.5.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.7-2018-q2-update.win32_1.5.0.202011040924\tools\arm-none-eabi\include\stdio.h>
//#include <C:\ST\STM32CubeIDE_1.5.1\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.7-2018-q2-update.win32_1.5.0.202011040924\tools\arm-none-eabi\include\time.h>
#include "stdio.h"
#include "time.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/**
 * @brief  APP_THREAD Status structures definition
 */
typedef enum {
	APP_THREAD_OK = 0x00, APP_THREAD_ERROR = 0x01,
} APP_THREAD_StatusTypeDef;

typedef enum {
	COLOR_RED = 0, COLOR_BLUE = 1, COLOR_GREEN = 2
} ColorCode;

//struct tm
//{
//	int	tm_sec;
//    int	tm_min;
//    int	tm_hour;
//    int	tm_mday;
//    int	tm_mon;
//    int	tm_year;
//    int	tm_wday;
//    int	tm_yday;
//    int	tm_isdst;
//  #ifdef __TM_GMTOFF
//    long	__TM_GMTOFF;
//  #endif
//  #ifdef __TM_ZONE
//    const char *__TM_ZONE;
//  #endif
//};

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
#define C_SIZE_CMD_STRING       256U
#define C_PANID                 0x1234U
#define C_CHANNEL_NB            23U

/* FreeRtos stacks attributes */
const osThreadAttr_t ThreadMsgM0ToM4Process_attr = { .name =
		CFG_THREAD_MSG_M0_TO_M4_PROCESS_NAME, .attr_bits =
		CFG_THREAD_MSG_M0_TO_M4_PROCESS_ATTR_BITS, .cb_mem =
		CFG_THREAD_MSG_M0_TO_M4_PROCESS_CB_MEM, .cb_size =
		CFG_THREAD_MSG_M0_TO_M4_PROCESS_CB_SIZE, .stack_mem =
		CFG_THREAD_MSG_M0_TO_M4_PROCESS_STACK_MEM, .priority =
		CFG_THREAD_MSG_M0_TO_M4_PROCESS_PRIORITY, .stack_size =
		CFG_THREAD_MSG_M0_TO_M4_PROCESS_STACK_SIZE };

const osThreadAttr_t ThreadCliProcess_attr = { .name =
		CFG_THREAD_CLI_PROCESS_NAME, .attr_bits =
		CFG_THREAD_CLI_PROCESS_ATTR_BITS, .cb_mem =
		CFG_THREAD_CLI_PROCESS_CB_MEM,
		.cb_size = CFG_THREAD_CLI_PROCESS_CB_SIZE, .stack_mem =
				CFG_THREAD_CLI_PROCESS_STACK_MEM, .priority =
				CFG_THREAD_CLI_PROCESS_PRIORITY, .stack_size =
				CFG_THREAD_CLI_PROCESS_STACK_SIZE };

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define C_RESSOURCE             "light"

#define COAP_SEND_TIMEOUT               (1*100*1000/CFG_TS_TICK_VAL) /**< 1s */
#define THREAD_CHANGE_MODE_TIMEOUT      (1*1000*1000/CFG_TS_TICK_VAL) /**< 1s */
#define THREAD_LINK_POLL_PERIOD         (5*1000*1000/CFG_TS_TICK_VAL) /**< 5s */

#define OT_AUTOSTART_MODE 1 // Automatic OT start and COAP after reset
// ste to 0 for GRL testing

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
static void APP_THREAD_CheckWirelessFirmwareInfo(void);
static void APP_THREAD_DeviceConfig(void);
static void APP_THREAD_StateNotif(uint32_t NotifFlags, void *pContext);
static void APP_THREAD_TraceError(const char *pMess, uint32_t ErrCode);
#if (CFG_FULL_LOW_POWER == 0)
static void Send_CLI_To_M0(void);
#endif /* (CFG_FULL_LOW_POWER == 0) */
static void Send_CLI_Ack_For_OT(void);
static void HostTxCb(void);
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
#if (CFG_FULL_LOW_POWER == 0)
static void RxCpltCallback(void);
#endif /* (CFG_FULL_LOW_POWER == 0) */
#endif /* (CFG_USB_INTERFACE_ENABLE != 0) */

/* FreeRTos wrapper functions */
static void APP_THREAD_FreeRTOSProcessMsgM0ToM4Task(void *argument);
#if (CFG_FULL_LOW_POWER == 0)
static void APP_THREAD_FreeRTOSSendCLIToM0Task(void *argument);
#endif /* (CFG_FULL_LOW_POWER == 0) */

/* USER CODE BEGIN PFP */
//void APP_THREAD_SendCoapMsg(void);
static void APP_THREAD_SendCoapMulticastRequest(uint8_t command);
static void APP_THREAD_CoapRequestHandler(void *pContext, otCoapHeader *pHeader,
		otMessage *pMessage, const otMessageInfo *pMessageInfo);

static void APP_THREAD_SetSleepyEndDeviceMode(void);
static void APP_THREAD_CoapTimingElapsed(void);

#ifdef OTA_ENABLED
static void Delete_Sectors( void );

static void APP_THREAD_DummyReqHandler(void                * p_context,
    otCoapHeader        * pHeader,
    otMessage           * pMessage,
    const otMessageInfo * pMessageInfo);

static void APP_THREAD_CoapReqHandlerFuotaProvisioning(otCoapHeader        * pHeader,
    otMessage           * pMessage,
    const otMessageInfo * pMessageInfo);
static void APP_THREAD_ProvisioningRespSend(otCoapHeader    * pRequestHeader,
    const otMessageInfo * pMessageInfo);

static void APP_THREAD_CoapReqHandlerFuota(otCoapHeader        * pHeader,
    otMessage           * pMessage,
    const otMessageInfo * pMessageInfo);
static void APP_THREAD_CoapSendDataResponseFuota(otCoapHeader    * pRequestHeader,
    const otMessageInfo * pMessageInfo);

static void APP_THREAD_CoapReqHandlerFuotaParameters(otCoapHeader * pHeader,
    otMessage            * pMessage,
    const otMessageInfo  * pMessageInfo);
static void APP_THREAD_CoapSendRespFuotaParameters(otCoapHeader    * pRequestHeader,
    const otMessageInfo * pMessageInfo,
    uint8_t * pData);
static void APP_THREAD_PerformReset(void);
static void APP_THREAD_TimingElapsed(void);
static APP_THREAD_StatusTypeDef APP_THREAD_CheckDeviceCapabilities(void);
#endif

void stm32UID(uint8_t *uid);

static void APP_THREAD_SendDataResponse(void *message, uint16_t msgSize,
		otCoapHeader *pRequestHeader, const otMessageInfo *pMessageInfo);

//static void APP_THREAD_DummyReqHandler(void *p_context, otCoapHeader *pHeader, otMessage *pMessage,
//		const otMessageInfo *pMessageInfo);

//static void APP_THREAD_SendNextBuffer(void);
//static void APP_THREAD_SendCoapMsg(char* message, char* ipv6_addr, char* resource, otCoapType type);
//static void APP_THREAD_SendCoapUnicastRequest(char* message, char* ipv6_addr  , char* resource);
static void APP_THREAD_SendCoapUnicastRequest(char *message,
		uint8_t message_length, char *ipv6_addr, char *resource);

//static void APP_THREAD_CoapRequestHandler(void                * pContext,
//                                          otCoapHeader        * pHeader,
//                                          otMessage           * pMessage,
//                                          const otMessageInfo * pMessageInfo);

//static void APP_THREAD_CoapRequestHandler(otCoapHeader        * pHeader,
//                                  otMessage           * pMessage,
//                                  const otMessageInfo * pMessageInfo);

static void APP_THREAD_CoapLightsSimpleRequestHandler(void                * pContext,
		otCoapHeader        * pHeader,
		otMessage           * pMessage,
		const otMessageInfo * pMessageInfo);

static void APP_THREAD_CoapLightsComplexRequestHandler(void                * pContext,
		otCoapHeader        * pHeader,
		otMessage           * pMessage,
		const otMessageInfo * pMessageInfo);

static void APP_THREAD_CoapToggleLoggingRequestHandler(void                * pContext,
		otCoapHeader        * pHeader,
		otMessage           * pMessage,
		const otMessageInfo * pMessageInfo);

static void APP_THREAD_CoapBorderTimeRequestHandler(void                * pContext,
		otCoapHeader        * pHeader,
		otMessage           * pMessage,
		const otMessageInfo * pMessageInfo);

//static void APP_THREAD_CoapBorderPacketRequestHandler(otCoapHeader *pHeader, otMessage *pMessage,
//		const otMessageInfo *pMessageInfo);

static void APP_THREAD_CoapNodeInfoRequestHandler(void                * pContext,
		otCoapHeader        * pHeader,
		otMessage           * pMessage,
		const otMessageInfo * pMessageInfo);

static void APP_THREAD_CoapRespHandler_UpdateBorderRouter(otCoapHeader *pHeader,
		otMessage *pMessage, const otMessageInfo *pMessageInfo, otError Result);

static void APP_THREAD_SendCoapMsgForBorderSync(void *message, uint16_t msgSize,
		otIp6Address *ipv6_addr, char *resource, uint8_t request_ack,
		otCoapCode coapCode, uint8_t msgID);

void APP_THREAD_GetBorderRouterIP(void);

void APP_THREAD_SendMyInfo(void);
void APP_THREAD_UpdateBorderRouter(void);
void APP_THREAD_SyncWithBorderRouter(void);

/* USER CODE END PFP */

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
static __IO uint32_t CptReceiveMsgFromM0 = 0;
static volatile int FlagReceiveAckFromM0 = 0;

PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_TH_Config_t ThreadConfigBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ThreadOtCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t ThreadNotifRspEvtBuffer[sizeof(TL_PacketHeader_t)
		+ TL_EVT_HDR_SIZE + 255U];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t ThreadCliCmdBuffer;

static osThreadId_t OsTaskMsgM0ToM4Id; /* Task managing the M0 to M4 messaging        */
#if (CFG_FULL_LOW_POWER == 0)
static osThreadId_t OsTaskCliId; /* Task used to manage CLI comamnd             */
#endif /* (CFG_FULL_LOW_POWER == 0) */

/* USER CODE BEGIN PV */
static otCoapResource OT_Ressource = { C_RESSOURCE,
		APP_THREAD_CoapRequestHandler, "myCtx", NULL };
static otMessageInfo OT_MessageInfo = { 0 };
static uint8_t OT_Command = 0;
static otCoapHeader OT_Header = { 0 };
static uint8_t OT_ReceivedCommand = 0;
static otMessage *pOT_Message = NULL;
static otLinkModeConfig OT_LinkMode = { 0 };
static uint32_t sleepyEndDeviceFlag = FALSE;
static uint8_t sedCoapTimerID;
static uint8_t setThreadModeTimerID;

/* Debug */
static uint32_t DebugRxCoapCpt = 0;
static uint32_t DebugTxCoapCpt = 0;
//static otCoapResource OT_Light_Ressource = {C_LIGHT_RESSOURCE, APP_THREAD_DummyReqHandler, (void*)APP_THREAD_CoapRequestHandler, NULL};
//static otCoapResource OT_Lights_Complex_Ressource = { C_LIGHTS_SIMPLE_RESSOURCE, APP_THREAD_DummyReqHandler,
//		(void*) APP_THREAD_CoapLightsSimpleRequestHandler, NULL };
//static otCoapResource OT_Lights_Simple_Ressource = { C_LIGHTS_COMPLEX_RESSOURCE, APP_THREAD_DummyReqHandler,
//		(void*) APP_THREAD_CoapLightsComplexRequestHandler, NULL };
//static otCoapResource OT_Border_Time_Ressource = { C_BORER_TIME_RESSOURCE, APP_THREAD_DummyReqHandler,
//		(void*) APP_THREAD_CoapBorderTimeRequestHandler, NULL };
//static otCoapResource OT_Node_Info_Ressource = { C_NODE_INFO_RESSOURCE, APP_THREAD_DummyReqHandler,
//		(void*) APP_THREAD_CoapNodeInfoRequestHandler, NULL };
static otCoapResource OT_Lights_Complex_Ressource = { C_LIGHTS_SIMPLE_RESSOURCE,
		(void*) APP_THREAD_CoapLightsSimpleRequestHandler, "myLightS", NULL };
static otCoapResource OT_Lights_Simple_Ressource = { C_LIGHTS_COMPLEX_RESSOURCE,
		(void*) APP_THREAD_CoapLightsComplexRequestHandler, "myLightC", NULL };
static otCoapResource OT_Border_Time_Ressource = { C_BORER_TIME_RESSOURCE,
		(void*) APP_THREAD_CoapBorderTimeRequestHandler, "myTime", NULL };
static otCoapResource OT_Node_Info_Ressource = { C_NODE_INFO_RESSOURCE,
		(void*) APP_THREAD_CoapNodeInfoRequestHandler, "myInfo", NULL };

#ifdef BORDER_ROUTER_NODE
static otCoapResource OT_Border_Log_Ressource = { C_BORDER_PACKET_RESSOURCE, APP_THREAD_DummyReqHandler,
		(void*) APP_THREAD_CoapBorderPacketRequestHandler, NULL };
#endif

#ifndef DONGLE_CODE
//static otCoapResource OT_Toggle_Logging_Ressource = {C_TOGGLE_LOGGING_RESSOURCE, APP_THREAD_DummyReqHandler, (void*)APP_THREAD_CoapToggleLoggingRequestHandler, NULL};
static otCoapResource OT_Toggle_Logging_Ressource = {
		C_TOGGLE_LOGGING_RESSOURCE,
		(void*) APP_THREAD_CoapToggleLoggingRequestHandler, "myTogLog", NULL };
#endif

//otMessageInfo OT_MessageInfo = { 0 };
//otCoapHeader OT_Header = { 0 };
const char borderSyncResource[15] = "borderTime";
const char borderPacket[15] = "borderLog";
const char capTouchResource[15] = "capTouch";
const char capLocResource[15] = "capLoc";
const char nodeInfoResource[15] = "nodeInfo";

otIp6Address multicastAddr;

//static otCoapResource OT_Ressource = {C_RESSOURCE, APP_THREAD_CoapRequestHandler,"myCtx", NULL};

struct sendIP_struct {
	char node_type[12];
	char description[12];
	uint8_t uid[8];
	uint16_t RLOC;
};

//static uint8_t OT_Command = 0;
//static uint16_t OT_BufferIdRead = 1U;
//static uint16_t OT_BufferIdSend = 1U;

//static otMessage *pOT_Message = NULL;
//static otIp6Address   OT_PeerAddress = { .mFields.m8 = { 0 } };

const otMasterKey masterKey = { { 0x33, 0x33, 0x44, 0x44, 0x33, 0x33, 0x44,
		0x44, 0x33, 0x33, 0x44, 0x44, 0x33, 0x33, 0x44, 0x44 } };
const otExtendedPanId extendedPanId = { { 0x11, 0x11, 0x11, 0x11, 0x22, 0x22,
		0x22, 0x22 } };
const char networkName[20] = "PatrickNetwork";

//otIp6Address binary_border_ipv6;
//static char border_ipv6[100] = "fd11:22::994e:6ed7:263d:6187";
//static char border_ipv6[50] = 0x0;

//static char border_ipv6[100] = "FF03::1";
//const char border_ipv6[50] = "fd33:3333:3344:0:0:ff:fe00:e400";
otError error = OT_ERROR_NONE;
//static uint8_t OT_ReceivedCommand = 0;
//static char OT_BufferSend[20] = "TEST PAYLOAD";
volatile otIp6Address *myAddress;
volatile otMessageInfo *tempMessageInfo;
volatile uint16_t myRloc16;
volatile otNetifAddress *unicastAddresses;

volatile bool isEnabledIpv6;
//volatile otNetifMulticastAddress * multicastAddresses;
//volatile otIp6Address *  meshLocalEID;
//volatile otIp6Address * linkLocalIPV6;
volatile otMessageInfo *receivedMessage;

struct LogMessage logMessage;
ColorCode lightMessage;
union ColorComplex lightMessageComplex;

struct SystemCal borderRouter = { 0 };
struct SystemCal receivedSystemCal = { 0 };

struct sendIP_struct msgSendMyIP = { .node_type = NODE_TYPE, .description =
		NODE_DESCRIPTION, .RLOC = 0, .uid = 0 };

otIp6Address multicastAddr;

//otNetifMulticastAddress* multicastAddresses;
//otIp6Address* meshLocalEID;
//otIp6Address* linkLocalIPV6;

volatile otNetifMulticastAddress *multicastAddresses;
volatile otIp6Address *meshLocalEID;
volatile otIp6Address *linkLocalIPV6;

#ifdef OTA_ENABLED
static otCoapResource OT_RessourceFuotaProvisioning = {C_RESSOURCE_FUOTA_PROVISIONING, APP_THREAD_DummyReqHandler, (void*)APP_THREAD_CoapReqHandlerFuotaProvisioning, NULL};
static otCoapResource OT_RessourceFuotaParameters = {C_RESSOURCE_FUOTA_PARAMETERS, APP_THREAD_DummyReqHandler, (void*)APP_THREAD_CoapReqHandlerFuotaParameters, NULL};
static otCoapResource OT_RessourceFuotaSend = {C_RESSOURCE_FUOTA_SEND, APP_THREAD_DummyReqHandler, (void*)APP_THREAD_CoapReqHandlerFuota, NULL};

static uint8_t OT_Command = 0;
//static otMessageInfo OT_MessageInfo = {0};
//static otCoapHeader  OT_Header = {0};
//static otMessage* pOT_Message = NULL;

static uint8_t TimerID;
static uint32_t FuotaBinData_index = 0;
static uint64_t FuotaTransferArray[FUOTA_NUMBER_WORDS_64BITS] = {0};
static APP_THREAD_OtaContext_t OtaContext;
static uint32_t flash_current_offset = 0;
#endif
/* USER CODE END PV */

/* Functions Definition ------------------------------------------------------*/

void APP_THREAD_Init(void) {
	/* USER CODE BEGIN APP_THREAD_INIT_1 */
#ifdef OTA_ENABLED
	  /**
	   * This is a safe clear in case the engi bytes are not all written
	   * The error flag should be cleared before moving forward
	   */
	  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

	  APP_DBG("Delete_Sectors");
	  Delete_Sectors();
#endif

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

	/* USER CODE BEGIN APP_THREAD_INIT_TIMER */

	/* USER CODE END APP_THREAD_INIT_TIMER */

	/* Create the different FreeRTOS tasks requested to run this Thread application*/
	OsTaskMsgM0ToM4Id = osThreadNew(APP_THREAD_FreeRTOSProcessMsgM0ToM4Task,
			NULL, &ThreadMsgM0ToM4Process_attr);

	/* USER CODE BEGIN APP_THREAD_INIT_FREERTOS */

	/* USER CODE END APP_THREAD_INIT_FREERTOS */

	/* Configure the Thread device at start */
	APP_THREAD_DeviceConfig();

	/* USER CODE BEGIN APP_THREAD_INIT_2 */

	/* USER CODE END APP_THREAD_INIT_2 */
}

/**
 * @brief  Trace the error or the warning reported.
 * @param  ErrId :
 * @param  ErrCode
 * @retval None
 */
void APP_THREAD_Error(uint32_t ErrId, uint32_t ErrCode) {
	/* USER CODE BEGIN APP_THREAD_Error_1 */

	/* USER CODE END APP_THREAD_Error_1 */
	switch (ErrId) {
	case ERR_REC_MULTI_MSG_FROM_M0:
		APP_THREAD_TraceError("ERROR : ERR_REC_MULTI_MSG_FROM_M0 ", ErrCode);
		break;
	case ERR_THREAD_SET_STATE_CB:
		APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_STATE_CB ", ErrCode);
		break;
	case ERR_THREAD_SET_CHANNEL:
		APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_CHANNEL ", ErrCode);
		break;
	case ERR_THREAD_SET_PANID:
		APP_THREAD_TraceError("ERROR : ERR_THREAD_SET_PANID ", ErrCode);
		break;
	case ERR_THREAD_IPV6_ENABLE:
		APP_THREAD_TraceError("ERROR : ERR_THREAD_IPV6_ENABLE ", ErrCode);
		break;
	case ERR_THREAD_START:
		APP_THREAD_TraceError("ERROR: ERR_THREAD_START ", ErrCode);
		break;
	case ERR_THREAD_ERASE_PERSISTENT_INFO:
		APP_THREAD_TraceError("ERROR : ERR_THREAD_ERASE_PERSISTENT_INFO ",
				ErrCode);
		break;
	case ERR_THREAD_CHECK_WIRELESS:
		APP_THREAD_TraceError("ERROR : ERR_THREAD_CHECK_WIRELESS ", ErrCode);
		break;
		/* USER CODE BEGIN APP_THREAD_Error_2 */

		/* USER CODE END APP_THREAD_Error_2 */
	default:
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
static void APP_THREAD_DeviceConfig(void) {
	otError error;
#ifndef DYNAMIC_MODE
	error = otInstanceErasePersistentInfo(NULL);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_ERASE_PERSISTENT_INFO, error);
	}
#endif
	otInstanceFinalize(NULL);
	otInstanceInitSingle();
	error = otSetStateChangedCallback(NULL, APP_THREAD_StateNotif, NULL);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_SET_STATE_CB, error);
	}
	error = otLinkSetChannel(NULL, C_CHANNEL_NB);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_SET_CHANNEL, error);
	}
	error = otLinkSetPanId(NULL, C_PANID);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_SET_PANID, error);
	}
	error = otIp6SetEnabled(NULL, true);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_IPV6_ENABLE, error);
	}
	error = otThreadSetEnabled(NULL, true);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_START, error);
	}

	/* USER CODE BEGIN DEVICECONFIG */
	error = otThreadSetEnabled(NULL, false);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_START, error);
	}

	error = otIp6SetEnabled(NULL, false);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_IPV6_ENABLE, error);
	}

	error = otPlatRadioSetTransmitPower(NULL, TRANSMIT_POWER);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_SET_CHANNEL, error);
	}

	otThreadSetChildTimeout(NULL, CHILD_SUPERVISION_TIMEOUT);

//	otChildSupervisionSetCheckTimeout(NULL, CHILD_SUPERVISION_TIMEOUT);
//	otChildSupervisionSetInterval(NULL, CHILD_SUPERVISION_INTERVAL);

//   error = otIp6AddressFromString("ff12::1", &multicastAddr);
//   error = otIp6SubscribeMulticastAddress(NULL, &multicastAddr);

	error = otThreadSetMasterKey(NULL, &masterKey);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_SET_CHANNEL, error);
	}

	error = otThreadSetNetworkName(NULL, networkName);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_SET_CHANNEL, error);
	}
	error = otThreadSetExtendedPanId(NULL, &extendedPanId);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_SET_CHANNEL, error);
	}

	error = otIp6SetEnabled(NULL, true);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_IPV6_ENABLE, error);
	}

	error = otThreadSetEnabled(NULL, true);
	if (error != OT_ERROR_NONE) {
		APP_THREAD_Error(ERR_THREAD_START, error);
	}

	error = otCoapStart(NULL, OT_DEFAULT_COAP_PORT);
	//  error = otCoapAddResource(NULL, &OT_Light_Ressource);
	error = otCoapAddResource(NULL, &OT_Lights_Complex_Ressource);
	error = otCoapAddResource(NULL, &OT_Lights_Simple_Ressource);
	error = otCoapAddResource(NULL, &OT_Border_Time_Ressource);
	error = otCoapAddResource(NULL, &OT_Node_Info_Ressource);
#ifdef BORDER_ROUTER_NODE
	error = otCoapAddResource(NULL, &OT_Border_Log_Ressource);
#endif
#ifdef OTA_ENABLED
	  error = otCoapAddResource(NULL, &OT_RessourceFuotaProvisioning);
	  error = otCoapAddResource(NULL, &OT_RessourceFuotaParameters);
	  error = otCoapAddResource(NULL, &OT_RessourceFuotaSend);
#endif
#ifndef DONGLE_CODE
	error = otCoapAddResource(NULL, &OT_Toggle_Logging_Ressource);
#endif
	// set default multicast address for border router
//    otIp6AddressFromString("ff03::1", &borderRouter.ipv6);
	otIp6AddressFromString("ff03::1", &multicastAddr);
	memcpy(&borderRouter.ipv6, &multicastAddr, sizeof(multicastAddr));

	// set UID in local state variable
//	msgSendMyIP.uid = (uint32_t)  DBGMCU->IDCODE;
//	 = UID64_BASE;
	stm32UID(msgSendMyIP.uid);

	/* USER CODE END DEVICECONFIG */
}

/**
 * @brief Thread notification when the state changes.
 * @param  aFlags  : Define the item that has been modified
 *         aContext: Context
 *
 * @retval None
 */
static void APP_THREAD_StateNotif(uint32_t NotifFlags, void *pContext) {
	/* Prevent unused argument(s) compilation warning */
	UNUSED(pContext);

	/* USER CODE BEGIN APP_THREAD_STATENOTIF */

	/* USER CODE END APP_THREAD_STATENOTIF */

	if ((NotifFlags & (uint32_t) OT_CHANGED_THREAD_ROLE)
			== (uint32_t) OT_CHANGED_THREAD_ROLE) {
		switch (otThreadGetDeviceRole(NULL)) {
		case OT_DEVICE_ROLE_DISABLED:
			/* USER CODE BEGIN OT_DEVICE_ROLE_DISABLED */
			borderRouter.epoch = 0;
			/* USER CODE END OT_DEVICE_ROLE_DISABLED */
			break;
		case OT_DEVICE_ROLE_DETACHED:
			/* USER CODE BEGIN OT_DEVICE_ROLE_DETACHED */
			borderRouter.epoch = 0;
			/* USER CODE END OT_DEVICE_ROLE_DETACHED */
			break;
		case OT_DEVICE_ROLE_CHILD:
			/* USER CODE BEGIN OT_DEVICE_ROLE_CHILD */
#ifdef DONGLE_CODE
			BSP_LED_On(LED_GREEN);
			BSP_LED_Off(LED_RED);
			BSP_LED_Off(LED_BLUE);
#endif
			APP_THREAD_UpdateBorderRouter();
			/* USER CODE END OT_DEVICE_ROLE_CHILD */
			break;
		case OT_DEVICE_ROLE_ROUTER:
			/* USER CODE BEGIN OT_DEVICE_ROLE_ROUTER */
#ifdef DONGLE_CODE
			BSP_LED_Off(LED_GREEN);
			BSP_LED_Off(LED_RED);
			BSP_LED_On(LED_BLUE);
#endif
			APP_THREAD_UpdateBorderRouter();
			/* USER CODE END OT_DEVICE_ROLE_ROUTER */
			break;
		case OT_DEVICE_ROLE_LEADER:
			/* USER CODE BEGIN OT_DEVICE_ROLE_LEADER */
#ifdef DONGLE_CODE
			BSP_LED_On(LED_GREEN);
			BSP_LED_Off(LED_RED);
			BSP_LED_On(LED_BLUE);
#endif
			APP_THREAD_UpdateBorderRouter();
			/* USER CODE END OT_DEVICE_ROLE_LEADER */
			break;
		default:
			/* USER CODE BEGIN DEFAULT */
			APP_THREAD_UpdateBorderRouter();
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
static void APP_THREAD_TraceError(const char *pMess, uint32_t ErrCode) {
	/* USER CODE BEGIN TRACE_ERROR */

	/* USER CODE END TRACE_ERROR */
}

/**
 * @brief Check if the Coprocessor Wireless Firmware loaded supports Thread
 *        and display associated informations
 * @param  None
 * @retval None
 */
static void APP_THREAD_CheckWirelessFirmwareInfo(void) {
	WirelessFwInfo_t wireless_info_instance;
	WirelessFwInfo_t *p_wireless_info = &wireless_info_instance;

	if (SHCI_GetWirelessFwInfo(p_wireless_info) != SHCI_Success) {
		APP_THREAD_Error((uint32_t) ERR_THREAD_CHECK_WIRELESS,
				(uint32_t) ERR_INTERFACE_FATAL);
	} else {
		APP_DBG("**********************************************************");
		APP_DBG("WIRELESS COPROCESSOR FW:");
		/* Print version */
		APP_DBG("VERSION ID = %d.%d.%d", p_wireless_info->VersionMajor,
				p_wireless_info->VersionMinor, p_wireless_info->VersionSub);

		switch (p_wireless_info->StackType) {
		case INFO_STACK_TYPE_THREAD_FTD:
			APP_DBG("FW Type : Thread FTD")
			;
			break;
		case INFO_STACK_TYPE_THREAD_MTD:
			APP_DBG("FW Type : Thread MTD")
			;
			break;
		case INFO_STACK_TYPE_BLE_THREAD_FTD_STATIC:
			APP_DBG("FW Type : Static Concurrent Mode BLE/Thread")
			;
			break;
		default:
			/* No Thread device supported ! */
			APP_THREAD_Error((uint32_t) ERR_THREAD_CHECK_WIRELESS,
					(uint32_t) ERR_INTERFACE_FATAL);
			break;
		}
		APP_DBG("**********************************************************");
	}
}

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
 *************************************************************/
static void APP_THREAD_FreeRTOSProcessMsgM0ToM4Task(void *argument) {
	UNUSED(argument);
	for (;;) {
		/* USER CODE BEGIN APP_THREAD_FREERTOS_PROCESS_MSG_M0_TO_M4_1 */

		/* USER END END APP_THREAD_FREERTOS_PROCESS_MSG_M0_TO_M4_1 */
		osThreadFlagsWait(1, osFlagsWaitAll, osWaitForever);
		APP_THREAD_ProcessMsgM0ToM4();
		/* USER CODE BEGIN APP_THREAD_FREERTOS_PROCESS_MSG_M0_TO_M4_2 */

		/* USER END END APP_THREAD_FREERTOS_PROCESS_MSG_M0_TO_M4_2 */
	}
}

#if (CFG_FULL_LOW_POWER == 0)
static void APP_THREAD_FreeRTOSSendCLIToM0Task(void *argument) {
	UNUSED(argument);
	for (;;) {
		/* USER CODE BEGIN APP_THREAD_FREERTOS_SEND_CLI_TO_M0_1 */

		/* USER END END APP_THREAD_FREERTOS_SEND_CLI_TO_M0_1 */
		osThreadFlagsWait(1, osFlagsWaitAll, osWaitForever);
		Send_CLI_To_M0();
		/* USER CODE BEGIN APP_THREAD_FREERTOS_SEND_CLI_TO_M0_2 */

		/* USER END END APP_THREAD_FREERTOS_SEND_CLI_TO_M0_2 */
	}
}
#endif /* (CFG_FULL_LOW_POWER == 0) */

/* USER CODE BEGIN FREERTOS_WRAPPER_FUNCTIONS */
void stm32UID(uint8_t *uid) {
	for (uint8_t i = 0; i < 12; i++) {
		uid[i] = *(volatile uint8_t*) (UID_BASE + i);
	}
}
/* USER CODE END FREERTOS_WRAPPER_FUNCTIONS */

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

/**
 * @brief Main entry point for the Thread Application
 * @param  none
 * @retval None
 */
void APP_THREAD_Init_Dyn_1(void) {
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
//  APP_THREAD_Init_UART_CLI();
	/* Send Thread start system cmd to M0 */
	ThreadInitStatus = SHCI_C2_THREAD_Init();

	/* Prevent unused argument(s) compilation warning */
	UNUSED(ThreadInitStatus);

	// TODO: added this call and removed the lines after since (I think) they are unnecessary
	OsTaskMsgM0ToM4Id = osThreadNew(APP_THREAD_FreeRTOSProcessMsgM0ToM4Task,
			NULL, &ThreadMsgM0ToM4Process_attr);

//  /* Register task */
//  /* Create the different tasks */
//  UTIL_SEQ_RegTask( 1<<(uint32_t)CFG_TASK_MSG_FROM_M0_TO_M4, UTIL_SEQ_RFU, APP_THREAD_ProcessMsgM0ToM4);
//  UTIL_SEQ_RegTask( 1<<(uint32_t)CFG_TASK_COAP_MSG_BUTTON, UTIL_SEQ_RFU, APP_THREAD_SendCoapMsg);
//
//  UTIL_SEQ_RegTask( 1<<(uint32_t)CFG_TASK_COAP_SEND_MSG, UTIL_SEQ_RFU,APP_THREAD_SendCoapMsg);
//  UTIL_SEQ_RegTask( 1<<(uint32_t)CFG_TASK_SET_THREAD_MODE, UTIL_SEQ_RFU,APP_THREAD_SetSleepyEndDeviceMode);
}

void APP_THREAD_Init_Dyn_2(void) {
	/* Initialize and configure the Thread device*/
	APP_THREAD_DeviceConfig();

	//TODO: removed below as per Thread-only FreeRTOS example
	/**
	 * Create timer to handle COAP request sending
	 */
//  HW_TS_Create(CFG_TIM_PROC_ID_ISR, &sedCoapTimerID, hw_ts_Repeated, APP_THREAD_CoapTimingElapsed);
	/* Allow the 800_15_4 IP to enter in low power mode */
}

void APP_THREAD_Stop(void) {
	otError error;
	/* STOP THREAD */
	error = otThreadSetEnabled(NULL, false);
	if (error != OT_ERROR_NONE) {
//     APP_THREAD_Error(ERR_THREAD_STOP,error);
	}
}

void APP_THREAD_CleanCallbacks(void) {
	otRemoveStateChangeCallback(NULL, APP_THREAD_StateNotif, NULL);
//  otCoapRemoveResource(NULL, &OT_Ressource);

//  /* Remove Timers if any */
//  HW_TS_Delete(setThreadModeTimerID);
//  HW_TS_Delete(sedCoapTimerID);
}

#ifdef OTA_ENABLED
static void Delete_Sectors( void )
{
  /**
   * The number of sectors to erase is read from SRAM1.
   * It shall be checked whether the number of sectors to erase does not overlap on the secured Flash
   * The limit can be read from the SFSA option byte which provides the first secured sector address.
   */

  uint32_t page_error;
  FLASH_EraseInitTypeDef p_erase_init;
  uint32_t first_secure_sector_idx;

  first_secure_sector_idx = (READ_BIT(FLASH->SFR, FLASH_SFR_SFSA) >> FLASH_SFR_SFSA_Pos);

  p_erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
  p_erase_init.Page = *((uint8_t*) SRAM1_BASE + 1);
  if(p_erase_init.Page < (CFG_APP_START_SECTOR_INDEX - 1))
  {
    /**
     * Something has been wrong as there is no case we should delete the BLE_Ota application
     * Reboot on the firmware application
     */
    *(uint8_t*)SRAM1_BASE = CFG_REBOOT_ON_FW_APP;
    NVIC_SystemReset();
  }
  p_erase_init.NbPages = *((uint8_t*) SRAM1_BASE + 2);

  if ((p_erase_init.Page + p_erase_init.NbPages) > first_secure_sector_idx)
  {
    p_erase_init.NbPages = first_secure_sector_idx - p_erase_init.Page;
  }

  APP_DBG("SFSA Option Bytes set to sector = %d (0x080%x)", first_secure_sector_idx, first_secure_sector_idx*4096);
  APP_DBG("Erase FLASH Memory from sector %d (0x080%x) to sector %d (0x080%x)", p_erase_init.Page, p_erase_init.Page*4096, p_erase_init.NbPages+p_erase_init.Page, (p_erase_init.NbPages+p_erase_init.Page)*4096);

  HAL_FLASH_Unlock();

  HAL_FLASHEx_Erase(&p_erase_init, &page_error);

  HAL_FLASH_Lock();

  return;
}
#endif

static void APP_THREAD_DummyRespHandler(void *p_context, otCoapHeader *pHeader,
		otMessage *pMessage, const otMessageInfo *pMessageInfo, otError Result) {
	/* Prevent unused argument(s) compilation warning */
	UNUSED(p_context);
	UNUSED(pHeader);
	UNUSED(pMessage);
	UNUSED(pMessageInfo);
	UNUSED(Result);
}

volatile uint16_t test_num;
static void APP_THREAD_CoapRespHandler_UpdateBorderRouter(otCoapHeader *pHeader,
		otMessage *pMessage, const otMessageInfo *pMessageInfo, otError Result) {

//	taskENTER_CRITICAL();
//	test_num = otMessageGetLength(pMessage);
//	test_num = otMessageRead(pMessage, otMessageGetOffset(pMessage), &receivedSystemCal, sizeof(receivedSystemCal));

	if (otMessageRead(pMessage, otMessageGetOffset(pMessage),
			&receivedSystemCal, sizeof(receivedSystemCal))
			== sizeof(receivedSystemCal)) {

		// if successful, update border router state variable
		memcpy(&borderRouter, &receivedSystemCal, sizeof(receivedSystemCal));

		// update the onboard RTC unix time
		updateRTC(borderRouter.epoch);
	}
//	taskEXIT_CRITICAL();
}

/**
 * @brief Dummy request handler
 * @param
 * @retval None
 */
//static void APP_THREAD_DummyReqHandler(void *p_context, otCoapHeader *pHeader, otMessage *pMessage,
//		const otMessageInfo *pMessageInfo) {
//	tempMessageInfo = pMessageInfo;
//	receivedMessage = (otMessageInfo*) pMessage;
//}
#ifdef OTA_ENABLED
/**
 * @brief Handler called when the server receives a COAP request.
 *
 * @param pHeader : Header
 * @param pMessage : Message
 * @param pMessageInfo : Message information
 * @retval None
 */
static void APP_THREAD_CoapReqHandlerFuotaProvisioning(otCoapHeader * pHeader,
    otMessage            * pMessage,
    const otMessageInfo  * pMessageInfo)
{
  APP_DBG(" Received CoAP request on FUOTA_PROVISIONING ressource");

  if (otCoapHeaderGetType(pHeader) == OT_COAP_TYPE_NON_CONFIRMABLE &&
      otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_GET)
  {
    OT_MessageInfo = *pMessageInfo;
    memset(&OT_MessageInfo.mSockAddr, 0, sizeof(OT_MessageInfo.mSockAddr));
    APP_THREAD_ProvisioningRespSend(pHeader, pMessageInfo);
  }
}

/**
 * @brief This function acknowledges the data reception by sending an ACK
 *    back to the sender.
 * @param  pRequestHeader coap header
 * @param  pMessageInfo message info pointer
 * @retval None
 */
static void APP_THREAD_ProvisioningRespSend(otCoapHeader    * pRequestHeader,
    const otMessageInfo * pMessageInfo)
{
  otError  error = OT_ERROR_NONE;

  do{
    APP_DBG("Provisiong: Send CoAP response");
    otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_CONTENT);
    otCoapHeaderSetToken(&OT_Header,
        otCoapHeaderGetToken(pRequestHeader),
        otCoapHeaderGetTokenLength(pRequestHeader));
    otCoapHeaderSetPayloadMarker(&OT_Header);

    pOT_Message = otCoapNewMessage(NULL, &OT_Header);
    if (pOT_Message == NULL)
    {
      APP_THREAD_Error(ERR_THREAD_COAP_APPEND_MSG, error);
      break;
    }

    error = otMessageAppend(pOT_Message, &OT_Command, sizeof(OT_Command));
    if (error != OT_ERROR_NONE)
    {
      APP_THREAD_Error(ERR_THREAD_COAP_APPEND_MSG, error);
    }

    error = otMessageAppend(pOT_Message, otThreadGetMeshLocalEid(NULL), sizeof(otIp6Address));
    if (error != OT_ERROR_NONE)
    {
      APP_THREAD_Error(ERR_THREAD_COAP_APPEND_MSG, error);
      break;
    }

    error = otCoapSendResponse(NULL, pOT_Message, pMessageInfo);
    if (error != OT_ERROR_NONE && pOT_Message != NULL)
    {
      otMessageFree(pOT_Message);
      APP_THREAD_Error(ERR_THREAD_COAP_DATA_RESPONSE,error);
    }
  }while(false);
}

/**
 * @brief Handler called when the server receives a COAP request.
 *
 * @param pHeader : Header
 * @param pMessage : Message
 * @param pMessageInfo : Message information
 * @retval None
 */
static void APP_THREAD_CoapReqHandlerFuotaParameters(otCoapHeader * pHeader,
    otMessage            * pMessage,
    const otMessageInfo  * pMessageInfo)
{
  if (otMessageRead(pMessage, otMessageGetOffset(pMessage), &OtaContext, sizeof(OtaContext)) != sizeof(OtaContext))
  {
    APP_THREAD_Error(ERR_THREAD_MESSAGE_READ, 0);
  }

  /* Display Ota_Context values */
  if(OtaContext.file_type == APP_THREAD_OTA_FILE_TYPE_FW_APP)
  {
    APP_DBG("FUOTA_PARAMETERS: File Type set to : FW_APP");
  }
  else  if (OtaContext.file_type == APP_THREAD_OTA_FILE_TYPE_FW_COPRO_WIRELESS)
  {
    APP_DBG("FUOTA_PARAMETERS: File Type set to : FW_COPRO_WIRELESS");
  }
  else
  {
    APP_DBG("FUOTA_PARAMETERS: File Type not recognized");
    APP_THREAD_Error(ERR_THREAD_FUOTA_FILE_TYPE_NOT_RECOGNIZED, 0);
  }

  APP_DBG("FUOTA_PARAMETERS: Binary Size = 0x%x", OtaContext.binary_size);
  APP_DBG("FUOTA_PARAMETERS: Address = 0x%x", OtaContext.base_address);
  APP_DBG("FUOTA_PARAMETERS: Magic Keyword = 0x%x", OtaContext.magic_keyword);

  /* Check if Device can be updated with Fuota Server request */
  if (APP_THREAD_CheckDeviceCapabilities() == APP_THREAD_OK)
  {
    OT_Command = APP_THREAD_OK;
    // TODO : add LED toggling here
//    HW_TS_Start(TimerID, (uint32_t)LED_TOGGLE_TIMING);
  }
  else
  {
    OT_Command = APP_THREAD_ERROR;
    APP_DBG("WARNING: Current Device capabilities cannot handle FUOTA. Check memory size available!");
  }
  /* If Message is Confirmable, send response */
  if (otCoapHeaderGetType(pHeader) == OT_COAP_TYPE_CONFIRMABLE)
  {
    APP_THREAD_CoapSendRespFuotaParameters(pHeader, pMessageInfo, &OT_Command);
  }
}

static APP_THREAD_StatusTypeDef APP_THREAD_CheckDeviceCapabilities(void)
{
  APP_THREAD_StatusTypeDef status = APP_THREAD_OK;

  /* Get Flash memory size available to copy binary from Server */
  uint32_t first_sector_idx;
  uint32_t first_secure_sector_idx;
  uint32_t free_sectors;
  uint32_t free_size;

  APP_DBG("Check Device capabilities");

  first_secure_sector_idx = (READ_BIT(FLASH->SFR, FLASH_SFR_SFSA) >> FLASH_SFR_SFSA_Pos);
  APP_DBG("SFSA Option Bytes set to sector = %d (0x080%x)", first_secure_sector_idx, first_secure_sector_idx*4096);

  first_sector_idx = *((uint8_t*) SRAM1_BASE + 1);
  if (first_sector_idx == 0)
  {
    APP_DBG("ERROR : SRAM1_BASE + 1 == 0");
    first_sector_idx = CFG_APP_START_SECTOR_INDEX;
  }
  APP_DBG("First available sector = %d (0x080%x)", first_sector_idx, first_sector_idx*4096);

  free_sectors = first_secure_sector_idx - first_sector_idx;
  free_size = free_sectors*4096;

  APP_DBG("free_sectors = %d , -> %d bytes of FLASH Free", free_sectors, free_size);

  APP_DBG("Server requests    : %d bytes", OtaContext.binary_size);
  APP_DBG("Client Free memory : %d bytes", free_size);

  if (free_size < OtaContext.binary_size)
  {
    status = APP_THREAD_ERROR;
    APP_DBG("WARNING: Not enough Free Flash Memory available to download binary from Server!");
  }
  else
  {
    APP_DBG("Device contains enough Flash Memory to download binary");
  }

  return status;
}

/**
 * @brief Handler called when the server receives a COAP request.
 *
 * @param pHeader : Header
 * @param pMessage : Message
 * @param pMessageInfo : Message information
 * @retval None
 */
static void APP_THREAD_CoapReqHandlerFuota(otCoapHeader * pHeader,
    otMessage            * pMessage,
    const otMessageInfo  * pMessageInfo)
{
  bool l_end_full_bin_transfer = FALSE;
  uint32_t flash_index = 0;
  uint64_t l_read64 = 0;

  if (otMessageRead(pMessage, otMessageGetOffset(pMessage), &FuotaTransferArray, FUOTA_PAYLOAD_SIZE) != FUOTA_PAYLOAD_SIZE)
  {
    APP_THREAD_Error(ERR_THREAD_MESSAGE_READ, 0);
  }

  /* Test if magic Keyword is in FuotaBinData */
  for (int index = 0; index < FUOTA_NUMBER_WORDS_64BITS; ++index) {
    if((FuotaTransferArray[index] & 0x00000000FFFFFFFF) == OtaContext.magic_keyword)
    {
      APP_DBG("1 - FUOTA_MAGIC_KEYWORD found at flash_current_offset = %d", (FuotaBinData_index + index)*8);
      l_end_full_bin_transfer = TRUE;
    }
    else
    if (((FuotaTransferArray[index] & 0xFFFFFFFF00000000) >> 32) == OtaContext.magic_keyword)
    {
      APP_DBG("2 - FUOTA_MAGIC_KEYWORD found at flash_current_offset = %d", (FuotaBinData_index + index)*8);
      l_end_full_bin_transfer = TRUE;
    }
  }

  FuotaBinData_index += FUOTA_NUMBER_WORDS_64BITS;

  /* Write to Flash Memory */
  for(flash_index = 0; flash_index < FUOTA_NUMBER_WORDS_64BITS; flash_index++)
  {
    while( LL_HSEM_1StepLock( HSEM, CFG_HW_FLASH_SEMID ) );
    HAL_FLASH_Unlock();
    while(LL_FLASH_IsActiveFlag_OperationSuspended());

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
        OtaContext.base_address + flash_current_offset,
        FuotaTransferArray[flash_index]) == HAL_OK)
    {
      /* Read back value for verification */
      l_read64 = *(uint64_t*)(OtaContext.base_address + flash_current_offset);
      if(l_read64 != FuotaTransferArray[flash_index])
      {
        APP_DBG("FLASH: Comparison failed l_read64 = 0x%jx / ram_array = 0x%jx", l_read64, FuotaTransferArray[flash_index])
                  APP_THREAD_Error(ERR_THREAD_MSG_COMPARE_FAILED,0);
      }
    }
    else
    {
      APP_DBG("HAL_FLASH_Program FAILED at flash_index = %d", flash_index)
      APP_THREAD_Error(ERR_THREAD_FLASH_PROGRAM,0);
    }

    HAL_FLASH_Lock();
    LL_HSEM_ReleaseLock( HSEM, CFG_HW_FLASH_SEMID, 0 );

    flash_current_offset += 8;
  }

  /* If Message is Confirmable, send response */
  if (otCoapHeaderGetType(pHeader) == OT_COAP_TYPE_CONFIRMABLE)
  {
    APP_THREAD_CoapSendDataResponseFuota(pHeader, pMessageInfo);
  }

  if(l_end_full_bin_transfer == TRUE)
  {
	  // TODO : add FREERTOS task flag enable here

	  APP_THREAD_PerformReset();

//    UTIL_SEQ_SetTask(TASK_FUOTA_RESET, CFG_SCH_PRIO_0);
  }
}

/**
 * @brief This function acknowledges the data reception by sending an ACK
 *    back to the sender.
 * @param  pRequestHeader coap header
 * @param  pMessageInfo message info pointer
 * @retval None
 */
static void APP_THREAD_CoapSendDataResponseFuota(otCoapHeader    * pRequestHeader,
    const otMessageInfo * pMessageInfo)
{
  otError  error = OT_ERROR_NONE;
  static otCoapHeader  OT_Header = {0};

  do{
    otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CHANGED);
    otCoapHeaderSetMessageId(&OT_Header, otCoapHeaderGetMessageId(pRequestHeader));
    otCoapHeaderSetToken(&OT_Header,
        otCoapHeaderGetToken(pRequestHeader),
        otCoapHeaderGetTokenLength(pRequestHeader));

    pOT_Message = otCoapNewMessage(NULL, &OT_Header);
    if (pOT_Message == NULL)
    {
      APP_THREAD_Error(ERR_THREAD_COAP_APPEND_MSG, error);
      break;
    }

    error = otCoapSendResponse(NULL, pOT_Message, pMessageInfo);
    if (error != OT_ERROR_NONE && pOT_Message != NULL)
    {
      otMessageFree(pOT_Message);
      APP_THREAD_Error(ERR_THREAD_COAP_DATA_RESPONSE,error);
    }
  }while(false);
}

/**
 * @brief This function acknowledges the data reception by sending an ACK
 *    back to the sender.
 * @param  pRequestHeader coap header
 * @param  pMessageInfo message info pointer
 * @param  pMessage     message pointer
 * @retval None
 */
static void APP_THREAD_CoapSendRespFuotaParameters(otCoapHeader    * pRequestHeader,
    const otMessageInfo * pMessageInfo,
    uint8_t * pData)
{
  otError  error = OT_ERROR_NONE;
  static otCoapHeader  OT_Header = {0};
  uint8_t data = *pData;
  APP_DBG("APP_THREAD_CoapSendRespFuotaParameters data = %d", data);

  do{
    APP_DBG("FUOTA: Send CoAP response for Fuota Parameters");
    otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_CONTENT);
    otCoapHeaderSetToken(&OT_Header,
        otCoapHeaderGetToken(pRequestHeader),
        otCoapHeaderGetTokenLength(pRequestHeader));
    otCoapHeaderSetPayloadMarker(&OT_Header);

    pOT_Message = otCoapNewMessage(NULL, &OT_Header);
    if (pOT_Message == NULL)
    {
      APP_THREAD_Error(ERR_THREAD_COAP_APPEND_MSG, error);
      break;
    }

    error = otMessageAppend(pOT_Message, &data, sizeof(data));
    if (error != OT_ERROR_NONE)
    {
      APP_THREAD_Error(ERR_THREAD_COAP_APPEND_MSG, error);
    }

    error = otCoapSendResponse(NULL, pOT_Message, pMessageInfo);
    if (error != OT_ERROR_NONE && pOT_Message != NULL)
    {
      otMessageFree(pOT_Message);
      APP_THREAD_Error(ERR_THREAD_COAP_DATA_RESPONSE,error);
    }
  }while(false);
}

/**
 * @brief Task responsible for the reset at the end of OTA transfer.
 * @param  None
 * @retval None
 */
static void APP_THREAD_PerformReset(void)
{
  APP_DBG("*******************************************************");
  APP_DBG(" FUOTA_CLIENT : END OF TRANSFER COMPLETED");
  /* Stop Toggling of the LED */
//  HW_TS_Stop(TimerID);
//  BSP_LED_On(LED1);

  /* Insert delay to make sure CoAP response has been sent */
  HAL_Delay(200);

  if(OtaContext.file_type == APP_THREAD_OTA_FILE_TYPE_FW_APP)
  {
    APP_DBG("  --> Request to reboot on FW Application");
    APP_DBG("*******************************************************");
    /**
     * Reboot on FW Application
     */
    *(uint8_t*)SRAM1_BASE = CFG_REBOOT_ON_FW_APP;
    NVIC_SystemReset();
  }
  else if(OtaContext.file_type == APP_THREAD_OTA_FILE_TYPE_FW_COPRO_WIRELESS)
  {
    APP_DBG("  --> Request to reboot on FUS");
    APP_DBG("*******************************************************");
    /**
     * Wireless firmware update is requested
     * Request CPU2 to reboot on FUS by sending two FUS command
     */
    SHCI_C2_FUS_GetState( NULL );
    SHCI_C2_FUS_GetState( NULL );
    while(1)
    {
      HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    }
  }
  else
  {
    APP_DBG("APP_THREAD_PerformReset: OtaContext.file_type not recognized");
    APP_THREAD_Error(ERR_THREAD_FUOTA_FILE_TYPE_NOT_RECOGNIZED,0);
  }

}

static void APP_THREAD_TimingElapsed(void)
{
  BSP_LED_Toggle(LED1);
}

#endif

//void APP_THREAD_GetBorderRouterIP(){
//	APP_THREAD_SendCoapUnicastRequest(NULL, NULL, MULICAST_FTD_BORDER_ROUTER, borderSyncResource);
//}

//char msgSendMyIP[5] = "cal";
////otIp6Address* test_ip;
//otNeighborInfo test_info_neighbor;
//otNeighborInfoIterator test_neighbor_iterator;
//otError error;
//
//volatile uint8_t random_var = 0;
//volatile bool state = 1;
//volatile otNetifMulticastAddress *test_addr;
//volatile otExtAddress *test_ext_addr;
//volatile otExtAddress test_1;
//volatile otExtAddress test_2;

void APP_THREAD_UpdateBorderRouter() {

	// if border router IP is still multicast (ff03::1), attempt to sync
	if (otIp6IsAddressEqual(&multicastAddr, &borderRouter.ipv6)) {
		APP_THREAD_SyncWithBorderRouter();
		APP_THREAD_SendMyInfo();
	}

	// send IP to border router
	//APP_THREAD_SendMyInfo();
}

// send a GET request to border router via multicast
void APP_THREAD_SyncWithBorderRouter() {
	APP_THREAD_SendCoapMsgForBorderSync(NULL, 0, &multicastAddr,
			(char*) borderSyncResource, NO_ACK, OT_COAP_CODE_GET, 1U);
}

void APP_THREAD_SendMyInfo() {
//	msgSendMyIP.uid = DBGMCU->IDCODE;
//	const otIp6Address *test_ip = otThreadGetLinkLocalIp6Address(NULL);
//	if(test_ip) random_var++;
//	state = otIp6IsMulticastPromiscuousEnabled(NULL);
//	test_addr = otIp6GetMulticastAddresses(NULL);
//	test_ext_addr = otLinkGetExtendedAddress(NULL);
//	memcpy(&test_1, test_ext_addr, sizeof(otExtAddress));
////	test_1 = test_ext_addr[0];
////	test_2 = &test_ext_addr;
//
//	error = otThreadGetNextNeighborInfo(NULL, &test_neighbor_iterator, &test_info_neighbor);
	// TODO: does this need an ACK
	APP_THREAD_SendCoapMsg(&msgSendMyIP, sizeof(msgSendMyIP),
			&borderRouter.ipv6, (char*) nodeInfoResource, NO_ACK,
			OT_COAP_CODE_PUT, 1U);
//	APP_THREAD_SendCoapUnicastMsg(NULL, NULL, borderRouter.ipv6  , borderSyncResource, 1U);
}

void APP_THREAD_SendBorderMessage(void *packet, uint8_t len, char *resource) {
//	APP_THREAD_SendCoapMsg(sensorPacket, borderRouter.ipv6, borderPacket, otCoapType type);
	APP_THREAD_SendCoapMsg(packet, len, &borderRouter.ipv6, resource, NO_ACK,
			OT_COAP_CODE_PUT, 1U);
}

void APP_THREAD_SendBorderPacket(struct LogPacket *sensorPacket) {
//	APP_THREAD_SendCoapMsg(sensorPacket, borderRouter.ipv6, borderPacket, otCoapType type);
	APP_THREAD_SendCoapMsg(sensorPacket, sizeof(struct LogPacket),
			&borderRouter.ipv6, (char*) borderPacket, NO_ACK, OT_COAP_CODE_PUT,
			1U);

}

#ifdef NETWORK_TEST

void APP_THREAD_NetworkTestBorderPacket(struct NetworkTestPacket *sensorPacket) {
//	APP_THREAD_SendCoapMsg(sensorPacket, borderRouter.ipv6, borderPacket, otCoapType type);
	APP_THREAD_SendCoapMsg(sensorPacket, sizeof(struct NetworkTestPacket),
			&borderRouter.ipv6, (char*) borderPacket, NO_ACK, OT_COAP_CODE_PUT,
			1U);
}
#endif

static void APP_THREAD_CoapLightsSimpleRequestHandler(void                * pContext,
		otCoapHeader        * pHeader,
		otMessage           * pMessage,
		const otMessageInfo * pMessageInfo) {
	do {

		// if get, send response with current log message
		if (otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_GET) {
//			APP_THREAD_SendDataResponse(pHeader, pMessageInfo, &lightsSimpleMessage, sizeof(lightsSimpleMessage), pHeader, pMessageInfo);
			APP_THREAD_SendDataResponse(&lightsSimpleMessage,
					sizeof(lightsSimpleMessage), pHeader, pMessageInfo);
			break;
		}

		if (otMessageRead(pMessage, otMessageGetOffset(pMessage),
				&lightsSimpleMessage, sizeof(lightsSimpleMessage)) == 4U) {
			if ((otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_PUT)
					|| (otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_POST)) {
				osMessageQueuePut(lightsSimpleQueueHandle, &lightsSimpleMessage,
						0U, 0U);
			}

		}

		tempMessageInfo = pMessageInfo;
		receivedMessage = (otMessageInfo*) pMessage;

		if (otCoapHeaderGetType(pHeader) == OT_COAP_TYPE_CONFIRMABLE) {
//			APP_THREAD_SendDataResponse(pHeader, pMessageInfo, NULL, 0);
			APP_THREAD_SendDataResponse(NULL, 0, pHeader, pMessageInfo);
			break;
		}

		if (otMessageRead(pMessage, otMessageGetOffset(pMessage),
				&OT_ReceivedCommand, 1U) != 1U) {
			//APP_THREAD_Error(ERR_THREAD_MESSAGE_READ, 0);
		}

		if (OT_ReceivedCommand == 1U) {
			//BSP_LED_Toggle(LED1);
		}
	} while (false);
}

#ifndef DONGLE_CODE
// request handler for when receiving a message directed at the data logging resource
static void APP_THREAD_CoapToggleLoggingRequestHandler(void                * pContext,
		otCoapHeader        * pHeader,
		otMessage           * pMessage,
		const otMessageInfo * pMessageInfo) {
	do {
		// if get, send response with current log message
		if (otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_GET) {
			APP_THREAD_SendDataResponse(&logMessage, sizeof(logMessage),
					pHeader, pMessageInfo);
			break;
		}

		// TODO : this will overwrite log message so maybe add a safer method
		if (otMessageRead(pMessage, otMessageGetOffset(pMessage), &logMessage,
				sizeof(logMessage)) == sizeof(logMessage)) {
//			otMessageRead(pMessage, otMessageGetOffset(pMessage), &logMessage, sizeof(logMessage));
			// if post or put, add to queue for masterthread processing
			if ((otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_PUT)
					|| (otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_POST)) {
				osMessageQueuePut(togLoggingQueueHandle, &logMessage, 0U, 0U);
			}
		}

		if (otCoapHeaderGetType(pHeader) == OT_COAP_TYPE_CONFIRMABLE) {
			APP_THREAD_SendDataResponse(NULL, 0, pHeader, pMessageInfo);
			break;
		}

	} while (false);
}

#endif

// request handler for when receiving a message directed at the border router synchronizing resource
static void APP_THREAD_CoapBorderTimeRequestHandler(void                * pContext,
		otCoapHeader        * pHeader,
		otMessage           * pMessage,
		const otMessageInfo * pMessageInfo) {
	do {
#ifdef DONGLE_CODE
		BSP_LED_Toggle(LED_RED);
#endif
		if (otMessageRead(pMessage, otMessageGetOffset(pMessage),
				&receivedSystemCal, sizeof(receivedSystemCal))
				== sizeof(receivedSystemCal)) {
			// if the message was a put request, copy message over to border router info struct
			if ((otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_PUT)
					|| (otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_POST)) {

				memcpy(&borderRouter, &receivedSystemCal,
						sizeof(receivedSystemCal));

				// update the onboard RTC unix time
				updateRTC(borderRouter.epoch);
			}
		}

		receivedMessage = (otMessageInfo*) pMessage;

		if (otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_GET) {

			//TODO: this is where you would put the return if you wanted a node to transmit BR info to other nodes
			//APP_THREAD_SendDataResponse(&borderRouter, sizeof(borderRouter), pHeader, pMessageInfo);

			break;
		}

		if (otCoapHeaderGetType(pHeader) == OT_COAP_TYPE_CONFIRMABLE) {
			APP_THREAD_SendDataResponse(NULL, 0, pHeader, pMessageInfo);
			break;
		}

	} while (false);
}

#ifdef BORDER_ROUTER_NODE

struct LogPacket receivedPacket;
//static uint8_t delimiter = 10;
static uint8_t delimiter[2] = {44,10};

struct sendViaUSB{
	struct LogPacket packet;
	uint8_t delimiter[2];
};

volatile struct sendViaUSB sendViaUSB_packet;

static void APP_THREAD_CoapBorderPacketRequestHandler(otCoapHeader *pHeader, otMessage *pMessage,
		const otMessageInfo *pMessageInfo) {
	do {
#ifdef DONGLE_CODE
		BSP_LED_Toggle(LED_RED);
#endif
		if (otMessageRead(pMessage, otMessageGetOffset(pMessage), &sendViaUSB_packet.packet, sizeof(struct LogPacket))
				== sizeof(struct LogPacket)) {
			// if the message was a put request, copy message over to border router info struct
			if ((otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_PUT)
					|| (otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_POST)) {


//				uint8_t buffer[] = "Hello, World!\r\n";
//				CDC_Transmit_FS(buffer, sizeof(buffer));
				sendViaUSB_packet.delimiter[0] = 10;
				sendViaUSB_packet.delimiter[1] = 10;
				CDC_Transmit_FS((uint8_t *) &sendViaUSB_packet, sizeof(struct sendViaUSB));
//				CDC_Transmit_FS((uint8_t *) delimiter, sizeof(delimiter));

			}
		}

//		receivedMessage = (otMessageInfo*) pMessage;

		if (otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_GET) {

			//TODO: this is where you would put the return if you wanted a node to transmit BR info to other nodes
//			APP_THREAD_SendDataResponse(&borderRouter, sizeof(borderRouter), pHeader, pMessageInfo);
			break;
		}

		if (otCoapHeaderGetType(pHeader) == OT_COAP_TYPE_CONFIRMABLE) {
			APP_THREAD_SendDataResponse(NULL, 0 , pHeader, pMessageInfo);
			break;
		}

	} while (false);
}
#endif

// Only get requests allowed for this resource
struct sendIP_struct tempVar = { "test", "test", 0 };
//char test_string[200] = "test";
static void APP_THREAD_CoapNodeInfoRequestHandler(void                * pContext,
		otCoapHeader        * pHeader,
		otMessage           * pMessage,
		const otMessageInfo * pMessageInfo) {
	do {
#ifdef DONGLE_CODE
		BSP_LED_Toggle(LED_RED);
#endif

//		if (otMessageRead(pMessage, otMessageGetOffset(pMessage), &tempVar, sizeof(tempVar))
//				== sizeof(tempVar)) {
//			// if the message was a put request, copy message over to border router info struct
////			if ((otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_PUT)
////					|| (otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_POST)) {
////
////				memcpy(&borderRouter, &receivedSystemCal, sizeof(receivedSystemCal));
////
////				// update the onboard RTC unix time
////				updateRTC(borderRouter.epoch);
////			}
//		}

//		receivedMessage = (otMessageInfo*) pMessage;

		// send info if requested
		if (otCoapHeaderGetCode(pHeader) == OT_COAP_CODE_GET) {
//			APP_THREAD_SendDataResponse(&borderRouter, sizeof(borderRouter), pHeader, pMessageInfo);
//			APP_THREAD_SendDataResponse(&msgSendMyIP, sizeof(msgSendMyIP), pHeader, pMessageInfo);
			APP_THREAD_SendMyInfo();

			break;
		}

		if (otCoapHeaderGetType(pHeader) == OT_COAP_TYPE_CONFIRMABLE) {
			APP_THREAD_SendDataResponse(NULL, 0, pHeader, pMessageInfo);
		}

	} while (false);
}

void updateRTC(time_t now) {

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	// https://www.st.com/content/ccc/resource/technical/document/application_note/2a/c2/6f/74/fa/0d/46/3a/CD00015424.pdf/files/CD00015424.pdf/jcr:content/translations/en.CD00015424.pdf
//	struct tm *time_tm;

	RTC_FromEpoch(now, &sTime, &sDate);

	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
	}

	/*
	 * update the RTC
	 */
	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
	}

	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x32F2); // lock it in with the backup registers
}

static void APP_THREAD_CoapLightsComplexRequestHandler(void                * pContext,
		otCoapHeader        * pHeader,
		otMessage           * pMessage,
		const otMessageInfo * pMessageInfo) {
	do {
//		APP_THREAD_SendCoapUnicastRequest(NULL, NULL, MULICAST_FTD_MED, borderSyncResource);
		APP_THREAD_SendMyInfo();
		if (otMessageRead(pMessage, otMessageGetOffset(pMessage),
				&lightMessageComplex, sizeof(lightMessageComplex))
				== sizeof(lightMessageComplex)) {
			FrontLightsSet(&lightMessageComplex);
		}

		tempMessageInfo = pMessageInfo;
		receivedMessage = (otMessageInfo*) pMessage;

		if (otCoapHeaderGetType(pHeader) != OT_COAP_TYPE_NON_CONFIRMABLE) {
			break;
		}

		if (otCoapHeaderGetCode(pHeader) != OT_COAP_CODE_PUT) {
			break;
		}

		if (otMessageRead(pMessage, otMessageGetOffset(pMessage),
				&OT_ReceivedCommand, 1U) != 1U) {
			//APP_THREAD_Error(ERR_THREAD_MESSAGE_READ, 0);
		}

		if (OT_ReceivedCommand == 1U) {
			//BSP_LED_Toggle(LED1);
		}

	} while (false);
}

static void APP_THREAD_SendCoapUnicastRequest(char *message,
		uint8_t message_length, char *ipv6_addr, char *resource) {
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
	do {
//			  myRloc16 = otThreadGetRloc16(NULL);
//			  isEnabledIpv6 = otIp6IsEnabled(NULL);
//			  multicastAddresses = otIp6GetMulticastAddresses(NULL);
//			  meshLocalEID =  otThreadGetMeshLocalEid(NULL);
//			  linkLocalIPV6 = otThreadGetLinkLocalIp6Address(NULL);
		memcpy(&meshLocalEID, otThreadGetMeshLocalEid(NULL),
				sizeof(otIp6Address));

		// clear info
		memset(&OT_MessageInfo, 0, sizeof(OT_MessageInfo));

		// set border IP address
		//error = otIp6AddressFromString("ff03::1", &OT_MessageInfo.mPeerAddr);
		error = otIp6AddressFromString(ipv6_addr, &OT_MessageInfo.mPeerAddr);

		memcpy(&OT_MessageInfo.mSockAddr, otThreadGetMeshLocalEid(NULL),
				sizeof(OT_MessageInfo.mSockAddr));

		// error = otIp6AddressFromString("fd11:22::994e:6ed7:263d:6187", &OT_MessageInfo.mPeerAddr);
		//error = otIp6AddressFromString("fdde:ad00:beef:0:0:ff:fe00:3800", &OT_MessageInfo.mPeerAddr);

		OT_MessageInfo.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
		OT_MessageInfo.mPeerPort = OT_DEFAULT_COAP_PORT;

		unicastAddresses = (otNetifAddress*OTCALL) otIp6GetUnicastAddresses(
				NULL);
		OT_MessageInfo.mSockAddr = unicastAddresses->mAddress;
		//OT_MessageInfo.mHopLimit = 20;

		/************** CREATE NEW MESSAGE ********************ifco*/

		// create header
		otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_NON_CONFIRMABLE,
				OT_COAP_CODE_PUT);
		//otCoapHeaderSetMessageId(&OT_Header,OT_BufferIdSend); //may not need since sendRequest should set to 0
		otCoapHeaderGenerateToken(&OT_Header, 2U); //This function sets the Token length and randomizes its value.

		// the name of the resource
		//error = otCoapHeaderAppendUriPathOptions(&OT_Header,C_RESSOURCE_BASIC);
		error = otCoapHeaderAppendUriPathOptions(&OT_Header, resource);

		// This function adds Payload Marker indicating beginning of the payload to the CoAP header
		otCoapHeaderSetPayloadMarker(&OT_Header);

		// creates new message with headers but with empty payload
		pOT_Message = otCoapNewMessage(NULL, &OT_Header);
//		if (pOT_Message == NULL)
//			while (1)
//				;
		// Append bytes to a message (this is where the payload gets added)

		//error = otMessageAppend(pOT_Message, OT_BufferSend, sizeof(OT_BufferSend));
//			  error = otMessageAppend(pOT_Message, &OT_MessageInfo, sizeof(OT_MessageInfo));
		error = otMessageAppend(pOT_Message, message, message_length);

		if (error != OT_ERROR_NONE)
			while (1)
				;

		error = otCoapSendRequest(NULL, pOT_Message, &OT_MessageInfo,
		NULL, (void*) NULL);

		// if error: free allocated message buffer if one was allocated
		if (error != OT_ERROR_NONE && pOT_Message != NULL) {
			otMessageFree(pOT_Message);
		}

		//HAL_Delay(10000);
	} while (false);

}

static char empty_message[10] = "";
void APP_THREAD_SendCoapMsg(void *message, uint16_t msgSize,
		otIp6Address *ipv6_addr, char *resource, uint8_t request_ack,
		otCoapCode coapCode, uint8_t msgID) {
	/************ SET MESSAGE INFO (WHERE THE PACKET GOES) ************/
	// https://openthread.io/reference/struct/ot-message-info.html#structot_message_info
	do {
		// REMOVE BELOW CALLS (ONLY FOR DEBUGGING)
		myRloc16 = otThreadGetRloc16(NULL);
		unicastAddresses = (otNetifAddress*OTCALL) otIp6GetUnicastAddresses(
				NULL);
		isEnabledIpv6 = otIp6IsEnabled(NULL);
		multicastAddresses =
				(otNetifMulticastAddress*) otIp6GetMulticastAddresses(NULL);
		meshLocalEID = (otIp6Address*OTCALL) otThreadGetMeshLocalEid(NULL);
		linkLocalIPV6 = (otIp6Address*) otThreadGetLinkLocalIp6Address(NULL);

		// clear info
		memset(&OT_MessageInfo, 0, sizeof(OT_MessageInfo));

		// add destination IPv6 address to header
		// TODO : swap the below statements once ST has their shit fixed

		if (msgSize > 100) { // TODO : semd to borderRouter if the message is a log message (this is a temporary fix)
#ifndef BORDER_ROUTER_NODE_TRANSMITTER
			memcpy(&OT_MessageInfo.mPeerAddr, &borderRouter.ipv6,
					sizeof(otIp6Address));
			memcpy(&OT_MessageInfo.mSockAddr, otThreadGetMeshLocalEid(NULL),
					sizeof(OT_MessageInfo.mSockAddr));
//			otIp6AddressFromString("fd11:22::c34c:7994:cccc:4b82", &OT_MessageInfo.mSockAddr);
#else
			memcpy(&OT_MessageInfo.mPeerAddr, &multicastAddr, sizeof(otIp6Address));
#endif
		} else {
			memcpy(&OT_MessageInfo.mPeerAddr, &multicastAddr,
					sizeof(otIp6Address));
			memcpy(&OT_MessageInfo.mSockAddr, otThreadGetMeshLocalEid(NULL),
					sizeof(OT_MessageInfo.mSockAddr));
		}

//			  memcpy(&OT_MessageInfo.mPeerAddr, ipv6_addr, sizeof(otIp6Address));

		// add source mesh-local IPv6 address to header
		// TODO: ST is supposed to address the issue in FW v1.5 but currently, the IP lookup function returns a null pointer
		// memcpy(&OT_MessageInfo.mSockAddr, otThreadGetMeshLocalEid(NULL), sizeof(OT_MessageInfo.mSockAddr));

		// REMOVE BELOW CALLS (ONLY FOR DEBUGGING)
		//			  otIp6AddressFromString("fd11:22::c55e:6732:c8cd:9443", &OT_MessageInfo.mPeerAddr);
//			  otIp6AddressFromString("fd11:1111:1122:0:7be1:2f12:9534:72d5", &OT_MessageInfo.mPeerAddr);
//			  			  otIp6AddressFromString("fd11:1111:1122:0:4faa:fd9:da06:9100", &OT_MessageInfo.mPeerAddr);
//			  			  otIp6AddressFromString("fd11:22:0:0:c34c:7994:19f2:4b82", &OT_MessageInfo.mPeerAddr);
//			   memcpy(&OT_MessageInfo.mSockAddr, otThreadGetMeshLocalEid(NULL), sizeof(OT_MessageInfo.mSockAddr));
//			  otIp6AddressFromString("fd11:22::c34c:7994:19f2:4b82", &OT_MessageInfo.mSockAddr);

//			  otIp6AddressFromString("fd11:22::c34c:7994:cccc:4b82", &OT_MessageInfo.mSockAddr);

		// populate message information
		OT_MessageInfo.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
		OT_MessageInfo.mPeerPort = OT_DEFAULT_COAP_PORT;
		OT_MessageInfo.mHopLimit = 64;

		/************** CREATE NEW MESSAGE ********************ifco*/

		// create header
		if (request_ack && (coapCode == OT_COAP_CODE_PUT))
			otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_CONFIRMABLE,
					OT_COAP_CODE_PUT);
		else if (request_ack && (coapCode == OT_COAP_CODE_GET))
			otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_CONFIRMABLE,
					OT_COAP_CODE_GET);
		else if (request_ack && (coapCode == OT_COAP_CODE_POST))
			otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_CONFIRMABLE,
					OT_COAP_CODE_POST);
		else if (!request_ack && (coapCode == OT_COAP_CODE_PUT))
			otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_NON_CONFIRMABLE,
					OT_COAP_CODE_PUT);
		else if (!request_ack && (coapCode == OT_COAP_CODE_GET))
			otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_NON_CONFIRMABLE,
					OT_COAP_CODE_GET);
		else if (!request_ack && (coapCode == OT_COAP_CODE_POST))
			otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_NON_CONFIRMABLE,
					OT_COAP_CODE_POST);
		else
			return // this return should never happen

//			  otCoapHeaderSetMessageId(&OT_Header, msgID); `//may not need since sendRequest should set to 0
			otCoapHeaderGenerateToken(&OT_Header, 2U); //This function sets the Token length and randomizes its value.

		// add the name of the resource
		error = otCoapHeaderAppendUriPathOptions(&OT_Header, resource);
//			  if (error != OT_ERROR_NONE) while(1);

		// need this so the coap server doesnt try to parse as 'utf-8' and error out
		otCoapHeaderAppendContentFormatOption(&OT_Header,
				OT_COAP_OPTION_CONTENT_FORMAT_OCTET_STREAM);
//			  if (error != OT_ERROR_NONE) while(1);

		// This function adds Payload Marker indicating beginning of the payload to the CoAP header
		if (msgSize > 0) {
			otCoapHeaderSetPayloadMarker(&OT_Header); //TODO: if no msg, dont set marker and remove empty message below
		}

		// creates new message with headers but with empty payload
		pOT_Message = otCoapNewMessage(NULL, &OT_Header);
//		if (pOT_Message == NULL)
//			while (1);

		// Append bytes to a message (this is where the payload gets added)

		// append message if there was one given
		if (msgSize > 0) {
			error = otMessageAppend(pOT_Message, message, msgSize);
		}
//		else{
//			error = otMessageAppend(pOT_Message, empty_message, 10);
//		}

//			  if (error != OT_ERROR_NONE) while(1);

		// TODO: the response function should only be used for the border update event (I think only if message is embedded in ACK)?

		error = otCoapSendRequest(NULL, pOT_Message, &OT_MessageInfo,
		NULL, (void*) NULL);

		// if error: free allocated message buffer if one was allocated
		if (error != OT_ERROR_NONE && pOT_Message != NULL) {
			otMessageFree(pOT_Message);
		}

	} while (false);
}

static void APP_THREAD_SendCoapMsgForBorderSync(void *message, uint16_t msgSize,
		otIp6Address *ipv6_addr, char *resource, uint8_t request_ack,
		otCoapCode coapCode, uint8_t msgID) {
	/************ SET MESSAGE INFO (WHERE THE PACKET GOES) ************/
	// https://openthread.io/reference/struct/ot-message-info.html#structot_message_info
	do {
		// REMOVE BELOW CALLS (ONLY FOR DEBUGGING)
//			  myRloc16 = otThreadGetRloc16(NULL);
//			  unicastAddresses = otIp6GetUnicastAddresses(NULL);
//			  isEnabledIpv6 = otIp6IsEnabled(NULL);
//			  multicastAddresses = otIp6GetMulticastAddresses(NULL);
//			  meshLocalEID =  otThreadGetMeshLocalEid(NULL);
//			  linkLocalIPV6 = otThreadGetLinkLocalIp6Address(NULL);

		// clear info
		memset(&OT_MessageInfo, 0, sizeof(OT_MessageInfo));

		// add destination IPv6 address to header
		// TODO : swap the below statements once ST has their shit fixed

		memcpy(&OT_MessageInfo.mPeerAddr, &multicastAddr, sizeof(otIp6Address));

//			  memcpy(&OT_MessageInfo.mPeerAddr, ipv6_addr, sizeof(otIp6Address));

		// add source mesh-local IPv6 address to header
		// TODO: ST is supposed to address the issue in FW v1.5 but currently, the IP lookup function returns a null pointer
		// memcpy(&OT_MessageInfo.mSockAddr, otThreadGetMeshLocalEid(NULL), sizeof(OT_MessageInfo.mSockAddr));

		// REMOVE BELOW CALLS (ONLY FOR DEBUGGING)
		//			  otIp6AddressFromString("fd11:22::c55e:6732:c8cd:9443", &OT_MessageInfo.mPeerAddr);
//			  otIp6AddressFromString("fd11:1111:1122:0:7be1:2f12:9534:72d5", &OT_MessageInfo.mPeerAddr);
//			  			  otIp6AddressFromString("fd11:1111:1122:0:4faa:fd9:da06:9100", &OT_MessageInfo.mPeerAddr);
//			  			  otIp6AddressFromString("fd11:22:0:0:c34c:7994:19f2:4b82", &OT_MessageInfo.mPeerAddr);
//			   memcpy(&OT_MessageInfo.mSockAddr, otThreadGetMeshLocalEid(NULL), sizeof(OT_MessageInfo.mSockAddr));
//			  otIp6AddressFromString("fd11:22::c34c:7994:19f2:4b82", &OT_MessageInfo.mSockAddr);

//			  otIp6AddressFromString("fd11:22::c34c:7994:cccc:4b82", &OT_MessageInfo.mSockAddr);

		// populate message information
		OT_MessageInfo.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
		OT_MessageInfo.mPeerPort = OT_DEFAULT_COAP_PORT;
		OT_MessageInfo.mHopLimit = 64;

		/************** CREATE NEW MESSAGE ********************ifco*/

		// create header
		if (request_ack && (coapCode == OT_COAP_CODE_PUT))
			otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_CONFIRMABLE,
					OT_COAP_CODE_PUT);
		else if (request_ack && (coapCode == OT_COAP_CODE_GET))
			otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_CONFIRMABLE,
					OT_COAP_CODE_GET);
		else if (request_ack && (coapCode == OT_COAP_CODE_POST))
			otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_CONFIRMABLE,
					OT_COAP_CODE_POST);
		else if (!request_ack && (coapCode == OT_COAP_CODE_PUT))
			otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_NON_CONFIRMABLE,
					OT_COAP_CODE_PUT);
		else if (!request_ack && (coapCode == OT_COAP_CODE_GET))
			otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_NON_CONFIRMABLE,
					OT_COAP_CODE_GET);
		else if (!request_ack && (coapCode == OT_COAP_CODE_POST))
			otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_NON_CONFIRMABLE,
					OT_COAP_CODE_POST);
		else
			return // this return should never happen

//			  otCoapHeaderSetMessageId(&OT_Header, msgID); `//may not need since sendRequest should set to 0
			otCoapHeaderGenerateToken(&OT_Header, 2U); //This function sets the Token length and randomizes its value.

		// add the name of the resource
		error = otCoapHeaderAppendUriPathOptions(&OT_Header, resource);
//			  if (error != OT_ERROR_NONE) while(1);

		// need this so the coap server doesnt try to parse as 'utf-8' and error out
//		otCoapHeaderAppendContentFormatOption(&OT_Header, OT_COAP_OPTION_CONTENT_FORMAT_OCTET_STREAM);
//			  if (error != OT_ERROR_NONE) while(1);

		// This function adds Payload Marker indicating beginning of the payload to the CoAP header
//		otCoapHeaderSetPayloadMarker(&OT_Header);

		// creates new message with headers but with empty payload
		pOT_Message = otCoapNewMessage(NULL, &OT_Header);
//		if (pOT_Message == NULL)
//			while (1);

		// Append bytes to a message (this is where the payload gets added)

		// append message if there was one given
//		if (msgSize > 0) {
//			error = otMessageAppend(pOT_Message, message, msgSize);
//		}else{
//			error = otMessageAppend(pOT_Message, empty_message, 10);
//		}

		// if (error != OT_ERROR_NONE) while(1);

		// TODO: the response function should only be used for the border update event (I think only if message is embedded in ACK)?

		error = otCoapSendRequest(NULL, pOT_Message, &OT_MessageInfo,
				&APP_THREAD_DummyRespHandler,
				(void*) &APP_THREAD_CoapRespHandler_UpdateBorderRouter);

		// if error: free allocated message buffer if one was allocated
		if (error != OT_ERROR_NONE && pOT_Message != NULL) {
			otMessageFree(pOT_Message);
		}

	} while (false);

}

/**
 * @brief This function acknowledge the data reception by sending an ACK
 *    back to the sender.
 * @param  pRequestHeader coap header
 * @param  pMessageInfo message info pointer
 * @retval None
 */

/*From RFC:  In a piggybacked response, the Message ID of the Confirmable
 request and the Acknowledgement MUST match, and the tokens of the
 response and original request MUST match.  In a separate
 response, just the tokens of the response and original request
 MUST match.*/

static void APP_THREAD_SendDataResponse(void *message, uint16_t msgSize,
		otCoapHeader *pRequestHeader, const otMessageInfo *pMessageInfo) {
	otError error = OT_ERROR_NONE;

	//APP_DBG(" ********* APP_THREAD_SendDataResponse \r\n");
	otCoapHeaderInit(&OT_Header, OT_COAP_TYPE_ACKNOWLEDGMENT,
			OT_COAP_CODE_CHANGED);
	otCoapHeaderSetMessageId(&OT_Header,
			otCoapHeaderGetMessageId(pRequestHeader));
	otCoapHeaderSetToken(&OT_Header, otCoapHeaderGetToken(pRequestHeader),
			otCoapHeaderGetTokenLength(pRequestHeader));

	if (msgSize > 0) {
		// need this so the coap server doesnt try to parse as 'utf-8' and error out
		otCoapHeaderAppendContentFormatOption(&OT_Header,
				OT_COAP_OPTION_CONTENT_FORMAT_OCTET_STREAM);
//			  if (error != OT_ERROR_NONE) while(1);

		// This function adds Payload Marker indicating beginning of the payload to the CoAP header
		otCoapHeaderSetPayloadMarker(&OT_Header); //TODO: if no msg, dont set marker and remove empty message below
	}

	pOT_Message = otCoapNewMessage(NULL, &OT_Header);
	if (pOT_Message == NULL) {
		//APP_THREAD_Error(ERR_NEW_MSG_ALLOC,error);
	}

	// append message if there was one given
	if (msgSize > 0) {
		error = otMessageAppend(pOT_Message, message, msgSize);
	}

	error = otCoapSendResponse(NULL, pOT_Message, pMessageInfo);
	if (error != OT_ERROR_NONE && pOT_Message != NULL) {
		otMessageFree(pOT_Message);
		//APP_THREAD_Error(ERR_THREAD_DATA_RESPONSE,error);
	}
}
/* USER CODE END FD_LOCAL_FUNCTIONS */

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/

void APP_THREAD_RegisterCmdBuffer(TL_CmdPacket_t *p_buffer) {
	p_thread_otcmdbuffer = p_buffer;
}

Thread_OT_Cmd_Request_t* THREAD_Get_OTCmdPayloadBuffer(void) {
	return (Thread_OT_Cmd_Request_t*) p_thread_otcmdbuffer->cmdserial.cmd.payload;
}

Thread_OT_Cmd_Request_t* THREAD_Get_OTCmdRspPayloadBuffer(void) {
	return (Thread_OT_Cmd_Request_t*) ((TL_EvtPacket_t*) p_thread_otcmdbuffer)->evtserial.evt.payload;
}

Thread_OT_Cmd_Request_t* THREAD_Get_NotificationPayloadBuffer(void) {
	return (Thread_OT_Cmd_Request_t*) (p_thread_notif_M0_to_M4)->evtserial.evt.payload;
}

/**
 * @brief  This function is used to transfer the Ot commands from the
 *         M4 to the M0.
 *
 * @param   None
 * @return  None
 */
void Ot_Cmd_Transfer(void) {
	/* OpenThread OT command cmdcode range 0x280 .. 0x3DF = 352 */
	p_thread_otcmdbuffer->cmdserial.cmd.cmdcode = 0x280U;
	/* Size = otCmdBuffer->Size (Number of OT cmd arguments : 1 arg = 32bits so multiply by 4 to get size in bytes)
	 * + ID (4 bytes) + Size (4 bytes) */
	uint32_t l_size =
			((Thread_OT_Cmd_Request_t*) (p_thread_otcmdbuffer->cmdserial.cmd.payload))->Size
					* 4U + 8U;
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
void TL_OT_CmdEvtReceived(TL_EvtPacket_t *Otbuffer) {
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
void TL_THREAD_NotReceived(TL_EvtPacket_t *Notbuffer) {
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
void Pre_OtCmdProcessing(void) {

}

/**
 * @brief  This function waits for getting an acknowledgment from the M0.
 *
 * @param  None
 * @retval None
 */
static void Wait_Getting_Ack_From_M0(void) {
	while (FlagReceiveAckFromM0 == 0) {
	}
	FlagReceiveAckFromM0 = 0;
}

/**
 * @brief  Receive an acknowledgment from the M0+ core.
 *         Each command send by the M4 to the M0 are acknowledged.
 *         This function is called under interrupt.
 * @param  None
 * @retval None
 */
static void Receive_Ack_From_M0(void) {
	FlagReceiveAckFromM0 = 1;
}

/**
 * @brief  Receive a notification from the M0+ through the IPCC.
 *         This function is called under interrupt.
 * @param  None
 * @retval None
 */
static void Receive_Notification_From_M0(void) {
	CptReceiveMsgFromM0++;
	osThreadFlagsSet(OsTaskMsgM0ToM4Id, 1);
}

#if (CFG_USB_INTERFACE_ENABLE != 0)
#else
#if (CFG_FULL_LOW_POWER == 0)
static void RxCpltCallback(void) {
	/* Filling buffer and wait for '\r' char */
	if (indexReceiveChar < C_SIZE_CMD_STRING) {
		CommandString[indexReceiveChar++] = aRxBuffer[0];
		if (aRxBuffer[0] == '\r') {
			CptReceiveCmdFromUser = 1U;

			/* UART task scheduling*/
			osThreadFlagsSet(OsTaskCliId, 1);
		}
	}

	/* Once a character has been sent, put back the device in reception mode */
//  HW_UART_Receive_IT(CFG_CLI_UART, aRxBuffer, 1U, RxCpltCallback);
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
    osThreadFlagsSet(OsTaskCliId,1)
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
static void Send_CLI_To_M0(void) {
	memset(ThreadCliCmdBuffer.cmdserial.cmd.payload, 0x0U, 255U);
	memcpy(ThreadCliCmdBuffer.cmdserial.cmd.payload, CommandString,
			indexReceiveChar);
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
static void Send_CLI_Ack_For_OT(void) {

	/* Notify M0 that characters have been sent to UART */
	TL_THREAD_CliSendAck();
}

/**
 * @brief Perform initialization of CLI UART interface.
 * @param  None
 * @retval None
 */
void APP_THREAD_Init_UART_CLI(void) {
#if (CFG_FULL_LOW_POWER == 0)
	OsTaskCliId = osThreadNew(APP_THREAD_FreeRTOSSendCLIToM0Task, NULL,
			&ThreadCliProcess_attr);
#endif /* (CFG_FULL_LOW_POWER == 0) */

#if (CFG_USB_INTERFACE_ENABLE != 0)
#else
#if (CFG_FULL_LOW_POWER == 0)
//  HW_UART_Receive_IT(CFG_CLI_UART, aRxBuffer, 1, RxCpltCallback);
#endif /* (CFG_FULL_LOW_POWER == 0) */
#endif /* (CFG_USB_INTERFACE_ENABLE != 0) */
}

/**
 * @brief Perform initialization of TL for THREAD.
 * @param  None
 * @retval None
 */
void APP_THREAD_TL_THREAD_INIT(void) {
	ThreadConfigBuffer.p_ThreadOtCmdRspBuffer = (uint8_t*) &ThreadOtCmdBuffer;
	ThreadConfigBuffer.p_ThreadNotAckBuffer =
			(uint8_t*) ThreadNotifRspEvtBuffer;
	ThreadConfigBuffer.p_ThreadCliRspBuffer = (uint8_t*) &ThreadCliCmdBuffer;

	TL_THREAD_Init(&ThreadConfigBuffer);
}

/**
 * @brief  This function is called when notification on CLI TL Channel from M0+ is received.
 *
 * @param   Notbuffer : a pointer to TL_EvtPacket_t
 * @return  None
 */
void TL_THREAD_CliNotReceived(TL_EvtPacket_t *Notbuffer) {
	TL_CmdPacket_t *l_CliBuffer = (TL_CmdPacket_t*) Notbuffer;
	uint8_t l_size = l_CliBuffer->cmdserial.cmd.plen;

	/* WORKAROUND: if string to output is "> " then respond directly to M0 and do not output it */
	if (strcmp((const char*) l_CliBuffer->cmdserial.cmd.payload, "> ") != 0) {
		/* Write to CLI UART */
#if (CFG_USB_INTERFACE_ENABLE != 0)
    VCP_SendData( l_CliBuffer->cmdserial.cmd.payload, l_size, HostTxCb);
#else
//    HW_UART_Transmit_IT(CFG_CLI_UART, l_CliBuffer->cmdserial.cmd.payload, l_size, HostTxCb);
#endif /*USAGE_OF_VCP */
	} else {
		Send_CLI_Ack_For_OT();
	}
}

/**
 * @brief  End of transfer callback for CLI UART sending.
 *
 * @param   Notbuffer : a pointer to TL_EvtPacket_t
 * @return  None
 */
void HostTxCb(void) {
	Send_CLI_Ack_For_OT();
}

/**
 * @brief Process the messages coming from the M0.
 * @param  None
 * @retval None
 */
void APP_THREAD_ProcessMsgM0ToM4(void) {
	if (CptReceiveMsgFromM0 != 0) {
		/* If CptReceiveMsgFromM0 is > 1. it means that we did not serve all the events from the radio */
		if (CptReceiveMsgFromM0 > 1U) {
			APP_THREAD_Error(ERR_REC_MULTI_MSG_FROM_M0, 0);
		} else {
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
