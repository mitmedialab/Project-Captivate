/**
 ******************************************************************************
 * @file    dts.c
 * @author  MCD Application Team
 * @brief   Data Transfer Service (Custom)
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

/* Includes ------------------------------------------------------------------*/
#include "ble_common.h"
#include "ble.h"
#include "dbg_trace.h"
#include "dts.h"   

#include "captivate_config.h"
#include "master_thread.h"

//#include "main.h"

#include "lp5523.h"

#include "time.h"
#include "rtc.h"

#include "app_thread.h"

#define UUID_128_SUPPORTED 1
#define	NUM_OF_CHARACTERISTICS 6 //https://community.st.com/s/question/0D50X00009XkYAvSAN/sensortile-bluenrgms-custom-service-aci

#if (UUID_128_SUPPORTED == 1)
#define DT_UUID_LENGTH  UUID_TYPE_128
#else
#define DT_UUID_LENGTH  UUID_TYPE_16
#endif

#if (UUID_128_SUPPORTED == 1)
const uint8_t DT_REQ_CHAR_UUID[16] = { 0x19, 0xed, 0x82, 0xae, 0xed, 0x21, 0x4c,
		0x9d, 0x41, 0x45, 0x22, 0x8e, 0x81, 0xFE, 0x00, 0x00 };
#else
const uint8_t DT_REQ_CHAR_UUID[2] = { 0x81, 0xFE };
#endif

#if (UUID_128_SUPPORTED == 1)
const uint8_t DT_REQ_CHAR2_UUID[16] = { 0x19, 0xed, 0x82, 0xae, 0xed, 0x21,
		0x4c, 0x9d, 0x41, 0x45, 0x22, 0x8e, 0x82, 0xFE, 0x00, 0x00 };
#else
const uint8_t DT_REQ_CHAR2_UUID[2] = { 0x82, 0xFE };
#endif

#if (UUID_128_SUPPORTED == 1)
const uint8_t DT_REQ_CHAR3_UUID[16] = { 0x19, 0xed, 0x82, 0xae, 0xed, 0x21,
		0x4c, 0x9d, 0x41, 0x45, 0x22, 0x8e, 0x83, 0xFE, 0x00, 0x00 };
#else
const uint8_t DT_REQ_CHAR3_UUID[2] = { 0x83, 0xFE };
#endif

#if (UUID_128_SUPPORTED == 1)
const uint8_t DT_REQ_CHAR_LED_UUID[16] = { 0x19, 0xed, 0x82, 0xae, 0xed, 0x21,
		0x4c, 0x9d, 0x41, 0x45, 0x22, 0x8e, 0x84, 0xFE, 0x00, 0x00 };
#else
const uint8_t DT_REQ_CHAR3_UUID[2] = { 0x83, 0xFE };
#endif

#if (UUID_128_SUPPORTED == 1)
const uint8_t DT_REQ_CHAR_CONTROL_UUID[16] = { 0x19, 0xed, 0x82, 0xae, 0xed,
		0x21, 0x4c, 0x9d, 0x41, 0x45, 0x22, 0x8e, 0x85, 0xFE, 0x00, 0x00 };
#else
const uint8_t DT_REQ_CHAR3_UUID[2] = { 0x83, 0xFE };
#endif

#if (UUID_128_SUPPORTED == 1)
const uint8_t DT_REQ_CHAR_TIME_UUID[16] = { 0x19, 0xed, 0x82, 0xae, 0xed, 0x21,
		0x4c, 0x9d, 0x41, 0x45, 0x22, 0x8e, 0x86, 0xFE, 0x00, 0x00 };
#else
const uint8_t DT_REQ_CHAR3_UUID[2] = { 0x83, 0xFE };
#endif

#if (UUID_128_SUPPORTED == 1)
const uint8_t DT_REQ_SERV_UUID[16] = { 0x19, 0xed, 0x82, 0xae, 0xed, 0x21, 0x4c,
		0x9d, 0x41, 0x45, 0x22, 0x8e, 0x80, 0xFE, 0x00, 0x00 };
