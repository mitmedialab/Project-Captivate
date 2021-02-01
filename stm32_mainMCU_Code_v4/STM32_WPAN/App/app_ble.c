/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_ble.c
 * @author  MCD Application Team
 * @brief   BLE Application
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

#include "dbg_trace.h"
#include "ble.h"
#include "tl.h"
#include "app_ble.h"

//#include "stm32_seq.h"
#include "shci.h"
#include "stm32_lpm.h"
#include "otp.h"
#include "p2p_server_app.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dis_app.h"
#include "main.h"
#include "ble_legacy.h"
#include "ble_defs.h"

#include "dt_client_app.h"
#include "dt_server_app.h"
#include "dts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/





enum
{
	NO_DEVICE_FOUND,
	AT_LEAST_ONE_DEVICE_FOUND
};

typedef enum
{
	GAP_PROC_PAIRING,
	GAP_PROC_SET_PHY,
} GapProcId_t;


/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
#define APPBLE_GAP_DEVICE_NAME_LENGTH 10
#define FAST_ADV_TIMEOUT               (30*1000*1000/CFG_TS_TICK_VAL) /**< 30s */
#define INITIAL_ADV_TIMEOUT            (120*1000*1000/CFG_TS_TICK_VAL) /**< 120s */

#define BD_ADDR_SIZE_LOCAL    6

/* USER CODE BEGIN PD */
#define LED_ON_TIMEOUT                 (0.005*1000*1000/CFG_TS_TICK_VAL) /**< 5ms */
#define FAST_CONN_ADV_INTERVAL_MIN  (0x20)  /**< 20ms */
#define FAST_CONN_ADV_INTERVAL_MAX  (0x30)  /**< 30ms */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_CmdPacket_t BleCmdBuffer;

static const uint8_t M_bd_addr[BD_ADDR_SIZE_LOCAL] =
{
		(uint8_t)((CFG_ADV_BD_ADDRESS & 0x0000000000FF)),
		(uint8_t)((CFG_ADV_BD_ADDRESS & 0x00000000FF00) >> 8),
		(uint8_t)((CFG_ADV_BD_ADDRESS & 0x000000FF0000) >> 16),
		(uint8_t)((CFG_ADV_BD_ADDRESS & 0x0000FF000000) >> 24),
		(uint8_t)((CFG_ADV_BD_ADDRESS & 0x00FF00000000) >> 32),
		(uint8_t)((CFG_ADV_BD_ADDRESS & 0xFF0000000000) >> 40)
};
static uint8_t bd_addr_udn[BD_ADDR_SIZE_LOCAL];

/**
 *   Identity root key used to derive LTK and CSRK
 */
static const uint8_t BLE_CFG_IR_VALUE[16] = CFG_BLE_IRK;

/**
 * Encryption root key used to derive LTK and CSRK
 */
static const uint8_t BLE_CFG_ER_VALUE[16] = CFG_BLE_ERK;

PLACE_IN_SECTION("BLE_APP_CONTEXT") BleApplicationContext_t BleApplicationContext;
PLACE_IN_SECTION("BLE_APP_CONTEXT") static uint16_t AdvIntervalMin, AdvIntervalMax;

P2PS_APP_ConnHandle_Not_evt_t handleNotification;

#if L2CAP_REQUEST_NEW_CONN_PARAM != 0
#define SIZE_TAB_CONN_INT            2
float tab_conn_interval[SIZE_TAB_CONN_INT] = {50, 1000} ; /* ms */
uint8_t index_con_int, mutex; 
#endif 

/**
 * Advertising Data
 */
#if (P2P_SERVER1 != 0)
//static const char local_name[] = { AD_TYPE_COMPLETE_LOCAL_NAME ,'P','2','P','S','R','V','1'};
//uint8_t manuf_data[14] = {
//    sizeof(manuf_data)-1, AD_TYPE_MANUFACTURER_SPECIFIC_DATA,
//    0x01/*SKD version */,
//    CFG_DEV_ID_P2P_SERVER1 /* STM32WB - P2P Server 1*/,
//    0x00 /* GROUP A Feature  */,
//    0x00 /* GROUP A Feature */,
//    CFG_FEATURE_THREAD_SWITCH /* GROUP B Feature */,
//    0x00 /* GROUP B Feature */,
//    0x00, /* BLE MAC start -MSB */
//    0x00,
//    0x00,
//    0x00,
//    0x00,
//    0x00, /* BLE MAC stop */
//};
#endif

#if (CFG_BLE_PERIPHERAL != 0)
static const char local_name[] = { AD_TYPE_COMPLETE_LOCAL_NAME, 'C', 'A', 'P', 'T', 'I', 'V', 'A', 'T', 'E'};
#endif

uint8_t index_con_int, mutex;

uint8_t const manuf_data[22] = { 2, AD_TYPE_TX_POWER_LEVEL, 0x00 /* 0 dBm */, /* Trasmission Power */
		10, AD_TYPE_COMPLETE_LOCAL_NAME, 'C', 'A', 'P', 'T', 'I', 'V', 'A', 'T', 'E', /* Complete Name */
		7, AD_TYPE_MANUFACTURER_SPECIFIC_DATA, 0x01/*SKD version */,
		CFG_DEV_ID_PERIPH_SERVER, /* NUCLEO-Board WB - DT Periph Server*/
		0x00 /*  */, 0x00 /*  */, 0x00 /*  */,
		CFG_FEATURE_DT /* Data Throughput Service features */
};
/**
 * Advertising Data
 */
#if (P2P_SERVER2 != 0)
static const char local_name[] = { AD_TYPE_COMPLETE_LOCAL_NAME, 'P', '2', 'P', 'S', 'R', 'V', '2'};
uint8_t manuf_data[14] = {
		sizeof(manuf_data)-1, AD_TYPE_MANUFACTURER_SPECIFIC_DATA,
		0x01/*SKD version */,
		CFG_DEV_ID_P2P_SERVER2 /* STM32WB - P2P Server 2*/,
		0x00 /* GROUP A Feature  */,
		0x00 /* GROUP A Feature */,
		0x00 /* GROUP B Feature */,
		0x00 /* GROUP B Feature */,
		0x00, /* BLE MAC start -MSB */
		0x00,
		0x00,
		0x00,
		0x00,
		0x00, /* BLE MAC stop */
};

#endif

#if (P2P_SERVER3 != 0)
static const char local_name[] = { AD_TYPE_COMPLETE_LOCAL_NAME, 'P', '2', 'P', 'S', 'R', 'V', '3'};
uint8_t manuf_data[14] = {
		sizeof(manuf_data)-1, AD_TYPE_MANUFACTURER_SPECIFIC_DATA,
		0x01/*SKD version */,
		CFG_DEV_ID_P2P_SERVER3 /* STM32WB - P2P Server 3*/,
		0x00 /* GROUP A Feature  */,
		0x00 /* GROUP A Feature */,
		0x00 /* GROUP B Feature */,
		0x00 /* GROUP B Feature */,
		0x00, /* BLE MAC start -MSB */
		0x00,
		0x00,
		0x00,
		0x00,
		0x00, /* BLE MAC stop */
};
#endif

#if (P2P_SERVER4 != 0)
static const char local_name[] = { AD_TYPE_COMPLETE_LOCAL_NAME, 'P', '2', 'P', 'S', 'R', 'V', '4'};
uint8_t manuf_data[14] = {
		sizeof(manuf_data)-1, AD_TYPE_MANUFACTURER_SPECIFIC_DATA,
		0x01/*SKD version */,
		CFG_DEV_ID_P2P_SERVER4 /* STM32WB - P2P Server 4*/,
		0x00 /* GROUP A Feature  */,
		0x00 /* GROUP A Feature */,
		0x00 /* GROUP B Feature */,
		0x00 /* GROUP B Feature */,
		0x00, /* BLE MAC start -MSB */
		0x00,
		0x00,
		0x00,
		0x00,
		0x00, /* BLE MAC stop */
};
#endif

