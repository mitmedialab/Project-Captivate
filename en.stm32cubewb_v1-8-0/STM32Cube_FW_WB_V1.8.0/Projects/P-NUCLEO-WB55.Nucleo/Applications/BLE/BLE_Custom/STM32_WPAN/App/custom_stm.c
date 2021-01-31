/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : App/custom_stm.c
  * Description        : Custom Example Service.
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#include "common_blesvc.h"
#include "custom_stm.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef struct{
  uint16_t  CustomMy_P2PsHdle;                   /**< My_P2P_Server handle */
  uint16_t  CustomLed_CHdle;                   /**< My_LED_Char handle */
  uint16_t  CustomSwitch_CHdle;                   /**< My_Switch_Char handle */
  uint16_t  CustomMy_HrsHdle;                   /**< My_Heart_Rate handle */
  uint16_t  CustomHrs_MHdle;                   /**< My_HRS_Meas handle */
  uint16_t  CustomHrs_SlHdle;                   /**< My_Sensor_Loc handle */
  uint16_t  CustomHrs_CtrlpHdle;                   /**< My_HRS_CTRL_Point handle */
}CustomContext_t;

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
#define UUID_128_SUPPORTED  1

#if (UUID_128_SUPPORTED == 1)
#define BM_UUID_LENGTH  UUID_TYPE_128
#else
#define BM_UUID_LENGTH  UUID_TYPE_16
#endif

#define BM_REQ_CHAR_SIZE    (3)

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define CHARACTERISTIC_VALUE_ATTRIBUTE_OFFSET              1
#define CHARACTERISTIC_DESCRIPTOR_ATTRIBUTE_OFFSET         2

#define CUSTOM_STM_HRS_CNTL_POINT_RESET_ENERGY_EXPENDED      (0x01)
#define CUSTOM_STM_HRS_CNTL_POINT_VALUE_IS_SUPPORTED         (0x00)
#define CUSTOM_STM_HRS_CNTL_POINT_VALUE_NOT_SUPPORTED        (0x80)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static const uint8_t SizeLed_C=2;
static const uint8_t SizeSwitch_C=2;
static const uint8_t SizeHrs_M=5;
static const uint8_t SizeHrs_Sl=1;
static const uint8_t SizeHrs_Ctrlp=1;
/**
 * START of Section BLE_DRIVER_CONTEXT
 */
PLACE_IN_SECTION("BLE_DRIVER_CONTEXT") static CustomContext_t CustomContext;

/**
 * END of Section BLE_DRIVER_CONTEXT
 */

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static SVCCTL_EvtAckStatus_t Custom_STM_Event_Handler(void *pckt);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
/* USER CODE BEGIN PFD */

/* USER CODE END PFD */

/* Private functions ----------------------------------------------------------*/

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
        uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
            uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
                uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

/* Hardware Characteristics Service */
/*
 The following 128bits UUIDs have been generated from the random UUID
 generator:
 D973F2E0-B19E-11E2-9E96-0800200C9A66: Service 128bits UUID
 D973F2E1-B19E-11E2-9E96-0800200C9A66: Characteristic_1 128bits UUID
 D973F2E2-B19E-11E2-9E96-0800200C9A66: Characteristic_2 128bits UUID
 */
#define COPY_MY_P2P_SERVER_UUID(uuid_struct)          COPY_UUID_128(uuid_struct,0x00,0x00,0xfe,0x40,0xcc,0x7a,0x48,0x2a,0x98,0x4a,0x7f,0x2e,0xd5,0xb3,0xe5,0x8f)
#define COPY_MY_LED_CHAR_UUID(uuid_struct)    COPY_UUID_128(uuid_struct,0x00,0x00,0xfe,0x41,0x8e,0x22,0x45,0x41,0x9d,0x4c,0x21,0xed,0xae,0x82,0xed,0x19)
#define COPY_MY_SWITCH_CHAR_UUID(uuid_struct)    COPY_UUID_128(uuid_struct,0x00,0x00,0xfe,0x42,0x8e,0x22,0x45,0x41,0x9d,0x4c,0x21,0xed,0xae,0x82,0xed,0x19)