#else
const uint8_t DT_REQ_SERV_UUID[2] = { 0x80, 0xFE };
#endif

/* Private typedef -----------------------------------------------------------*/
typedef enum {
	DTS_STM_NOTIFICATION_MASK = (1 << 0), DTS_STM_INDICATION_MASK = (1 << 1),
} ClientCharConfMask_t;

typedef struct {
	uint16_t DataTransferSvcHdle; /**< Service handle */
	uint16_t DataTransferTxCharHdle; /**< Characteristic handle */
	uint16_t DataTransferRxCharHdle; /**< Characteristic handle */
	uint16_t DataTransferTxChar3Hdle; /**< Characteristic handle */
	uint16_t DataTransferRxCharLedHdle; /**< Characteristic handle */
	uint16_t DataTransferRxCharControlHdle; /**< Characteristic handle */
	uint16_t DataTransferRxCharTimeHdle;
} DataTransferSvcContext_t;

/* Private defines -----------------------------------------------------------*/
#define DATA_TRANSFER_NOTIFICATION_LEN_MAX                                 (255)

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static tBleStatus TX_Update_Char(DTS_STM_Payload_t *pDataValue);
static SVCCTL_EvtAckStatus_t DTS_Event_Handler(void *pckt);
static DataTransferSvcContext_t aDataTransferContext;
extern uint16_t Att_Mtu_Exchanged;
struct LogMessage receivedCntrlPacket;
union ColorComplex receivedColor;
time_t receivedEpoch;

/* Functions Definition ------------------------------------------------------*/
/* Private functions ----------------------------------------------------------*/

/**
 * @brief  Event handler
 * @param  Event: Address of the buffer holding the Event
 * @retval Ack: Return whether the Event has been managed or not
 */