#if (P2P_SERVER5 != 0)
static const char local_name[] = { AD_TYPE_COMPLETE_LOCAL_NAME, 'P', '2', 'P', 'S', 'R', 'V', '5'};
uint8_t manuf_data[14] = {
		sizeof(manuf_data)-1, AD_TYPE_MANUFACTURER_SPECIFIC_DATA,
		0x01/*SKD version */,
		CFG_DEV_ID_P2P_SERVER5 /* STM32WB - P2P Server 5*/,
		0x00 /* GROUP A Feature  */,
		0x00 /* GROUP A Feature */,
		0x00 /* GROUP B Feature */,
		0x00 /* GROUP B Feature */,
		0x00, /* BLE MAC start -MSB */
		0x00,
		0x00,
		0x00,
		0x00,
		0x00, /* BLE MAC stop */
};
#endif

#if (P2P_SERVER6 != 0)
static const char local_name[] = { AD_TYPE_COMPLETE_LOCAL_NAME, 'P', '2', 'P', 'S', 'R', 'V', '6'};
uint8_t manuf_data[14] = {
		sizeof(manuf_data)-1, AD_TYPE_MANUFACTURER_SPECIFIC_DATA,
		0x01/*SKD version */,
		CFG_DEV_ID_P2P_SERVER6 /* STM32WB - P2P Server 1*/,
		0x00 /* GROUP A Feature  */,
		0x00 /* GROUP A Feature */,
		0x00 /* GROUP B Feature */,
		0x00 /* GROUP B Feature */,
		0x00, /* BLE MAC start -MSB */
		0x00,
		0x00,
		0x00,
		0x00,
		0x00, /* BLE MAC stop */
};
#endif

/* USER CODE BEGIN PV */
/* Global variables ----------------------------------------------------------*/
osMutexId_t MtxHciId;
osSemaphoreId_t SemHciId;
osThreadId_t AdvUpdateProcessId;
osThreadId_t HciUserEvtProcessId;
osThreadId_t LinkConfigProcessId;
osThreadId_t AdvCancelProcessId;
osThreadId_t AdvReqProcessId;


tBDAddr SERVER_REMOTE_BDADDR;

const osThreadAttr_t AdvUpdateProcess_attr = {
		.name = CFG_ADV_UPDATE_PROCESS_NAME,
		.attr_bits = CFG_ADV_UPDATE_PROCESS_ATTR_BITS,
		.cb_mem = CFG_ADV_UPDATE_PROCESS_CB_MEM,
		.cb_size = CFG_ADV_UPDATE_PROCESS_CB_SIZE,
		.stack_mem = CFG_ADV_UPDATE_PROCESS_STACK_MEM,
		.priority = CFG_ADV_UPDATE_PROCESS_PRIORITY,
		.stack_size = CFG_ADV_UPDATE_PROCESS_STACK_SIZE * 2
};

const osThreadAttr_t AdvCancelProcess_attr = {
		.name = "ADV_CANCEL",
		.attr_bits = CFG_ADV_UPDATE_PROCESS_ATTR_BITS,
		.cb_mem = CFG_ADV_UPDATE_PROCESS_CB_MEM,
		.cb_size = CFG_ADV_UPDATE_PROCESS_CB_SIZE,
		.stack_mem = CFG_ADV_UPDATE_PROCESS_STACK_MEM,
		.priority = CFG_ADV_UPDATE_PROCESS_PRIORITY,
		.stack_size = CFG_ADV_UPDATE_PROCESS_STACK_SIZE
};

const osThreadAttr_t HciUserEvtProcess_attr = {
		.name = CFG_HCI_USER_EVT_PROCESS_NAME,
		.attr_bits = CFG_HCI_USER_EVT_PROCESS_ATTR_BITS,
		.cb_mem = CFG_HCI_USER_EVT_PROCESS_CB_MEM,
		.cb_size = CFG_HCI_USER_EVT_PROCESS_CB_SIZE,
		.stack_mem = CFG_HCI_USER_EVT_PROCESS_STACK_MEM,
		.priority = CFG_HCI_USER_EVT_PROCESS_PRIORITY,
		.stack_size = CFG_HCI_USER_EVT_PROCESS_STACK_SIZE
};

const osThreadAttr_t LinkConfigProcess_attr = {
		.name = CFG_TP_LINK_CONFIG_PROCESS_NAME,
		.attr_bits = CFG_TP_GENERIC_PROCESS_ATTR_BITS,
		.cb_mem = CFG_TP_GENERIC_PROCESS_CB_MEM,
		.cb_size = CFG_TP_GENERIC_PROCESS_CB_SIZE,
		.stack_mem = CFG_TP_GENERIC_PROCESS_STACK_MEM,
		.priority = CFG_TP_GENERIC_PROCESS_PRIORITY,
		.stack_size = CFG_TP_GENERIC_PROCESS_STACK_SIZE * 2
};

uint8_t TimerDataThroughputWrite_Id;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void BLE_UserEvtRx( void * pPayload );
static void BLE_StatusNot( HCI_TL_CmdStatus_t status );
static void Ble_Tl_Init( void );
static void Ble_Hci_Gap_Gatt_Init(void);
static const uint8_t* BleGetBdAddress( void );
static void Adv_Request( APP_BLE_ConnStatus_t New_Status );
static void Adv_Cancel( void* argument );
#ifndef DYNAMIC_MODE
static void Adv_Cancel_Req( void );
#endif
static void Switch_OFF_GPIO( void );


/* USER CODE BEGIN PFP */
static void AdvUpdateProcess(void *argument);
static void Adv_Mgr( void );
static void Adv_Update( void );
static void Add_Advertisment_Service_UUID( uint16_t servUUID );
static void HciUserEvtProcess(void *argument);
void Adv_Request_TP( void );

//static void Adv_Request(void);
void DataThroughput_proc(void);

