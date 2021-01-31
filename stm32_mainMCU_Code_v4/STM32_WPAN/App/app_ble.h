/* USER CODE BEGIN Header */
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_BLE_H
#define APP_BLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hci_tl.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_conf.h"
#include "cmsis_os.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/

    typedef enum
    {
      APP_BLE_IDLE,
      APP_BLE_FAST_ADV,
      APP_BLE_LP_ADV,
      APP_BLE_SCAN,
      APP_BLE_LP_CONNECTING,
      APP_BLE_CONNECTED_SERVER,
      APP_BLE_CONNECTED_CLIENT
    } APP_BLE_ConnStatus_t;
    
/* USER CODE BEGIN ET */
    /**
     * security parameters structure
     */
    typedef struct _tSecurityParams
    {
      /**
       * IO capability of the device
       */
      uint8_t ioCapability;

      /**
       * Authentication requirement of the device
       * Man In the Middle protection required?
       */
      uint8_t mitm_mode;

      /**
       * bonding mode of the device
       */
      uint8_t bonding_mode;

      /**
       * Flag to tell whether OOB data has
       * to be used during the pairing process
       */
      uint8_t OOB_Data_Present;

      /**
       * OOB data to be used in the pairing process if
       * OOB_Data_Present is set to TRUE
       */
      uint8_t OOB_Data[16];

      /**
       * this variable indicates whether to use a fixed pin
       * during the pairing process or a passkey has to be
       * requested to the application during the pairing process
       * 0 implies use fixed pin and 1 implies request for passkey
       */
      uint8_t Use_Fixed_Pin;

      /**
       * minimum encryption key size requirement
       */
      uint8_t encryptionKeySizeMin;

      /**
       * maximum encryption key size requirement
       */
      uint8_t encryptionKeySizeMax;

      /**
       * fixed pin to be used in the pairing process if
       * Use_Fixed_Pin is set to 1
       */
      uint32_t Fixed_Pin;

      /**
       * this flag indicates whether the host has to initiate
       * the security, wait for pairing or does not have any security
       * requirements.\n
       * 0x00 : no security required
       * 0x01 : host should initiate security by sending the slave security
       *        request command
       * 0x02 : host need not send the clave security request but it
       * has to wait for paiirng to complete before doing any other
       * processing
       */
      uint8_t initiateSecurity;
    }tSecurityParams;

    /**
     * global context
     * contains the variables common to all
     * services
     */
    typedef struct _tBLEProfileGlobalContext
    {

      /**
       * security requirements of the host
       */
      tSecurityParams bleSecurityParam;

      /**
       * gap service handle
       */
      uint16_t gapServiceHandle;

      /**
       * device name characteristic handle
       */
      uint16_t devNameCharHandle;

      /**
       * appearance characteristic handle
       */
      uint16_t appearanceCharHandle;

      /**
       * connection handle of the current active connection
       * When not in connection, the handle is set to 0xFFFF
       */
      uint16_t connectionHandle;

      /**
       * length of the UUID list to be used while advertising
       */
      uint8_t advtServUUIDlen;

      /**
       * the UUID list to be used while advertising
       */
      uint8_t advtServUUID[100];

    }BleGlobalContext_t;

    typedef struct
    {
      BleGlobalContext_t BleApplicationContext_legacy;
       APP_BLE_ConnStatus_t Device_Connection_Status;
       /**
       * ID of the Advertising Timeout
       */
       uint8_t Advertising_mgr_timer_Id;

      uint8_t SwitchOffGPIO_timer_Id;

      uint8_t DeviceServerFound;
    }BleApplicationContext_t;
/* USER CODE END ET */  

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */
extern PLACE_IN_SECTION("BLE_APP_CONTEXT") BleApplicationContext_t BleApplicationContext;
extern osThreadId_t LinkConfigProcessId;

/* USER CODE END EV */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions ---------------------------------------------*/
#ifndef DYNAMIC_MODE
  void APP_BLE_Init( void ); 
#else
  void APP_BLE_Init_Dyn_1( void );
  void APP_BLE_Init_Dyn_2( void );
#endif
  APP_BLE_ConnStatus_t APP_BLE_Get_Server_Connection_Status(void);

/* USER CODE BEGIN EF */
  void BLE_SVC_L2CAP_Conn_Update_7_5(void);

void APP_BLE_Key_Button1_Action(void);
void APP_BLE_Key_Button2_Action(void);
void APP_BLE_Key_Button3_Action(void);
void APP_BLE_Stop(void);
uint8_t APP_BLE_ComputeCRC8( uint8_t *DataPtr , uint8_t Datalen );

#if(L2CAP_REQUEST_NEW_CONN_PARAM != 0)
void BLE_SVC_L2CAP_Conn_Update(uint16_t Connection_Handle);
#endif
/* USER CODE END EF */

#ifdef __cplusplus
}
#endif

#endif /*APP_BLE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