/* USER CODE BEGIN PF */

/* USER CODE END PF */

/**
 * @brief  Event handler
 * @param  Event: Address of the buffer holding the Event
 * @retval Ack: Return whether the Event has been managed or not
 */
static SVCCTL_EvtAckStatus_t Custom_STM_Event_Handler(void *Event)
{
  SVCCTL_EvtAckStatus_t return_value;
  hci_event_pckt *event_pckt;
  evt_blue_aci *blue_evt;
/* USER CODE BEGIN Custom_STM_Event_Handler_1 */
  aci_gatt_attribute_modified_event_rp0 *attribute_modified;
  /* read_req is useful if Characteristic property = CHAR_PROP_READ 
                           Gatt Event Mask = GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP are defined, so:
                           BLE core event EVT_BLUE_GATT_READ_PERMIT_REQ must be considered*/
  /* aci_gatt_read_permit_req_event_rp0    *read_req; */ 
  Custom_STM_App_Notification_evt_t     Notification;
  aci_gatt_write_permit_req_event_rp0   *write_perm_req; 
/* USER CODE END Custom_STM_Event_Handler_1 */

  return_value = SVCCTL_EvtNotAck;
  event_pckt = (hci_event_pckt *)(((hci_uart_pckt*)Event)->data);

  switch(event_pckt->evt)
  {
    case EVT_VENDOR:
      blue_evt = (evt_blue_aci*)event_pckt->data;
      switch(blue_evt->ecode)
      {

        case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
          /* USER CODE BEGIN EVT_BLUE_GATT_ATTRIBUTE_MODIFIED */
          /**
          *  Manage My_HRS_Meas Characteristic, Notify descriptor
          */
          attribute_modified = (aci_gatt_attribute_modified_event_rp0*)blue_evt->data;
          if(attribute_modified->Attr_Handle == (CustomContext.CustomHrs_MHdle + CHARACTERISTIC_DESCRIPTOR_ATTRIBUTE_OFFSET))
          {
            return_value = SVCCTL_EvtAckFlowEnable;

            /**
             * Notify the application to start measurement
             */
            if(attribute_modified->Attr_Data[0] & COMSVC_Notification)
            {
              /**
               * Notify the application that My_HRS_Meas notifications have been enabled
               */
              Notification.Custom_Evt_Opcode = CUSTOM_STM_HRS_M_NOTIFY_ENABLED_EVT;
              Custom_STM_App_Notification(&Notification);
            }
            else
            {
              /**
               * Notify the application that My_HRS_Meas notifications have been disabled
               */
              Notification.Custom_Evt_Opcode = CUSTOM_STM_HRS_M_NOTIFY_DISABLED_EVT;
              Custom_STM_App_Notification(&Notification);
            }
          }

          /**
          *  Manage My_Switch_Char Characteristic, Notify descriptor
          */
          else if(attribute_modified->Attr_Handle == (CustomContext.CustomSwitch_CHdle + CHARACTERISTIC_DESCRIPTOR_ATTRIBUTE_OFFSET))
            {
              /**
               * Descriptor handle
               */
              return_value = SVCCTL_EvtAckFlowEnable;
              /**
               * Notify to application
               */
              if(attribute_modified->Attr_Data[0] & COMSVC_Notification)
              {
              /**
               * Notify the application that My_Switch_Char notications have been enabled
               */
                Notification.Custom_Evt_Opcode = CUSTOM_STM_SWITCH_C_NOTIFY_ENABLED_EVT;
                Custom_STM_App_Notification(&Notification);
              }
              else
              {
              /**
               * Notify the application that My_Switch_Char notications have been disabled
               */
                Notification.Custom_Evt_Opcode = CUSTOM_STM_SWITCH_C_NOTIFY_DISABLED_EVT;
                Custom_STM_App_Notification(&Notification);
              }
            }

          /**
          *  Manage My_LED_Char Characteristic Write
          */
          else if(attribute_modified->Attr_Handle == (CustomContext.CustomLed_CHdle + CHARACTERISTIC_VALUE_ATTRIBUTE_OFFSET))
            {
              Notification.Custom_Evt_Opcode = CUSTOM_STM_LED_C_WRITE_NO_RESP_EVT;
              Notification.DataTransfered.Length=attribute_modified->Attr_Data_Length;
              Notification.DataTransfered.pPayload=attribute_modified->Attr_Data;
              Custom_STM_App_Notification(&Notification);  
            }
          /* USER CODE END EVT_BLUE_GATT_ATTRIBUTE_MODIFIED */
          break;
        case EVT_BLUE_GATT_READ_PERMIT_REQ :
          /* USER CODE BEGIN EVT_BLUE_GATT_READ_PERMIT_REQ */

          /* USER CODE END EVT_BLUE_GATT_READ_PERMIT_REQ */
          break;
        case EVT_BLUE_GATT_WRITE_PERMIT_REQ:
          /* USER CODE BEGIN EVT_BLUE_GATT_WRITE_PERMIT_REQ */
          /**
          *  Manage My_HRS_CTRL_Point Characteristic Write
          *  Configure Heart Rate CtrlPoint Characteristic to enable a Client to write control points to a Server to control behavior.
          *  Support for this characteristic is mandatory if the Server supports the Energy Expended feature. 
          */
          
          write_perm_req = (aci_gatt_write_permit_req_event_rp0*)blue_evt->data;

          if(write_perm_req->Attribute_Handle == (CustomContext.CustomHrs_CtrlpHdle + CHARACTERISTIC_VALUE_ATTRIBUTE_OFFSET))
          {
            return_value = SVCCTL_EvtAckFlowEnable;

            if (write_perm_req->Data[0] == CUSTOM_STM_HRS_CNTL_POINT_RESET_ENERGY_EXPENDED)
            {
              /* received a correct value for My_HRS_CTRL_Point char */
              aci_gatt_write_resp(write_perm_req->Connection_Handle,
                                      write_perm_req->Attribute_Handle,
                                      0x00,                                                     /* write_status = 0 (no error))*/
                                      (uint8_t)CUSTOM_STM_HRS_CNTL_POINT_VALUE_IS_SUPPORTED,    /* err_code */
                                      write_perm_req->Data_Length,
                                      (uint8_t *)&write_perm_req->Data[0]);

              /**
               * Notify the application to Reset The Energy Expended Value
               */
              Notification.Custom_Evt_Opcode = CUSTOM_STM_HRS_CTRLP_WRITE_EVT;
              Custom_STM_App_Notification(&Notification);
            }
            else
            {
              /* received value of HRM control point char is incorrect */
              aci_gatt_write_resp(write_perm_req->Connection_Handle,
                                      write_perm_req->Attribute_Handle,
                                      0x1,                                                      /* write_status = 1 (error))*/
                                      (uint8_t)CUSTOM_STM_HRS_CNTL_POINT_VALUE_NOT_SUPPORTED,   /* err_code */
                                      write_perm_req->Data_Length,
                                      (uint8_t *)&write_perm_req->Data[0]);
            }
          }
          /* USER CODE END EVT_BLUE_GATT_WRITE_PERMIT_REQ */
          break;

        default:
          /* USER CODE BEGIN EVT_DEFAULT */

          /* USER CODE END EVT_DEFAULT */
          break;
      }
      /* USER CODE BEGIN EVT_VENDOR*/

      /* USER CODE END EVT_VENDOR*/
      break; /* EVT_VENDOR */

    /* USER CODE BEGIN EVENT_PCKT_CASES*/

    /* USER CODE END EVENT_PCKT_CASES*/

    default:
      break;
  }

/* USER CODE BEGIN Custom_STM_Event_Handler_2 */

/* USER CODE END Custom_STM_Event_Handler_2 */

  return(return_value);
}/* end Custom_STM_Event_Handler */

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Service initialization
 * @param  None
 * @retval None
 */