void LinkConfiguration(void * argument);
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void APP_BLE_Init_Dyn_1( void )
{
	/* USER CODE BEGIN APP_BLE_Init_1 */

	/* USER CODE END APP_BLE_Init_1 */

	SHCI_C2_Ble_Init_Cmd_Packet_t ble_init_cmd_packet =
	{
			{{0,0,0}},                          /**< Header unused */
			{0,                                 /** pBleBufferAddress not used */
					0,                                  /** BleBufferSize not used */
					CFG_BLE_NUM_GATT_ATTRIBUTES,
					CFG_BLE_NUM_GATT_SERVICES,
					CFG_BLE_ATT_VALUE_ARRAY_SIZE,
					CFG_BLE_NUM_LINK,
					CFG_BLE_DATA_LENGTH_EXTENSION,
					CFG_BLE_PREPARE_WRITE_LIST_SIZE,
					CFG_BLE_MBLOCK_COUNT,
					CFG_BLE_MAX_ATT_MTU,
					CFG_BLE_SLAVE_SCA,
					CFG_BLE_MASTER_SCA,
					CFG_BLE_LSE_SOURCE,
					CFG_BLE_MAX_CONN_EVENT_LENGTH,
					CFG_BLE_HSE_STARTUP_TIME,
					CFG_BLE_VITERBI_MODE,
					CFG_BLE_LL_ONLY,
					0},
	};

	/**
	 * Initialize Ble Transport Layer
	 */
	Ble_Tl_Init( );

	/**
	 * Do not allow standby in the application
	 */
	UTIL_LPM_SetOffMode(1 << CFG_LPM_APP_BLE, UTIL_LPM_DISABLE);

	//TODO: REPLACED
	//  /**
	//   * Register the hci transport layer to handle BLE User Asynchronous Events
	//   */
	//  UTIL_SEQ_RegTask( 1<<CFG_TASK_HCI_ASYNCH_EVT_ID, UTIL_SEQ_RFU, hci_user_evt_proc);

	/**
	 * Register the hci transport layer to handle BLE User Asynchronous Events
	 */
	HciUserEvtProcessId = osThreadNew(HciUserEvtProcess, NULL, &HciUserEvtProcess_attr);

	//TODO: REPLACED
	//  /**
	//   * Starts the BLE Stack on CPU2
	//   */
	//  SHCI_C2_BLE_Init( &ble_init_cmd_packet );

	/**
	 * Starts the BLE Stack on CPU2
	 */
	if (SHCI_C2_BLE_Init( &ble_init_cmd_packet ) != SHCI_Success)
	{
		Error_Handler();
	}

	/**
	 * Initialization of HCI & GATT & GAP layer
	 */
	Ble_Hci_Gap_Gatt_Init();

	/**
	 * Initialization of the BLE Services
	 */
	SVCCTL_Init();

#if (L2CAP_REQUEST_NEW_CONN_PARAM != 0 )
	index_con_int = 0;
	mutex = 1;
#endif


	/* TODO: ERROR SOMEWHERE BELOW? */


	//  /**
	//   * Initialization of the BLE App Context
	//   */
	//  BleApplicationContext.Device_Connection_Status = APP_BLE_IDLE;
	//  BleApplicationContext.BleApplicationContext_legacy.connectionHandle = 0xFFFF;

	//TODO: REPLACED
	//  /**
	//   * From here, all initialization are BLE application specific
	//   */
	//  UTIL_SEQ_RegTask( 1<<CFG_TASK_ADV_CANCEL_ID, UTIL_SEQ_RFU, Adv_Cancel);
	//  AdvCancelProcessId = osThreadNew(Adv_Cancel, NULL, &AdvCancelProcess_attr);


	/**
	 * From here, all initialization are BLE application specific
	 */
	AdvUpdateProcessId = osThreadNew(AdvUpdateProcess, NULL, &AdvUpdateProcess_attr);

	/**
	 * Initialization of ADV - Ad Manufacturer Element - Support OTA Bit Mask
	 */
#if(BLE_CFG_OTA_REBOOT_CHAR != 0)
	manuf_data[sizeof(manuf_data)-8] = CFG_FEATURE_OTA_REBOOT;
#endif
#if(RADIO_ACTIVITY_EVENT != 0)
	aci_hal_set_radio_activity_mask(0x0006);
#endif


	/* todo: error end */


	/**
	 * From here, all initialization are BLE application specific
	 */
#if(CFG_BLE_PERIPHERAL != 0)
	//ST SW Engineers converted the below statement to a FreeRTOS-friendly function: Adv_Request()
	//  UTIL_SEQ_RegTask( 1<<CFG_TASK_START_ADV_ID, UTIL_SEQ_RFU, Adv_Request);
	/**
	 * Create timer for Data Throughput process (write data)
	 */
	HW_TS_Create(CFG_TIM_PROC_ID_ISR, &(TimerDataThroughputWrite_Id), hw_ts_SingleShot, DataThroughput_proc);
#endif

#if(CFG_BLE_CENTRAL != 0)
	//TODO: below not converted to FreeRTOS but the goal is to implement the peripheral above
	//  UTIL_SEQ_RegTask( 1<<CFG_TASK_START_SCAN_ID, UTIL_SEQ_RFU, Scan_Request);
	//  UTIL_SEQ_RegTask( 1<<CFG_TASK_CONN_DEV_1_ID, UTIL_SEQ_RFU, Connect_Request);
	//  UTIL_SEQ_RegTask( 1<<CFG_TASK_CONN_UPDATE_ID, UTIL_SEQ_RFU, Connection_Update);
#endif


	LinkConfigProcessId= osThreadNew(LinkConfiguration, NULL, &LinkConfigProcess_attr);

	BleApplicationContext.DeviceServerFound = NO_DEVICE_FOUND;

	/**
	 * Clear DataBase
	 */
	aci_gap_clear_security_db();

	/**
	 * Initialize Data Client (this shouldn''t ideally happen but some of ST's example code for server
	 * relies on functions within this function)
	 * todo: rip out what's needed and fold it into DTS_App_Init()
	 */

	DTC_App_Init();

	/**
	 * Initialize Data Server (GATT SERVER)
	 */

	DTS_App_Init();

	//  //TODO: ripped from heartbeat. I think this sends the manufacturer information to the connecting device
	//  /**
	//   * Initialize DIS Application
	//   */
	//  DISAPP_Init();


	/**
	 * Initialize P2P Server Application
	 */
	//TODO: there is a SEQUENCER call here that should be FreeRTOS.... commenting for now
	//  P2PS_APP_Init();


#ifndef DYNAMIC_MODE
	/**
	 * Create timer to handle the Advertising Stop
	 */
	HW_TS_Create(CFG_TIM_PROC_ID_ISR, &(BleApplicationContext.Advertising_mgr_timer_Id), hw_ts_SingleShot, Adv_Cancel_Req);
#endif
	/**
	 * Create timer to handle the Led Switch OFF
	 */
	//  HW_TS_Create(CFG_TIM_PROC_ID_ISR, &(BleApplicationContext.SwitchOffGPIO_timer_Id), hw_ts_SingleShot, Switch_OFF_GPIO);
	return;
}

void APP_BLE_Init_Dyn_2( void ) {
	//  /**
	//   * Make device discoverable
	//   */
	//  BleApplicationContext.BleApplicationContext_legacy.advtServUUID[0] = NULL; //TODO: the heartbeat example uses: AD_TYPE_16_BIT_SERV_UUID
	//  BleApplicationContext.BleApplicationContext_legacy.advtServUUIDlen = 0;
	//  /* Initialize intervals for reconnexion without intervals update */
	AdvIntervalMin = CFG_FAST_CONN_ADV_INTERVAL_MIN;
	AdvIntervalMax = CFG_FAST_CONN_ADV_INTERVAL_MAX;

	/**
	 * Start to Advertise to be connected by P2P Client
	 */
#ifndef DYNAMIC_MODE
	Adv_Request(APP_BLE_FAST_ADV);
#else
	osThreadFlagsSet(AdvUpdateProcessId, 1);
//	Adv_Request(APP_BLE_LP_ADV);
#endif
	/* USER CODE BEGIN APP_BLE_Init_2 */

	/* USER CODE END APP_BLE_Init_2 */
	return;
}

