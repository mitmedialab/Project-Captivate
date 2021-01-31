
/**
  ******************************************************************************
  * @file    app_ble.h
  * @author  MCD Application Team
  * @brief   Header for ble application
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_BLE_H
#define __APP_BLE_H

#ifdef __cplusplus
extern "C" {
#endif

  /* Includes ------------------------------------------------------------------*/
#include "hci_tl.h"
  
  /* Exported types ------------------------------------------------------------*/
  typedef enum
  {
  APP_BLE_IDLE,
  APP_BLE_FAST_ADV,
  APP_BLE_LP_ADV,
  APP_BLE_SCAN,
  APP_BLE_CONNECTING,
  APP_BLE_CONNECTED,
  
  APP_BLE_DISCOVER_SERVICES,
  APP_BLE_DISCOVER_CHARACS,
  APP_BLE_DISCOVER_LED_CHAR_DESC,
  APP_BLE_DISCOVER_BUTTON_CHAR_DESC,
  APP_BLE_DISCOVER_NOTIFICATION_CHAR_DESC,
  APP_BLE_ENABLE_NOTIFICATION_BUTTON_DESC,
  APP_BLE_DISABLE_NOTIFICATION_TX_DESC      
} APP_BLE_ConnStatus_t;  

  
typedef enum
{
  P2P_START_TIMER_EVT,
  P2P_STOP_TIMER_EVT,
  P2P_BUTTON_INFO_RECEIVED_EVT,
} P2P_Client_Opcode_Notification_evt_t;

typedef struct
{
  uint8_t * pPayload;
  uint8_t     Length;
}P2P_Client_Data_t;  

typedef struct
{
  P2P_Client_Opcode_Notification_evt_t  P2P_Client_Evt_Opcode;
  P2P_Client_Data_t DataTransfered;
  uint8_t   ServiceInstance;
}P2P_Client_App_Notification_evt_t;
  /* Exported constants --------------------------------------------------------*/
  /* External variables --------------------------------------------------------*/
  /* Exported macros -----------------------------------------------------------*/
  /* Exported functions ------------------------------------------------------- */
  void APP_BLE_Init( void );
    
  APP_BLE_ConnStatus_t APP_BLE_Get_Client_Connection_Status( uint16_t Connection_Handle );
  void APP_BLE_Key_Button1_Action( void );

#ifdef __cplusplus
}
#endif

#endif /*__APP_BLE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