void SVCCTL_InitCustomSvc(void)
{

  Char_UUID_t  uuid;
/* USER CODE BEGIN SVCCTL_InitCustomSvc_1 */

/* USER CODE END SVCCTL_InitCustomSvc_1 */

  /**
   *	Register the event handler to the BLE controller
   */
  SVCCTL_RegisterSvcHandler(Custom_STM_Event_Handler);

    /*
     *          My_P2P_Server
     *
     * Max_Attribute_Records = 1 + 2*2 + 1*no_of_char_with_notify_or_indicate_property
     * service_max_attribute_record = 1 for My_P2P_Server +
     *                                2 for My_LED_Char +
     *                                2 for My_Switch_Char +
     *                                1 for My_Switch_Char configuration descriptor +
     *                              = 6
     */

    COPY_MY_P2P_SERVER_UUID(uuid.Char_UUID_128);
    aci_gatt_add_service(UUID_TYPE_128,
                      (Service_UUID_t *) &uuid,
                      PRIMARY_SERVICE,
                      6,
                      &(CustomContext.CustomMy_P2PsHdle));

    /**
     *  My_LED_Char
     */
    COPY_MY_LED_CHAR_UUID(uuid.Char_UUID_128);
    aci_gatt_add_char(CustomContext.CustomMy_P2PsHdle,
                      UUID_TYPE_128, &uuid,
                      SizeLed_C,
                      CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RESP,
                      ATTR_PERMISSION_NONE,
                      GATT_NOTIFY_ATTRIBUTE_WRITE,
                      0x10,
                      CHAR_VALUE_LEN_VARIABLE,
                      &(CustomContext.CustomLed_CHdle));
    /**
     *  My_Switch_Char
     */
    COPY_MY_SWITCH_CHAR_UUID(uuid.Char_UUID_128);
    aci_gatt_add_char(CustomContext.CustomMy_P2PsHdle,
                      UUID_TYPE_128, &uuid,
                      SizeSwitch_C,
                      CHAR_PROP_NOTIFY,
                      ATTR_PERMISSION_NONE,
                      GATT_NOTIFY_ATTRIBUTE_WRITE,
                      0x10,
                      CHAR_VALUE_LEN_VARIABLE,
                      &(CustomContext.CustomSwitch_CHdle));

    /*
     *          My_Heart_Rate
     *
     * Max_Attribute_Records = 1 + 2*3 + 1*no_of_char_with_notify_or_indicate_property
     * service_max_attribute_record = 1 for My_Heart_Rate +
     *                                2 for My_HRS_Meas +
     *                                2 for My_Sensor_Loc +
     *                                2 for My_HRS_CTRL_Point +
     *                                1 for My_HRS_Meas configuration descriptor +
     *                              = 8
     */

    uuid.Char_UUID_16 = 0x180d;
    aci_gatt_add_service(UUID_TYPE_16,
                      (Service_UUID_t *) &uuid,
                      PRIMARY_SERVICE,
                      8,
                      &(CustomContext.CustomMy_HrsHdle));

    /**
     *  My_HRS_Meas
     */
    uuid.Char_UUID_16 = 0x2a37;
    aci_gatt_add_char(CustomContext.CustomMy_HrsHdle,
                      UUID_TYPE_16, &uuid,
                      SizeHrs_M,
                      CHAR_PROP_NOTIFY,
                      ATTR_PERMISSION_NONE,
                      GATT_DONT_NOTIFY_EVENTS,
                      0x10,
                      CHAR_VALUE_LEN_VARIABLE,
                      &(CustomContext.CustomHrs_MHdle));
    /**
     *  My_Sensor_Loc
     */
    uuid.Char_UUID_16 = 0x2a38;
    aci_gatt_add_char(CustomContext.CustomMy_HrsHdle,
                      UUID_TYPE_16, &uuid,
                      SizeHrs_Sl,
                      CHAR_PROP_READ,
                      ATTR_PERMISSION_NONE,
                      GATT_DONT_NOTIFY_EVENTS,
                      0x10,
                      CHAR_VALUE_LEN_CONSTANT,
                      &(CustomContext.CustomHrs_SlHdle));
    /**
     *  My_HRS_CTRL_Point
     */
    uuid.Char_UUID_16 = 0x2a39;
    aci_gatt_add_char(CustomContext.CustomMy_HrsHdle,
                      UUID_TYPE_16, &uuid,
                      SizeHrs_Ctrlp,
                      CHAR_PROP_WRITE,
                      ATTR_PERMISSION_NONE,
                      GATT_NOTIFY_WRITE_REQ_AND_WAIT_FOR_APPL_RESP,
                      0x10,
                      CHAR_VALUE_LEN_CONSTANT,
                      &(CustomContext.CustomHrs_CtrlpHdle));

/* USER CODE BEGIN SVCCTL_InitCustomSvc_2 */

/* USER CODE END SVCCTL_InitCustomSvc_2 */

  return;
}