SVCCTL_UserEvtFlowStatus_t SVCCTL_App_Notification( void *pckt )
{
	hci_event_pckt *event_pckt;
	evt_le_meta_event *meta_evt;
	evt_blue_aci *blue_evt;

	hci_le_connection_complete_event_rp0 * connection_complete_event;
	hci_le_advertising_report_event_rp0 * le_advertising_event;
	hci_le_phy_update_complete_event_rp0 *evt_le_phy_update_complete;
	hci_le_connection_update_complete_event_rp0 *connection_update_complete;
	uint8_t event_type, event_data_size;
	int k = 0;
	uint8_t adtype, adlength;
	uint8_t *adv_report_data;
	float Connection_Interval;
	float Supervision_Timeout;

	event_pckt = (hci_event_pckt*) ((hci_uart_pckt *) pckt)->data;

	switch (event_pckt->evt)
	{
	case EVT_DISCONN_COMPLETE:
	{
		hci_disconnection_complete_event_rp0 *disconnection_complete_event;
		disconnection_complete_event = (hci_disconnection_complete_event_rp0 *) event_pckt->data;

		if (disconnection_complete_event->Connection_Handle == BleApplicationContext.BleApplicationContext_legacy.connectionHandle)
		{
			BleApplicationContext.BleApplicationContext_legacy.connectionHandle = 0;
			BleApplicationContext.Device_Connection_Status = APP_BLE_IDLE;
			APP_DBG_MSG("\r\n\r** DISCONNECTION EVENT WITH CLIENT \n");
		}
		/* restart advertising */
#ifndef DYNAMIC_MODE
		Adv_Request(APP_BLE_FAST_ADV);
#else
		osThreadFlagsSet(AdvUpdateProcessId, 1);
//		Adv_Request(APP_BLE_LP_ADV);
#endif
		//       Adv_Request(APP_BLE_FAST_ADV);
//		/*
//		 * SPECIFIC to P2P Server APP
//		 */
//		handleNotification.P2P_Evt_Opcode = PEER_DISCON_HANDLE_EVT;
//		handleNotification.ConnectionHandle = BleApplicationContext.BleApplicationContext_legacy.connectionHandle;
//		P2PS_APP_Notification(&handleNotification);

		/* USER CODE BEGIN EVT_DISCONN_COMPLETE */

		/* USER CODE END EVT_DISCONN_COMPLETE */
	}

	break; /* EVT_DISCONN_COMPLETE */




	case EVT_LE_META_EVENT:
	{
		meta_evt = (evt_le_meta_event*) event_pckt->data;
		/* USER CODE BEGIN EVT_LE_META_EVENT */

		/* USER CODE END EVT_LE_META_EVENT */
		switch (meta_evt->subevent)
		{
		case EVT_LE_PHY_UPDATE_COMPLETE:
			evt_le_phy_update_complete = (hci_le_phy_update_complete_event_rp0*)meta_evt->data;
			if (evt_le_phy_update_complete->Status == 0)
			{
				APP_DBG_MSG("EVT_UPDATE_PHY_COMPLETE, success \n");
			}
			else
			{
				APP_DBG_MSG("EVT_UPDATE_PHY_COMPLETE, failure %d \n", evt_le_phy_update_complete->Status);
			}

			//todo: this is for central only so can delete
//	          UTIL_SEQ_SetEvt(1 << CFG_IDLEEVT_GAP_PROC_COMPLETE);

			break;
		case EVT_LE_CONN_UPDATE_COMPLETE:
			//TODO: added from throughput example
			mutex = 1;
			connection_update_complete = (hci_le_connection_update_complete_event_rp0*)meta_evt->data;

			APP_DBG_MSG("EVT_LE_CONN_UPDATE_COMPLETE \n");
			Connection_Interval = connection_update_complete->Conn_Interval * 1.25;
			APP_DBG_MSG("interval= %.2f ms \n",Connection_Interval);
			APP_DBG_MSG("latency= 0x%x \n",connection_update_complete->Conn_Latency);
			Supervision_Timeout = connection_update_complete->Supervision_Timeout * 10;
			APP_DBG_MSG("supervision_timeout= %.2f ms \n",Supervision_Timeout);

			break;
		case EVT_LE_CONN_COMPLETE:
			connection_complete_event = (hci_le_connection_complete_event_rp0 *) meta_evt->data;

			BleApplicationContext.BleApplicationContext_legacy.connectionHandle = connection_complete_event->Connection_Handle;
#if(CFG_BLE_PERIPHERAL != 0)
			APP_DBG_MSG("EVT_LE_CONN_COMPLETE connection as slave\n");
#endif

#if(CFG_BLE_CENTRAL != 0)
			APP_DBG_MSG("EVT_LE_CONN_COMPLETE connection as master\n");
#endif
			Connection_Interval = connection_complete_event->Conn_Interval * 1.25;

			APP_DBG_MSG("EVT_LE_CONN_COMPLETE connection as slave\n");
			APP_DBG_MSG("interval= %.2f ms \n",Connection_Interval);
			APP_DBG_MSG("latency= 0x%x \n",connection_complete_event->Conn_Latency);
			Supervision_Timeout = connection_complete_event->Supervision_Timeout * 10;
			APP_DBG_MSG("supervision_timeout= %.2f ms \n",Supervision_Timeout);

			//	          UTIL_SEQ_SetTask(1 << CFG_TASK_LINK_CONFIG_ID, CFG_SCH_PRIO_0);
			osThreadFlagsSet( LinkConfigProcessId, 1 );

			//todo: removed below from example and replaced from BLE DT example
			//          hci_le_connection_complete_event_rp0 *connection_complete_event;
			//
			//          /**
			//           * The connection is done, there is no need anymore to schedule the LP ADV
			//           */
			//          connection_complete_event = (hci_le_connection_complete_event_rp0 *) meta_evt->data;
			//
			//          HW_TS_Stop(BleApplicationContext.Advertising_mgr_timer_Id);
			//
			//            APP_DBG_MSG("EVT_LE_CONN_COMPLETE for connection handle 0x%x\n",
			//                      connection_complete_event->Connection_Handle);
			//
			//            if (BleApplicationContext.Device_Connection_Status == APP_BLE_LP_CONNECTING)
			//            {
			//              /* Connection as client */
			//              BleApplicationContext.Device_Connection_Status = APP_BLE_CONNECTED_CLIENT;
			//            }
			//            else
			//            {
			//              /* Connection as server */
			//              BleApplicationContext.Device_Connection_Status = APP_BLE_CONNECTED_SERVER;
			//            }
			//
			//            BleApplicationContext.BleApplicationContext_legacy.connectionHandle =
			//                connection_complete_event->Connection_Handle;

			//todo: do we need the P2P APP calls still?
			/*
			 * SPECIFIC to P2P Server APP
			 */
			//          handleNotification.P2P_Evt_Opcode = PEER_CONN_HANDLE_EVT;
			//          handleNotification.ConnectionHandle = BleApplicationContext.BleApplicationContext_legacy.connectionHandle;
			//          P2PS_APP_Notification(&handleNotification);
			/* USER CODE BEGIN HCI_EVT_LE_CONN_COMPLETE */

			//          osThreadFlagsSet( LinkConfigProcessId, 1 );
			/* USER CODE END HCI_EVT_LE_CONN_COMPLETE */

		break; /* HCI_EVT_LE_CONN_COMPLETE */

		case EVT_LE_ADVERTISING_REPORT:

			le_advertising_event = (hci_le_advertising_report_event_rp0 *) meta_evt->data;

			event_type = le_advertising_event->Advertising_Report[0].Event_Type;

			event_data_size = le_advertising_event->Advertising_Report[0].Length_Data;

			adv_report_data = (uint8_t*)(&le_advertising_event->Advertising_Report[0].Length_Data) + 1;
			k = 0;

			/* search AD TYPE AD_TYPE_COMPLETE_LOCAL_NAME (Complete Local Name) */
			/* search AD Type AD_TYPE_16_BIT_SERV_UUID (16 bits UUIDS) */
			if (event_type == ADV_IND)
			{
				/*ISOLATION OF BD ADDRESS AND LOCAL NAME*/
				while(k < event_data_size)
				{
					adlength = adv_report_data[k];
					adtype = adv_report_data[k + 1];
					switch (adtype)
					{
					case AD_TYPE_FLAGS: /* now get flags */
					break;

					case AD_TYPE_TX_POWER_LEVEL: /* Tx power level */
						break;

					case AD_TYPE_MANUFACTURER_SPECIFIC_DATA: /* Manufacturer Specific */
						if (adlength >= 7 && adv_report_data[k + 2] == 0x01)
						{ /* ST VERSION ID 01 */
							APP_DBG_MSG("--- ST MANUFACTURER ID --- \n");
							switch (adv_report_data[k + 3])
							{   /* Demo ID */
							case CFG_DEV_ID_PERIPH_SERVER:   /* (Periph Server DT) */
								APP_DBG_MSG("-- SERVER DETECTED -- VIA MAN ID\n");
								BleApplicationContext.DeviceServerFound = AT_LEAST_ONE_DEVICE_FOUND;
								SERVER_REMOTE_BDADDR[0] = le_advertising_event->Advertising_Report[0].Address[0];
								SERVER_REMOTE_BDADDR[1] = le_advertising_event->Advertising_Report[0].Address[1];
								SERVER_REMOTE_BDADDR[2] = le_advertising_event->Advertising_Report[0].Address[2];
								SERVER_REMOTE_BDADDR[3] = le_advertising_event->Advertising_Report[0].Address[3];
								SERVER_REMOTE_BDADDR[4] = le_advertising_event->Advertising_Report[0].Address[4];
								SERVER_REMOTE_BDADDR[5] = le_advertising_event->Advertising_Report[0].Address[5];

								/* The device has been found - scan may be stopped */
								aci_gap_terminate_gap_proc(GAP_GENERAL_DISCOVERY_PROC);
								break;

							default:
								break;
							}
						}
						break;

					case AD_TYPE_SERVICE_DATA: /* service data 16 bits */
						break;

					default:
						break;
					} /* end switch adtype */
					k += adlength + 1;
				} /* end while */

			} /*end if ADV_IND */
			break;

		default:
			/* USER CODE BEGIN SUBEVENT_DEFAULT */

			/* USER CODE END SUBEVENT_DEFAULT */
			break;
		}
	}
	break; /* HCI_EVT_LE_META_EVENT */

	case EVT_VENDOR:
		blue_evt = (evt_blue_aci*) event_pckt->data;
		/* USER CODE BEGIN EVT_VENDOR */

		/* USER CODE END EVT_VENDOR */
		switch (blue_evt->ecode)
		{
		/* USER CODE BEGIN ecode */
		case EVT_BLUE_GAP_PAIRING_CMPLT:
			APP_DBG_MSG("Pairing complete \n");
			BSP_LED_On(LED_RED);
			BSP_LED_On(LED_BLUE);
			//	   UTIL_SEQ_SetEvt(1 << CFG_IDLEEVT_GAP_PROC_COMPLETE);
			break;

		case EVT_BLUE_GAP_PASS_KEY_REQUEST:
			APP_DBG_MSG("respond to the passkey request\n");
			BSP_LED_On(LED_BLUE);
			BSP_LED_On(LED_GREEN);
			aci_gap_pass_key_resp(BleApplicationContext.BleApplicationContext_legacy.connectionHandle, 111111);
			break;

		case (EVT_BLUE_GAP_NUMERIC_COMPARISON_VALUE):
			   APP_DBG_MSG("Hex_value = %ld\n",
					   ((aci_gap_numeric_comparison_value_event_rp0 *)(blue_evt->data))->Numeric_Value);
		BSP_LED_On(LED_RED);
		BSP_LED_On(LED_BLUE);
		BSP_LED_On(LED_GREEN);

		aci_gap_numeric_comparison_value_confirm_yesno(BleApplicationContext.BleApplicationContext_legacy.connectionHandle, 1); /* CONFIRM_YES = 1 */

		APP_DBG_MSG("\r\n\r** aci_gap_numeric_comparison_value_confirm_yesno-->YES \n");
		break;

		case EVT_BLUE_GATT_TX_POOL_AVAILABLE:
			BSP_LED_On(LED_RED);
			DTS_App_TxPoolAvailableNotification();
			break;
			/* USER CODE END ecode */
			/*
			 * SPECIFIC to P2P Server APP
			 */
		case EVT_BLUE_L2CAP_CONNECTION_UPDATE_RESP:
#if (L2CAP_REQUEST_NEW_CONN_PARAM != 0 )
			mutex = 1;
#endif
			/* USER CODE BEGIN EVT_BLUE_L2CAP_CONNECTION_UPDATE_RESP */

			/* USER CODE END EVT_BLUE_L2CAP_CONNECTION_UPDATE_RESP */
			break;
		case EVT_BLUE_GAP_PROCEDURE_COMPLETE:
			APP_DBG_MSG("\r\n\r** EVT_BLUE_GAP_PROCEDURE_COMPLETE \n");
			/* USER CODE BEGIN EVT_BLUE_GAP_PROCEDURE_COMPLETE */
			aci_gap_proc_complete_event_rp0 *gap_evt_proc_complete = (void*) blue_evt->data;
			/* CHECK GAP GENERAL DISCOVERY PROCEDURE COMPLETED & SUCCEED */
			if (gap_evt_proc_complete->Procedure_Code == GAP_GENERAL_DISCOVERY_PROC)
			{
				if( gap_evt_proc_complete->Status != BLE_STATUS_SUCCESS )
				{
					APP_DBG_MSG("-- GAP GENERAL DISCOVERY PROCEDURE FAILED\n");
				}
				else
				{
					//			   BSP_LED_On(LED_BLUE);
					BSP_LED_On(LED_RED);

					/*if a device found, connect to it, device 1 being chosen first if both found*/
					if (BleApplicationContext.DeviceServerFound != NO_DEVICE_FOUND)
					{
						APP_DBG_MSG("-- GAP GENERAL DISCOVERY PROCEDURE COMPLETED\n");
						//todo: the below is just for a BLE Central Node
						//				 UTIL_SEQ_SetTask(1 << CFG_TASK_CONN_DEV_1_ID, CFG_SCH_PRIO_0);
					}
					else
					{
						APP_DBG_MSG("-- GAP GENERAL DISCOVERY PROCEDURE COMPLETED WITH NO DEVICE FOUND\n");
					}
				}
			}
			/* USER CODE END EVT_BLUE_GAP_PROCEDURE_COMPLETE */
			break; /* EVT_BLUE_GAP_PROCEDURE_COMPLETE */

#if(RADIO_ACTIVITY_EVENT != 0)
		case 0x0004:
			/* USER CODE BEGIN RADIO_ACTIVITY_EVENT*/
			//          BSP_LED_On(LED_GREEN);
			HW_TS_Start(BleApplicationContext.SwitchOffGPIO_timer_Id, (uint32_t)LED_ON_TIMEOUT);
			/* USER CODE END RADIO_ACTIVITY_EVENT*/
			break; /* RADIO_ACTIVITY_EVENT */
#endif
		}
		break; /* EVT_VENDOR */

		default:
			/* USER CODE BEGIN ECODE_DEFAULT*/

			/* USER CODE END ECODE_DEFAULT*/
			break;
	}

	return (SVCCTL_UserEvtFlowEnable);
}