static SVCCTL_EvtAckStatus_t DTS_Event_Handler(void *Event) {
	SVCCTL_EvtAckStatus_t return_value;
	hci_event_pckt *event_pckt;
	evt_blue_aci *blue_evt;
	aci_gatt_attribute_modified_event_rp0 *attribute_modified;
	aci_att_exchange_mtu_resp_event_rp0 *exchange_mtu_resp;
	aci_gatt_write_permit_req_event_rp0 *write_permit_req;

	DTS_STM_App_Notification_evt_t Notification;

	return_value = SVCCTL_EvtNotAck;
	event_pckt = (hci_event_pckt*) (((hci_uart_pckt*) Event)->data);

	switch (event_pckt->evt) {
	case EVT_VENDOR: {
		blue_evt = (evt_blue_aci*) event_pckt->data;

		switch (blue_evt->ecode) {
		case EVT_BLUE_ATT_EXCHANGE_MTU_RESP:
#ifdef NUCLEO_LED_ACTIVE
        	 BSP_LED_On(LED_BLUE);
#endif
			APP_DBG_MSG("EVT_BLUE_ATT_EXCHANGE_MTU_RESP \n");
			exchange_mtu_resp =
					(aci_att_exchange_mtu_resp_event_rp0*) blue_evt->data;
			APP_DBG_MSG("MTU_size = %d \n",exchange_mtu_resp->Server_RX_MTU );
			Att_Mtu_Exchanged = exchange_mtu_resp->Server_RX_MTU;
#ifdef NUCLEO_LED_ACTIVE
     	 BSP_LED_On(LED_GREEN);
#endif
			break;
			/* server */
		case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED: {
			attribute_modified =
					(aci_gatt_attribute_modified_event_rp0*) blue_evt->data;
			if (attribute_modified->Attr_Handle
					== (aDataTransferContext.DataTransferTxCharHdle + 2)) {
				/**
				 * Notify to application to start measurement
				 */
				if (attribute_modified->Attr_Data[0]
						& DTS_STM_NOTIFICATION_MASK) {
					APP_DBG_MSG("notification enabled\n");
					Notification.Evt_Opcode = DTS_STM__NOTIFICATION_ENABLED;
					DTS_Notification(&Notification);
				} else {
					APP_DBG_MSG("notification disabled\n");
					Notification.Evt_Opcode = DTS_STM_NOTIFICATION_DISABLED;
					DTS_Notification(&Notification);
				}
			}

			if (attribute_modified->Attr_Handle
					== (aDataTransferContext.DataTransferRxCharHdle + 2)) {
				return_value = SVCCTL_EvtAckFlowEnable;

				Notification.Evt_Opcode = DTS_STM_DATA_RECEIVED;
				Notification.DataTransfered.Length =
						attribute_modified->Attr_Data_Length;
				DTS_Notification(&Notification);
			}

			// if LED characteristic was modified
			if (attribute_modified->Attr_Handle
					== (aDataTransferContext.DataTransferRxCharLedHdle + 1)) {
#ifdef NUCLEO_LED_ACTIVE
        	  	  	  	  BSP_LED_Toggle(LED_BLUE);
#endif
				memcpy(&receivedColor, attribute_modified->Attr_Data,
						sizeof(receivedColor));
				//FrontLightsSet(&receivedColor);
				osMessageQueuePut(lightsComplexQueueHandle, &receivedColor, 0, 0);

			}

			// if system config was modified
			if (attribute_modified->Attr_Handle
					== (aDataTransferContext.DataTransferRxCharControlHdle + 1)) {
#ifdef NUCLEO_LED_ACTIVE
        	  BSP_LED_Toggle(LED_GREEN);
#endif
				memcpy(&receivedCntrlPacket, attribute_modified->Attr_Data,
						sizeof(struct LogMessage));

				osMessageQueuePut(togLoggingQueueHandle, &receivedCntrlPacket,
						0U, 0U);

			}

			// if epoch was set
			if (attribute_modified->Attr_Handle
					== (aDataTransferContext.DataTransferRxCharTimeHdle + 1)) {
#ifdef NUCLEO_LED_ACTIVE
						  BSP_LED_Toggle(LED_GREEN);
#endif
				memcpy(&receivedEpoch, attribute_modified->Attr_Data,
						sizeof(receivedEpoch));

				receivedEpoch = receivedEpoch / 1000;
				updateRTC(receivedEpoch);

			}

		}
			break;
		case EVT_BLUE_GATT_TX_POOL_AVAILABLE:
			Resume_Notification();
			break;

		case EVT_BLUE_GATT_WRITE_PERMIT_REQ:
			APP_DBG_MSG("write permit req\r\n");
			write_permit_req =
					(aci_gatt_write_permit_req_event_rp0*) blue_evt->data;
			aci_gatt_write_resp(write_permit_req->Connection_Handle,
					write_permit_req->Attribute_Handle, 0, 0,
					write_permit_req->Data_Length, write_permit_req->Data);
			break;

		default:
			break;
		}
	}
		break; /* HCI_EVT_VENDOR_SPECIFIC */

	default:
		break;
	}

	return (return_value);
}/* end SVCCTL_EvtAckStatus_t */

/**
 * @brief  Feature Characteristic update
 * @param  Service_Instance: Instance of the service to which the characteristic belongs
 * @param  pFeatureValue: The address of the new value to be written
 * @retval None
 */
static tBleStatus TX_Update_Char(DTS_STM_Payload_t *pDataValue) {
	tBleStatus ret;

	/**
	 *  Notification Data Transfer Packet
	 */
	ret = aci_gatt_update_char_value(aDataTransferContext.DataTransferSvcHdle,
			aDataTransferContext.DataTransferTxCharHdle, 0, /* charValOffset */
			pDataValue->Length, /* charValueLen */
			(uint8_t*) pDataValue->pPayload);

	return ret;
}/* end TX_Update_Char() */

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Service initialization
 * @param  None
 * @retval None
 */