/**
 * @brief  Characteristic update
 * @param  CharOpcode: Characteristic identifier
 * @param  Service_Instance: Instance of the service to which the characteristic belongs
 *
 */
tBleStatus Custom_STM_App_Update_Char(Custom_STM_Char_Opcode_t CharOpcode, uint8_t *pPayload)
{
  tBleStatus result = BLE_STATUS_INVALID_PARAMS;
/* USER CODE BEGIN Custom_STM_App_Update_Char_1 */

/* USER CODE END Custom_STM_App_Update_Char_1 */

  switch(CharOpcode)
  {

    case CUSTOM_STM_LED_C:
      result = aci_gatt_update_char_value(CustomContext.CustomMy_P2PsHdle,
                            CustomContext.CustomLed_CHdle,
                            0, /* charValOffset */
                            SizeLed_C, /* charValueLen */
                            (uint8_t *)  pPayload);
    /* USER CODE BEGIN CUSTOM_STM_LED_C*/

    /* USER CODE END CUSTOM_STM_LED_C*/
      break;

    case CUSTOM_STM_SWITCH_C:
      result = aci_gatt_update_char_value(CustomContext.CustomMy_P2PsHdle,
                            CustomContext.CustomSwitch_CHdle,
                            0, /* charValOffset */
                            SizeSwitch_C, /* charValueLen */
                            (uint8_t *)  pPayload);
    /* USER CODE BEGIN CUSTOM_STM_SWITCH_C*/

    /* USER CODE END CUSTOM_STM_SWITCH_C*/
      break;

    case CUSTOM_STM_HRS_M:
      result = aci_gatt_update_char_value(CustomContext.CustomMy_HrsHdle,
                            CustomContext.CustomHrs_MHdle,
                            0, /* charValOffset */
                            SizeHrs_M, /* charValueLen */
                            (uint8_t *)  pPayload);
    /* USER CODE BEGIN CUSTOM_STM_HRS_M*/

    /* USER CODE END CUSTOM_STM_HRS_M*/
      break;

    case CUSTOM_STM_HRS_SL:
      result = aci_gatt_update_char_value(CustomContext.CustomMy_HrsHdle,
                            CustomContext.CustomHrs_SlHdle,
                            0, /* charValOffset */
                            SizeHrs_Sl, /* charValueLen */
                            (uint8_t *)  pPayload);
    /* USER CODE BEGIN CUSTOM_STM_HRS_SL*/

    /* USER CODE END CUSTOM_STM_HRS_SL*/
      break;

    case CUSTOM_STM_HRS_CTRLP:
      result = aci_gatt_update_char_value(CustomContext.CustomMy_HrsHdle,
                            CustomContext.CustomHrs_CtrlpHdle,
                            0, /* charValOffset */
                            SizeHrs_Ctrlp, /* charValueLen */
                            (uint8_t *)  pPayload);
    /* USER CODE BEGIN CUSTOM_STM_HRS_CTRLP*/

    /* USER CODE END CUSTOM_STM_HRS_CTRLP*/
      break;

    default:
      break;
  }

/* USER CODE BEGIN Custom_STM_App_Update_Char_2 */

/* USER CODE END Custom_STM_App_Update_Char_2 */

  return result;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