APP_BLE_ConnStatus_t APP_BLE_Get_Server_Connection_Status(void)
{
	return BleApplicationContext.Device_Connection_Status;
}

/* USER CODE BEGIN FD*/
void APP_BLE_Key_Button1_Action(void)
{
	P2PS_APP_SW1_Button_Action();
}

void APP_BLE_Key_Button2_Action(void)
{
#if (L2CAP_REQUEST_NEW_CONN_PARAM != 0 )    
	if (BleApplicationContext.Device_Connection_Status != APP_BLE_FAST_ADV && BleApplicationContext.Device_Connection_Status != APP_BLE_IDLE)
	{
		BLE_SVC_L2CAP_Conn_Update(BleApplicationContext.BleApplicationContext_legacy.connectionHandle);

	}
	return;
#endif    
}

void APP_BLE_Key_Button3_Action(void)
{
}

void APP_BLE_Stop(void)
{
	/* BLE STOP Procedure */
	aci_hal_stack_reset();

	APP_DBG("INSERT SOME WAIT");
	HAL_Delay(100);

	/* Stop Advertising Timer */
	HW_TS_Stop(BleApplicationContext.Advertising_mgr_timer_Id);
	HW_TS_Delete(BleApplicationContext.Advertising_mgr_timer_Id);
}
/* USER CODE END FD*/
/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
static void Ble_Tl_Init( void )
{
	HCI_TL_HciInitConf_t Hci_Tl_Init_Conf;

	Hci_Tl_Init_Conf.p_cmdbuffer = (uint8_t*)&BleCmdBuffer;
	Hci_Tl_Init_Conf.StatusNotCallBack = BLE_StatusNot;
	hci_init(BLE_UserEvtRx, (void*) &Hci_Tl_Init_Conf);

	return;
}

static void Ble_Hci_Gap_Gatt_Init(void){

	uint8_t role;
//	uint8_t index;
	uint16_t gap_service_handle, gap_dev_name_char_handle, gap_appearance_char_handle;
	const uint8_t *bd_addr;
	uint32_t srd_bd_addr[2];
	uint16_t appearance[1] = { BLE_CFG_GAP_APPEARANCE };

	/**
	 * Initialize HCI layer
	 */
	/*HCI Reset to synchronise BLE Stack*/
	hci_reset();

	/**
	 * Write the BD Address
	 */

	bd_addr = BleGetBdAddress();
	aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
			CONFIG_DATA_PUBADDR_LEN,
			(uint8_t*) bd_addr);

	/* BLE MAC in ADV Packet */
	//  manuf_data[ sizeof(manuf_data)-6] = bd_addr[5];
	//  manuf_data[ sizeof(manuf_data)-5] = bd_addr[4];
	//  manuf_data[ sizeof(manuf_data)-4] = bd_addr[3];
	//  manuf_data[ sizeof(manuf_data)-3] = bd_addr[2];
	//  manuf_data[ sizeof(manuf_data)-2] = bd_addr[1];
	//  manuf_data[ sizeof(manuf_data)-1] = bd_addr[0];

	/**
	 * Static random Address
	 * The two upper bits shall be set to 1
	 * The lowest 32bits is read from the UDN to differentiate between devices
	 * The RNG may be used to provide a random number on each power on
	 */
	srd_bd_addr[1] =  0x0000ED6E;
	srd_bd_addr[0] =  LL_FLASH_GetUDN( );
	aci_hal_write_config_data( CONFIG_DATA_RANDOM_ADDRESS_OFFSET, CONFIG_DATA_RANDOM_ADDRESS_LEN, (uint8_t*)srd_bd_addr );

	/**
	 * Write Identity root key used to derive LTK and CSRK
	 */
	aci_hal_write_config_data( CONFIG_DATA_IR_OFFSET, CONFIG_DATA_IR_LEN, (uint8_t*)BLE_CFG_IR_VALUE );

	/**
	 * Write Encryption root key used to derive LTK and CSRK
	 */
	aci_hal_write_config_data( CONFIG_DATA_ER_OFFSET, CONFIG_DATA_ER_LEN, (uint8_t*)BLE_CFG_ER_VALUE );

	/**
	 * Set TX Power to 0dBm.
	 */
	aci_hal_set_tx_power_level(1, CFG_TX_POWER);

	/**
	 * Initialize GATT interface
	 */
	aci_gatt_init();

	/**
	 * Initialize GAP interface
	 */
	role = 0;