void DTS_STM_Init(void) {
	tBleStatus hciCmdResult = BLE_STATUS_FAILED;

	/**
	 *	Register the event handler to the BLE controller
	 */
	SVCCTL_RegisterSvcHandler(DTS_Event_Handler);

	/* DT service and characteristics */
	hciCmdResult = aci_gatt_add_service(DT_UUID_LENGTH,
			(Service_UUID_t*) DT_REQ_SERV_UUID,
			PRIMARY_SERVICE, 1 + 3 * NUM_OF_CHARACTERISTICS,
			&(aDataTransferContext.DataTransferSvcHdle));
	if (hciCmdResult != 0) {
		APP_DBG_MSG("error add service 0x%x\n", hciCmdResult);
#ifdef NUCLEO_LED_ACTIVE
    BSP_LED_On(LED_RED);
#endif
	}

	/**
	 *  Add Data Transfer TX Characteristic (characteristic that is used to send data)
	 */
	hciCmdResult = aci_gatt_add_char(aDataTransferContext.DataTransferSvcHdle,
	DT_UUID_LENGTH, (Char_UUID_t*) DT_REQ_CHAR_UUID,
	DATA_TRANSFER_NOTIFICATION_LEN_MAX,
	CHAR_PROP_NOTIFY,
	ATTR_PERMISSION_NONE,
	GATT_DONT_NOTIFY_EVENTS, /* gattEvtMask */
	10, /* encryKeySize */
	1, /* isVariable */
	&(aDataTransferContext.DataTransferTxCharHdle));
	if (hciCmdResult != 0) {
		APP_DBG_MSG("error add char Tx 0x%x\n", hciCmdResult);
#ifdef NUCLEO_LED_ACTIVE
    BSP_LED_On(LED_RED);
#endif
	}

	/**
	 *  Add Data Transfer RX Characteristic (not intended to be used in the end)
	 */
	hciCmdResult = aci_gatt_add_char(aDataTransferContext.DataTransferSvcHdle,
	DT_UUID_LENGTH, (Char_UUID_t*) DT_REQ_CHAR2_UUID, 255, /* DATA_TRANSFER_NOTIFICATION_LEN_MAX, */
	CHAR_PROP_WRITE_WITHOUT_RESP,
	ATTR_PERMISSION_NONE,
	GATT_NOTIFY_ATTRIBUTE_WRITE, //GATT_NOTIFY_WRITE_REQ_AND_WAIT_FOR_APPL_RESP, /* gattEvtMask */
			10, /* encryKeySize */
			1, /* isVariable */
			&(aDataTransferContext.DataTransferRxCharHdle));
	if (hciCmdResult != 0) {
		APP_DBG_MSG("error add char Tx\n");

#ifdef NUCLEO_LED_ACTIVE
    BSP_LED_On(LED_RED);
#endif
	}
//
//  /**
//   *  Add Data Transfer TX Characteristic (not intended to be used in the end)
//   */
//  hciCmdResult = aci_gatt_add_char(aDataTransferContext.DataTransferSvcHdle,
//  DT_UUID_LENGTH,
//                    (Char_UUID_t *) DT_REQ_CHAR3_UUID,
//                    255, /* DATA_TRANSFER_NOTIFICATION_LEN_MAX, */
//                    CHAR_PROP_NOTIFY,
//                    ATTR_PERMISSION_NONE,
//                    GATT_DONT_NOTIFY_EVENTS, /* gattEvtMask */
//                    10, /* encryKeySize */
//                    1, /* isVariable */
//                    &(aDataTransferContext.DataTransferTxChar3Hdle));
//  if (hciCmdResult != 0)
//  {
//    APP_DBG_MSG("error add char Tx\n");
//#ifdef NUCLEO_LED_ACTIVE
//    BSP_LED_On(LED_RED);
//#endif
//  }

	/**
	 *  Add Data LED Control Characteristic
	 */
	hciCmdResult = aci_gatt_add_char(aDataTransferContext.DataTransferSvcHdle,
	DT_UUID_LENGTH, (Char_UUID_t*) DT_REQ_CHAR_LED_UUID, 100, /* DATA_TRANSFER_NOTIFICATION_LEN_MAX, */
	CHAR_PROP_WRITE_WITHOUT_RESP,
	ATTR_PERMISSION_NONE,
	GATT_NOTIFY_ATTRIBUTE_WRITE, //GATT_NOTIFY_WRITE_REQ_AND_WAIT_FOR_APPL_RESP, /* gattEvtMask */
			10, /* encryKeySize */
			1, /* isVariable */
			&(aDataTransferContext.DataTransferRxCharLedHdle));
	if (hciCmdResult != 0) {
		APP_DBG_MSG("error add char Tx\n");
#ifdef NUCLEO_LED_ACTIVE
    BSP_LED_On(LED_RED);
#endif
	}

	/**
	 *  Add System Control Characteristic
	 */
	hciCmdResult = aci_gatt_add_char(aDataTransferContext.DataTransferSvcHdle,
	DT_UUID_LENGTH, (Char_UUID_t*) DT_REQ_CHAR_CONTROL_UUID, 100, /* DATA_TRANSFER_NOTIFICATION_LEN_MAX, */
	CHAR_PROP_WRITE_WITHOUT_RESP,
	ATTR_PERMISSION_NONE,
	GATT_NOTIFY_ATTRIBUTE_WRITE, //GATT_NOTIFY_WRITE_REQ_AND_WAIT_FOR_APPL_RESP, /* gattEvtMask */
			10, /* encryKeySize */
			1, /* isVariable */
			&(aDataTransferContext.DataTransferRxCharControlHdle));
	if (hciCmdResult != 0) {
		APP_DBG_MSG("error add char Tx\n");
#ifdef NUCLEO_LED_ACTIVE
      BSP_LED_On(LED_RED);
  #endif
	}

	/**
	 *  Add System Time Characteristic
	 */
	hciCmdResult = aci_gatt_add_char(aDataTransferContext.DataTransferSvcHdle,
	DT_UUID_LENGTH, (Char_UUID_t*) DT_REQ_CHAR_TIME_UUID, 10, /* DATA_TRANSFER_NOTIFICATION_LEN_MAX, */
	CHAR_PROP_WRITE_WITHOUT_RESP,
	ATTR_PERMISSION_NONE,
	GATT_NOTIFY_ATTRIBUTE_WRITE, //GATT_NOTIFY_WRITE_REQ_AND_WAIT_FOR_APPL_RESP, /* gattEvtMask */
			10, /* encryKeySize */
			1, /* isVariable */
			&(aDataTransferContext.DataTransferRxCharTimeHdle));
	if (hciCmdResult != 0) {
		APP_DBG_MSG("error add char Tx\n");
#ifdef NUCLEO_LED_ACTIVE
      BSP_LED_On(LED_RED);
  #endif
	}

	return;
}

/**
 * @brief  Characteristic update
 * @param  UUID: UUID of the characteristic
 * @param  Service_Instance: Instance of the service to which the characteristic belongs
 * 
 */
tBleStatus DTS_STM_UpdateChar(uint16_t UUID, uint8_t *pPayload) {
	tBleStatus result = BLE_STATUS_INVALID_PARAMS;
	switch (UUID) {
	case DATA_TRANSFER_TX_CHAR_UUID:
		result = TX_Update_Char((DTS_STM_Payload_t*) pPayload);
		break;

	default:
		break;
	}
	return result;
}/* end DTS_STM_UpdateChar() */

tBleStatus DTS_STM_UpdateCharThroughput(DTS_STM_Payload_t *pDataValue) {
	tBleStatus result = BLE_STATUS_INVALID_PARAMS;
	/**
	 *  Notification Data Transfer Packet
	 */
	result = aci_gatt_update_char_value(
			aDataTransferContext.DataTransferSvcHdle,
			aDataTransferContext.DataTransferTxChar3Hdle, 0, /* charValOffset */
			pDataValue->Length, /* charValueLen */
			(uint8_t*) pDataValue->pPayload);
	return result;
}/* end DTS_STM_UpdateChar() */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