#if (BLE_CFG_PERIPHERAL == 1)
	role |= GAP_PERIPHERAL_ROLE;
#endif

#if (BLE_CFG_CENTRAL == 1)
	role |= GAP_CENTRAL_ROLE;
#endif

	if (role > 0)
	{
		const char *name = "CAPTIVATE";
		aci_gap_init(role, 0,
				APPBLE_GAP_DEVICE_NAME_LENGTH,
				&gap_service_handle, &gap_dev_name_char_handle, &gap_appearance_char_handle);

		if (aci_gatt_update_char_value(gap_service_handle, gap_dev_name_char_handle, 0, strlen(name), (uint8_t *) name))
		{
			BLE_DBG_SVCCTL_MSG("Device Name aci_gatt_update_char_value failed.\n");
		}
	}

	if(aci_gatt_update_char_value(gap_service_handle,
			gap_appearance_char_handle,
			0,
			2,
			(uint8_t *)&appearance))
	{
		BLE_DBG_SVCCTL_MSG("Appearance aci_gatt_update_char_value failed.\n");
	}
	///**
	//   * Initialize Default PHY
	//   */
	//  hci_le_set_default_phy(ALL_PHYS_PREFERENCE,TX_2M_PREFERRED,RX_2M_PREFERRED);

	/**
	 * Initialize IO capability
	 */
	BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.ioCapability = CFG_IO_CAPABILITY;
	aci_gap_set_io_capability(BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.ioCapability);

	/**
	 * Initialize authentication
	 */
	//  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode = CFG_MITM_PROTECTION;
	//  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.OOB_Data_Present = 0;
	//  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin = 8;
	//  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax = 16;
	//  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Use_Fixed_Pin = 1;
	//  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Fixed_Pin = 111111;
	//  BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode = 1;
	//  for (index = 0; index < 16; index++)
	//  {
	//    BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.OOB_Data[index] = (uint8_t) index;
	//  }
	//
	//  aci_gap_set_authentication_requirement(BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode,
	//                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode,
	//                                         0,
	//                                         0,
	//                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin,
	//                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax,
	//                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Use_Fixed_Pin,
	//                                         BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Fixed_Pin,
	//                                         0
	//  );
	BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode = CFG_MITM_PROTECTION;
	BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin = CFG_ENCRYPTION_KEY_SIZE_MIN;
	BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax = CFG_ENCRYPTION_KEY_SIZE_MAX;
	BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Use_Fixed_Pin = CFG_USED_FIXED_PIN;
	BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Fixed_Pin = CFG_FIXED_PIN;
	BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode = CFG_BONDING_MODE;

	aci_gap_set_authentication_requirement(BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode,
			BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.mitm_mode,
			CFG_SC_SUPPORT,
			CFG_KEYPRESS_NOTIFICATION_SUPPORT,
			BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMin,
			BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.encryptionKeySizeMax,
			BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Use_Fixed_Pin,
			BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.Fixed_Pin,
			PUBLIC_ADDR
	);

	/**
	 * Initialize whitelist
	 */
	 if (BleApplicationContext.BleApplicationContext_legacy.bleSecurityParam.bonding_mode)
	 {
		 aci_gap_configure_whitelist();
	 }
}

static void Adv_Request(APP_BLE_ConnStatus_t New_Status)
{
	tBleStatus ret = BLE_STATUS_INVALID_PARAMS;
	uint16_t Min_Inter, Max_Inter;

	if (New_Status == APP_BLE_FAST_ADV)
	{
		Min_Inter = AdvIntervalMin;
		Max_Inter = AdvIntervalMax;
	}
	else
	{
		Min_Inter = CFG_LP_CONN_ADV_INTERVAL_MIN;
		Max_Inter = CFG_LP_CONN_ADV_INTERVAL_MAX;
	}

	/**
	 * Stop the timer, it will be restarted for a new shot
	 * It does not hurt if the timer was not running
	 */
	HW_TS_Stop(BleApplicationContext.Advertising_mgr_timer_Id);

	APP_DBG_MSG("First index in %d state \n",
			BleApplicationContext.Device_Connection_Status);

	if ((New_Status == APP_BLE_LP_ADV)
			&& ((BleApplicationContext.Device_Connection_Status == APP_BLE_FAST_ADV)
					|| (BleApplicationContext.Device_Connection_Status == APP_BLE_LP_ADV)))
	{
		/* Connection in ADVERTISE mode have to stop the current advertising */
		ret = aci_gap_set_non_discoverable();
		if (ret == BLE_STATUS_SUCCESS)
		{
			APP_DBG_MSG("Successfully Stopped Advertising");
		}
		else
		{
			APP_DBG_MSG("Stop Advertising Failed , result: %d \n", ret);
		}
	}

	BleApplicationContext.Device_Connection_Status = New_Status;
	/* Start Fast or Low Power Advertising */
	//    ret = aci_gap_set_discoverable(
	//        ADV_IND,
	//        Min_Inter,
	//        Max_Inter,
	//        PUBLIC_ADDR,
	//        NO_WHITE_LIST_USE, /* use white list */
	//        sizeof(local_name),
	//        (uint8_t*) local_name,
	//        BleApplicationContext.BleApplicationContext_legacy.advtServUUIDlen,
	//        BleApplicationContext.BleApplicationContext_legacy.advtServUUID,
	//        6,
	//        8);
	ret = aci_gap_set_discoverable(ADV_IND,
			FAST_CONN_ADV_INTERVAL_MIN,
			FAST_CONN_ADV_INTERVAL_MAX,
			PUBLIC_ADDR,
			NO_WHITE_LIST_USE, /* use white list */
			sizeof(local_name), (uint8_t*) local_name, 0,
			NULL,
			6, 8);
	/* Update Advertising data */
	ret = aci_gap_update_adv_data(sizeof(manuf_data), (uint8_t*) manuf_data);
	//    ret = aci_gap_update_adv_data(22, (uint8_t*) manuf_data);

	if (ret == BLE_STATUS_SUCCESS)
	{
		if (New_Status == APP_BLE_FAST_ADV)
		{
			APP_DBG_MSG("Successfully Start Fast Advertising \n" );
			/* Start Timer to STOP ADV - TIMEOUT */
			HW_TS_Start(BleApplicationContext.Advertising_mgr_timer_Id, INITIAL_ADV_TIMEOUT);
		}
		else
		{
			APP_DBG_MSG("\n\rSuccessfully Start Low Power Advertising \n\r");
		}
	}
	else
	{
		if (New_Status == APP_BLE_FAST_ADV)
		{
			APP_DBG_MSG("Start Fast Advertising Failed , result: %d \n\r", ret);
		}
		else
		{
			APP_DBG_MSG("Start Low Power Advertising Failed , result: %d \n", ret);
		}
	}

	return;
}

const uint8_t* BleGetBdAddress( void )
{
	uint8_t *otp_addr;
	const uint8_t *bd_addr;
	uint32_t udn;
	uint32_t company_id;
	uint32_t device_id;

	udn = LL_FLASH_GetUDN();

	if(udn != 0xFFFFFFFF)
	{
		company_id = LL_FLASH_GetSTCompanyID();
		device_id = LL_FLASH_GetDeviceID();

		bd_addr_udn[0] = (uint8_t)(udn & 0x000000FF);
		bd_addr_udn[1] = (uint8_t)( (udn & 0x0000FF00) >> 8 );
		bd_addr_udn[2] = (uint8_t)( (udn & 0x00FF0000) >> 16 );
		bd_addr_udn[3] = (uint8_t)device_id;
		bd_addr_udn[4] = (uint8_t)(company_id & 0x000000FF);;
		bd_addr_udn[5] = (uint8_t)( (company_id & 0x0000FF00) >> 8 );

		bd_addr = (const uint8_t *)bd_addr_udn;
	}
	else
	{
		otp_addr = OTP_Read(0);
		if(otp_addr)
		{
			bd_addr = ((OTP_ID0_t*)otp_addr)->bd_address;
		}
		else
		{
			bd_addr = M_bd_addr;
		}

	}

	return bd_addr;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTION */
uint8_t APP_BLE_ComputeCRC8( uint8_t *DataPtr , uint8_t Datalen )
{
	uint8_t i, j;
	const uint8_t PolynomeCRC = 0x97;
	uint8_t CRC8 = 0x00;

	for (i = 0; i < Datalen; i++)
	{
		CRC8 ^= DataPtr[i];
		for (j = 0; j < 8; j++)
		{
			if ((CRC8 & 0x80) != 0)
			{
				CRC8 = (uint8_t) ((CRC8 << 1) ^ PolynomeCRC);
			}
			else
			{
				CRC8 <<= 1;
			}
		}
	}
	return (CRC8);
}

static void Add_Advertisment_Service_UUID( uint16_t servUUID )
{
//	BleApplicationContext.BleApplicationContext_legacy.advtServUUID[BleApplicationContext.BleApplicationContext_legacy.advtServUUIDlen] =
//			(uint8_t) (servUUID & 0xFF);
//	BleApplicationContext.BleApplicationContext_legacy.advtServUUIDlen++;
//	BleApplicationContext.BleApplicationContext_legacy.advtServUUID[BleApplicationContext.BleApplicationContext_legacy.advtServUUIDlen] =
//			(uint8_t) (servUUID >> 8) & 0xFF;
//	BleApplicationContext.BleApplicationContext_legacy.advtServUUIDlen++;

	return;
}

static void Adv_Mgr( void )
{
	/**
	 * The code shall be executed in the background as an aci command may be sent
	 * The background is the only place where the application can make sure a new aci command
	 * is not sent if there is a pending one
	 */
//	osThreadFlagsSet( AdvUpdateProcessId, 1 );

	return;
}

static void AdvUpdateProcess(void *argument)
{
	UNUSED(argument);

	for(;;)
	{
		osThreadFlagsWait( 1, osFlagsWaitAny, osWaitForever);
		Adv_Update( );
	}
}

//void Adv_Request_TP( void )
//{
//	tBleStatus result;
//
//	result = aci_gap_set_discoverable(ADV_IND,
//			FAST_CONN_ADV_INTERVAL_MIN,
//			FAST_CONN_ADV_INTERVAL_MAX,
//			PUBLIC_ADDR,
//			NO_WHITE_LIST_USE, /* use white list */
//			sizeof(local_name), (uint8_t*) local_name, 0,
//			NULL,
//			6, 8);
//	if (result == BLE_STATUS_SUCCESS)
//	{
//		APP_DBG_MSG("  \r\n\r");APP_DBG_MSG("** START ADVERTISING **  \r\n\r");
//	}
//	else
//	{
//		APP_DBG_MSG("** START ADVERTISING **  Failed \r\n\r");
//	}
//
//	/* Send Advertising data */
//	result = aci_gap_update_adv_data(22, (uint8_t*) manuf_data);
//
//	if (result == BLE_STATUS_SUCCESS)
//	{
//		APP_DBG_MSG("  \r\n\r");APP_DBG_MSG("** add ADV data **  \r\n\r");
//	}
//	else
//	{
//		APP_DBG_MSG("** add ADV data **  Failed \r\n\r");
//	}
//	return;
//}

static void Adv_Update( void )
{
	Adv_Request(APP_BLE_LP_ADV);

	return;
}

static void HciUserEvtProcess(void *argument)
{
	UNUSED(argument);

	for(;;)
	{
		osThreadFlagsWait( 1, osFlagsWaitAny, osWaitForever);
		hci_user_evt_proc( );
	}
}

#if (CFG_BLE_PERIPHERAL != 0)
//void Adv_Request( void )
//{
//  tBleStatus result;
//
//  result = aci_gap_set_discoverable(ADV_IND,
//                                    FAST_CONN_ADV_INTERVAL_MIN,
//                                    FAST_CONN_ADV_INTERVAL_MAX,
//                                    PUBLIC_ADDR,
//                                    NO_WHITE_LIST_USE, /* use white list */
//                                    sizeof(local_name), (uint8_t*) local_name, 0,
//                                    NULL,
//                                    6, 8);
//  if (result == BLE_STATUS_SUCCESS)
//  {
//    APP_DBG_MSG("  \r\n\r");APP_DBG_MSG("** START ADVERTISING **  \r\n\r");
//  }
//  else
//  {
//    APP_DBG_MSG("** START ADVERTISING **  Failed \r\n\r");
//  }
//
//  /* Send Advertising data */
//  result = aci_gap_update_adv_data(22, (uint8_t*) manuf_data);
//
//  if (result == BLE_STATUS_SUCCESS)
//  {
//    APP_DBG_MSG("  \r\n\r");APP_DBG_MSG("** add ADV data **  \r\n\r");
//  }
//  else
//  {
//    APP_DBG_MSG("** add ADV data **  Failed \r\n\r");
//  }
//  return;
//}

void DataThroughput_proc(void){

	//  UTIL_SEQ_SetTask(1 << CFG_TASK_DATA_WRITE_ID, CFG_SCH_PRIO_0);

	osThreadFlagsSet( DataWriteProcessId, 1 );
}
#endif

void LinkConfiguration(void * argument)
{
	UNUSED(argument);
	tBleStatus status;

	while(1)
	{
		osThreadFlagsWait( 1, osFlagsWaitAny, osWaitForever);

#if (CFG_BLE_CENTRAL != 0)
		uint8_t tx_phy;
		uint8_t rx_phy;
#endif

		/**
		 * The client will start ATT configuration after the link is fully configured
		 * Setup PHY
		 * Setup Data Length
		 * Setup Pairing
		 */
#if (((CFG_TX_PHY == 2) || (CFG_RX_PHY == 2)) && (CFG_BLE_CENTRAL != 0))
		GapProcReq(GAP_PROC_SET_PHY);
#endif

#if (CFG_BLE_CENTRAL != 0)
		APP_DBG_MSG("Reading_PHY\n");
		status = hci_le_read_phy(BleApplicationContext.BleApplicationContext_legacy.connectionHandle,&tx_phy,&rx_phy);
		if (status != BLE_STATUS_SUCCESS)
		{
			APP_DBG_MSG("Read phy cmd failure: 0x%x \n", status);
		}
		else
		{
			APP_DBG_MSG("TX PHY = %d\n", tx_phy);
			APP_DBG_MSG("RX PHY = %d\n", rx_phy);
		}
#endif

		  APP_DBG_MSG("set data length \n");
//		  BSP_LED_On(LED_BLUE);
		status = hci_le_set_data_length(BleApplicationContext.BleApplicationContext_legacy.connectionHandle,251,2120);
		if (status != BLE_STATUS_SUCCESS)
		{
//				  BSP_LED_On(LED_RED);
			    APP_DBG_MSG("set data length command error \n");
		}

#if ((CFG_ENCRYPTION_ENABLE != 0) && (CFG_BLE_CENTRAL != 0))
		GapProcReq(GAP_PROC_PAIRING);
#endif

		DTC_App_LinkReadyNotification(BleApplicationContext.BleApplicationContext_legacy.connectionHandle);
	}
	//  while(1){
	//	  osDelay(10000);
	//  }
}

void BLE_SVC_GAP_Change_PHY(void)
{
	uint8_t TX_PHY, RX_PHY;
	tBleStatus ret = BLE_STATUS_INVALID_PARAMS;
	ret = hci_le_read_phy(BleApplicationContext.BleApplicationContext_legacy.connectionHandle,&TX_PHY,&RX_PHY);
	if (ret == BLE_STATUS_SUCCESS)
	{
		APP_DBG_MSG("Read_PHY success \n");
		APP_DBG_MSG("PHY Param  TX= %d, RX= %d \n", TX_PHY, RX_PHY);
		if ((TX_PHY == TX_2M) && (RX_PHY == RX_2M))
		{
			APP_DBG_MSG("hci_le_set_phy PHY Param  TX= %d, RX= %d \n", TX_1M, RX_1M);
			ret = hci_le_set_phy(BleApplicationContext.BleApplicationContext_legacy.connectionHandle,ALL_PHYS_PREFERENCE,TX_1M,RX_1M,0);
		}
		else
		{
			APP_DBG_MSG("hci_le_set_phy PHY Param  TX= %d, RX= %d \n", TX_2M_PREFERRED, RX_2M_PREFERRED);
			ret = hci_le_set_phy(BleApplicationContext.BleApplicationContext_legacy.connectionHandle,ALL_PHYS_PREFERENCE,TX_2M_PREFERRED,RX_2M_PREFERRED,0);
		}
	}
	else
	{
		APP_DBG_MSG("Read conf not succeess \n");
	}

	if (ret == BLE_STATUS_SUCCESS)
	{
		APP_DBG_MSG("set PHY cmd ok\n");
	}
	else
	{
		APP_DBG_MSG("set PHY cmd NOK\n");
	}

	return;
}

/* USER CODE END FD_LOCAL_FUNCTION */

/*************************************************************
 *
 *SPECIFIC FUNCTIONS FOR P2P SERVER
 *
 *************************************************************/
static void Adv_Cancel(void* argument )
{
	/* USER CODE BEGIN Adv_Cancel_1 */
	//  BSP_LED_Off(LED_GREEN);
	/* USER CODE END Adv_Cancel_1 */
	UNUSED(argument);
	for(;;)
	{
		osThreadFlagsWait( 1, osFlagsWaitAny, osWaitForever);
		if (BleApplicationContext.Device_Connection_Status != APP_BLE_CONNECTED_SERVER)

		{

			tBleStatus result = 0x00;

			result = aci_gap_set_non_discoverable();

			BleApplicationContext.Device_Connection_Status = APP_BLE_IDLE;
			if (result == BLE_STATUS_SUCCESS)
			{
				APP_DBG_MSG("  \r\n\r");APP_DBG_MSG("** STOP ADVERTISING **  \r\n\r");
			}
			else
			{
				APP_DBG_MSG("** STOP ADVERTISING **  Failed \r\n\r");
			}

		}
	}

	/* USER CODE BEGIN Adv_Cancel_2 */

	/* USER CODE END Adv_Cancel_2 */
	return;
}

#ifndef DYNAMIC_MODE
static void Adv_Cancel_Req( void )
{
	/* USER CODE BEGIN Adv_Cancel_Req_1 */

	/* USER CODE END Adv_Cancel_Req_1 */
	osThreadFlagsSet( AdvCancelProcessId, 1 );

	//  UTIL_SEQ_SetTask(1 << CFG_TASK_ADV_CANCEL_ID, CFG_SCH_PRIO_0);
	/* USER CODE BEGIN Adv_Cancel_Req_2 */

	/* USER CODE END Adv_Cancel_Req_2 */
	return;
}
#endif

static void Switch_OFF_GPIO(){
	/* USER CODE BEGIN Switch_OFF_GPIO */
	//  BSP_LED_Off(LED_GREEN);
	/* USER CODE END Switch_OFF_GPIO */
}

#if(L2CAP_REQUEST_NEW_CONN_PARAM != 0)  
void BLE_SVC_L2CAP_Conn_Update(uint16_t Connection_Handle)
{
	/* USER CODE BEGIN BLE_SVC_L2CAP_Conn_Update_1 */

	/* USER CODE END BLE_SVC_L2CAP_Conn_Update_1 */
	if(mutex == 1) {
		mutex = 0;
		index_con_int = (index_con_int + 1)%SIZE_TAB_CONN_INT;
		uint16_t interval_min = CONN_P(tab_conn_interval[index_con_int]);
		uint16_t interval_max = CONN_P(tab_conn_interval[index_con_int]);
		uint16_t slave_latency = L2CAP_SLAVE_LATENCY;
		uint16_t timeout_multiplier = L2CAP_TIMEOUT_MULTIPLIER;
		tBleStatus result;

		result = aci_l2cap_connection_parameter_update_req(BleApplicationContext.BleApplicationContext_legacy.connectionHandle,
				interval_min, interval_max,
				slave_latency, timeout_multiplier);
		if( result == BLE_STATUS_SUCCESS )
		{
			APP_DBG_MSG("BLE_SVC_L2CAP_Conn_Update(), Successfully \r\n\r");
		}
		else
		{
			APP_DBG_MSG("BLE_SVC_L2CAP_Conn_Update(), Failed \r\n\r");
		}
	}
	/* USER CODE BEGIN BLE_SVC_L2CAP_Conn_Update_2 */

	/* USER CODE END BLE_SVC_L2CAP_Conn_Update_2 */
	return;
}
#endif

/* USER CODE BEGIN FD_SPECIFIC_FUNCTIONS */

/* USER CODE END FD_SPECIFIC_FUNCTIONS */
/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/
void hci_notify_asynch_evt(void* pdata)
{
	UNUSED(pdata);
	osThreadFlagsSet( HciUserEvtProcessId, 1 );
	return;
}

void hci_cmd_resp_release(uint32_t flag)
{
	UNUSED(flag);
	osSemaphoreRelease( SemHciId );
	return;
}

void hci_cmd_resp_wait(uint32_t timeout)
{
	UNUSED(timeout);
	osSemaphoreAcquire( SemHciId, osWaitForever );
	return;
}

static void BLE_UserEvtRx( void * pPayload )
{
	SVCCTL_UserEvtFlowStatus_t svctl_return_status;
	tHCI_UserEvtRxParam *pParam;

	pParam = (tHCI_UserEvtRxParam *)pPayload;

	svctl_return_status = SVCCTL_UserEvtRx((void *)&(pParam->pckt->evtserial));
	if (svctl_return_status != SVCCTL_UserEvtFlowDisable)
	{
		pParam->status = HCI_TL_UserEventFlow_Enable;
	}
	else
	{
		pParam->status = HCI_TL_UserEventFlow_Disable;
	}
}

static void BLE_StatusNot( HCI_TL_CmdStatus_t status )
{
	switch (status)
	{
	case HCI_TL_CmdBusy:
		osMutexAcquire( MtxHciId, osWaitForever );
		break;

	case HCI_TL_CmdAvailable:
		osMutexRelease( MtxHciId );
		break;

	default:
		break;
	}
	return;
}

void SVCCTL_ResumeUserEventFlow( void )
{
	hci_resume_flow();
	return;
}

/* USER CODE BEGIN FD_WRAP_FUNCTIONS */
void SVCCTL_InitCustomSvc( void )
{
	DTS_STM_Init();
}

void BLE_SVC_L2CAP_Conn_Update_7_5(void)
{
/* USER CODE BEGIN BLE_SVC_L2CAP_Conn_Update_1 */

/* USER CODE END BLE_SVC_L2CAP_Conn_Update_1 */
  if(mutex == 1) {
    mutex = 0;
    uint16_t interval_min = CONN_P(7.5);
    uint16_t interval_max = CONN_P(7.5);
    uint16_t slave_latency = L2CAP_SLAVE_LATENCY;
    uint16_t timeout_multiplier = L2CAP_TIMEOUT_MULTIPLIER;
    tBleStatus result;

    result = aci_l2cap_connection_parameter_update_req(BleApplicationContext.BleApplicationContext_legacy.connectionHandle,
                                                       interval_min, interval_max,
                                                       slave_latency, timeout_multiplier);
    if( result == BLE_STATUS_SUCCESS )
    {
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("BLE_SVC_L2CAP_Conn_Update(), Successfully \r\n\r");
#endif
    }
    else
    {
#if(CFG_DEBUG_APP_TRACE != 0)
      APP_DBG_MSG("BLE_SVC_L2CAP_Conn_Update(), Failed \r\n\r");
#endif
    }
  }
/* USER CODE BEGIN BLE_SVC_L2CAP_Conn_Update_2 */

/* USER CODE END BLE_SVC_L2CAP_Conn_Update_2 */
  return;
}
/* USER CODE END FD_WRAP_FUNCTIONS */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
