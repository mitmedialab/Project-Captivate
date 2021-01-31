/**
  ******************************************************************************
  * @file    appli_sensor.c
  * @author  BLE Mesh Team
  * @brief   Application interface for Sensor Mesh Models 
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

#include "hal_common.h"
#include "types.h"
#include "sensors.h"
#include "light_lc.h"
#include "appli_sensor.h"
#include "mesh_cfg.h"
//#include "LPS25HB.h"
#include "string.h"
#include "common.h"
//#include "math.h"

/** @addtogroup ST_BLE_Mesh
 *  @{
 */

/** @addtogroup Application_Mesh_Models
 *  @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#if 0
/**
* @brief  PRESSURE init structure definition
*/
PRESSURE_InitTypeDef InitStructure =
{
  LPS25HB_ODR_1Hz,
  LPS25HB_BDU_READ, 
  LPS25HB_DIFF_ENABLE,  
  LPS25HB_SPI_SIM_3W,  
  LPS25HB_P_RES_AVG_32,
  LPS25HB_T_RES_AVG_16 
};
#endif

/* Application variables of sensor model definition */
#ifdef ENABLE_SENSOR_MODEL_SERVER

Appli_Sensor_DescriptorStatus_t Appli_Sensor_DescriptorStatus;
Appli_Sensor_SettingSet_t Appli_Sensor_SettingSet[3];

/* By Default value used for cadence set for testing. */
Sensor_CadenceSet_t Sensor_CadenceSet[NUMBER_OF_SENSOR];

/*
{
  {0x0071 , 0x2 , 2 , 2 ,2 ,1 ,0X05 , 0x64},
  {0x2A6D , 0x2 , 1 , 1 , 1, 1, 0X258 , 0x3ED},
  {0X2A7F ,0x2,1,1,1,0,0x10, 0x20}
};*/


#endif

MODEL_Property_IDTableParam_t Property_ID_Table[NUMBER_OF_SENSOR] = 
{
  {TEMPERATURE_PID},
  {PRESSURE_PID},
  {TIME_OF_FLIGHT_PID}
}; 
MOBLEUINT8 Occupancy_Flag = MOBLE_FALSE;
extern MOBLEUINT8 NumberOfElements;
extern MOBLEUINT8 ProvisionFlag;
MOBLEUINT8 Sensor_Setting_Access  = 0x01 ;



/* Temperature and Pressure init structure*/ 
#if 0
 PRESSURE_DrvTypeDef* xLPS25HBDrv = &LPS25HBDrv; 
#endif

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
#ifdef ENABLE_SENSOR_MODEL_SERVER
/******************************************************************************/


/**
* @brief  Appli_Sensor_Cadence_Set: This function is callback for Application
*         when sensor cadence Set message is received
* @param  pCadence_param: Pointer to the parameters received for message
* @param  property_ID: Property is of sensor coming in data packet
* @param  length: Received data length.
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Sensor_Cadence_Set(Sensor_CadenceParam_t* pCadence_param, 
                                      MOBLEUINT16 property_ID, 
                                      MOBLEUINT32 length)                                    
{  
  if(property_ID == (MOBLEUINT16)TEMPERATURE_PID)
  {
    Sensor_CadenceSet[0].Property_ID = pCadence_param->Property_ID;
    Sensor_CadenceSet[0].FastCadenceDevisor = pCadence_param->FastCadenceDevisor ;
    Sensor_CadenceSet[0].StatusTriggerType = pCadence_param->StatusTriggerType ;
    Sensor_CadenceSet[0].triggerDeltaDown = pCadence_param->triggerDeltaDown;
    Sensor_CadenceSet[0].triggerDeltaUp = pCadence_param->triggerDeltaUp;
    Sensor_CadenceSet[0].StatusMinInterval = pCadence_param->StatusMinInterval;
    Sensor_CadenceSet[0].FastCadenceLow = pCadence_param->FastCadenceLow;
    Sensor_CadenceSet[0].FastCadenceHigh = pCadence_param->FastCadenceHigh;
  }
  else if(property_ID == (MOBLEUINT16)PRESSURE_PID)
  {
    Sensor_CadenceSet[1].Property_ID = pCadence_param->Property_ID;
    Sensor_CadenceSet[1].FastCadenceDevisor = pCadence_param->FastCadenceDevisor ;
    Sensor_CadenceSet[1].StatusTriggerType = pCadence_param->StatusTriggerType ;
    Sensor_CadenceSet[1].triggerDeltaDown = pCadence_param->triggerDeltaDown;
    Sensor_CadenceSet[1].triggerDeltaUp = pCadence_param->triggerDeltaUp;
    Sensor_CadenceSet[1].StatusMinInterval = pCadence_param->StatusMinInterval;
    Sensor_CadenceSet[1].FastCadenceLow = pCadence_param->FastCadenceLow;
    Sensor_CadenceSet[1].FastCadenceHigh = pCadence_param->FastCadenceHigh;
  }
  
  else if( property_ID == (MOBLEUINT16) TIME_OF_FLIGHT_PID)
  {
    Sensor_CadenceSet[2].Property_ID = pCadence_param->Property_ID;
    Sensor_CadenceSet[2].FastCadenceDevisor = pCadence_param->FastCadenceDevisor ;
    Sensor_CadenceSet[2].StatusTriggerType = pCadence_param->StatusTriggerType ;
    Sensor_CadenceSet[2].triggerDeltaDown = pCadence_param->triggerDeltaDown;
    Sensor_CadenceSet[2].triggerDeltaUp = pCadence_param->triggerDeltaUp;
    Sensor_CadenceSet[2].StatusMinInterval = pCadence_param->StatusMinInterval;
    Sensor_CadenceSet[2].FastCadenceLow = pCadence_param->FastCadenceLow;
    Sensor_CadenceSet[2].FastCadenceHigh = pCadence_param->FastCadenceHigh;
  }
  
  return MOBLE_RESULT_SUCCESS;
}

/**
* @brief  Appli_Sensor_Cadence_Get: This function is callback for Application
*         when sensor cadence Set message is received
* @param  pCadence_get: Pointer to the parameters received for message
* @param  property_ID: Property is of sensor coming in data packet
* @param  length: Received data length.
* @retval MOBLE_RESULT
*/ 

MOBLE_RESULT Appli_Sensor_Cadence_Get(MOBLEUINT8* sensor_DataCadence, 
                                      MOBLEUINT16 property_ID, MOBLEUINT32 length)
{
  return MOBLE_RESULT_SUCCESS;
}
/**
* @brief  Appli_Sensor_Setting_Set: This function is callback for Application
*         when sensor setting Set message is received
* @param  pSensor_SettingParam: Pointer to the parameters received for message
* @param  OptionalValid: Flag to inform about the validity of optional parameters 
* @retval MOBLE_RESULT
*/ 

MOBLE_RESULT Appli_Sensor_Setting_Set(Sensor_SettingParam_t* pSensor_SettingParam,
                                      MOBLEUINT8 OptionalValid,MOBLEUINT16 prop_ID)
{  
  
  for(int i=0;i< NUMBER_OF_SENSOR;i++)
  {
    if(prop_ID == Property_ID_Table[i].Property_ID)
    {
      Appli_Sensor_SettingSet[i].Property_ID = pSensor_SettingParam->Property_ID;

      Appli_Sensor_SettingSet[i].Sensor_Setting_ID = pSensor_SettingParam->Sensor_Setting_ID;

      Appli_Sensor_SettingSet[i].Sensor_Setting_Access = Sensor_Setting_Access;
      Appli_Sensor_SettingSet[i].Sensor_Setting_Value = pSensor_SettingParam->Sensor_Setting_Value;
    }
  }
  return MOBLE_RESULT_SUCCESS;
}



/**
* @brief  Appli_Sensor_Data_Status: This function is callback for Application
*         when Sensor Get message is received
* @param  sensor_Data: Pointer to buffer to be updated with parameters
* @param  pLength: Length of the parameters to be sent in response
* @param  prop_ID: Property Id of requested sensor (optional)
* @param  length: Received data length
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Sensor_Data_Status(MOBLEUINT8* sensor_Data , 
                                      MOBLEUINT32* pLength, 
                                      MOBLEUINT16 prop_ID , 
                                      MOBLEUINT32 length)
{
  MOBLE_RESULT result = MOBLE_RESULT_FALSE;                                  
  MOBLEUINT32 temperatureData = 0;
  MOBLEUINT32 pressureData = 0;
  MOBLEUINT8 data_Length = 0x03;
  MOBLEUINT32 distance = 0x000000C8; // 200 cm
  MOBLEUINT8 data_Length_UnknownID;
                                     
#if 0
  LPS25HB_GetTemperature((float*)&temperatureData);
  
  LPS25HB_GetPressure((float*)&pressureData); 
#endif
  
  result = Check_Property_ID(Property_ID_Table , prop_ID);
  
  if(result == MOBLE_RESULT_SUCCESS && length > 0)
  {
    if((prop_ID == TEMPERATURE_PID ))    /*  format A */
    {
      /*(prop_Id_Temp & 0x07) << 5) | (Len <<1) Format A 
      Property calculation is done like above line
      */
      *(sensor_Data) = ((TEMPERATURE_PID & 0x07) << 5) | (data_Length <<1) ; 
      *(sensor_Data+1) = (TEMPERATURE_PID >> 3) & 0xFF; 
      
      memcpy(&sensor_Data[2],(void*)&temperatureData,4);
      
      *pLength  =6;    
    } 
    else if((prop_ID == PRESSURE_PID))
    {
      /* Format B for Pressure sensor */
      *(sensor_Data+0) = ((data_Length <<1) | 0x01); 
      *(sensor_Data+1) = (MOBLEUINT8)PRESSURE_PID ;
      *(sensor_Data+2) = (MOBLEUINT8)(PRESSURE_PID >> 8);
      
      memcpy(&sensor_Data[3],(void*)&pressureData,4);
      
      *pLength  =7;    
    }
    else if((prop_ID == TIME_OF_FLIGHT_PID))
    {
      /* Format B for Pressure sensor */
      *(sensor_Data+0) = ((data_Length <<1) | 0x01); 
      *(sensor_Data+1) = (MOBLEUINT8)TIME_OF_FLIGHT_PID ;
      *(sensor_Data+2) = (MOBLEUINT8)(TIME_OF_FLIGHT_PID >> 8);
      
      memcpy(&sensor_Data[3],(void*)&distance,4);
      
      *pLength  =7;    
    }
  }
  else
  {
    if(length == 0)
    {
      /*(prop_Id_Temp & 0x07) << 5) | (Len <<1) Format A 
      Property calculation is done like above line
      */
      *(sensor_Data) = ((TEMPERATURE_PID & 0x07) << 5) | (data_Length <<1) ; 
      *(sensor_Data+1) = (TEMPERATURE_PID >> 3) & 0xFF; 
      
      memcpy(&sensor_Data[2],(void*)&temperatureData,4);
      
      /* Format B for Pressure sensor */
      *(sensor_Data+6) = ((data_Length <<1) | 0x01); 
      *(sensor_Data+7) = (MOBLEUINT8)PRESSURE_PID ;
      *(sensor_Data+8) = (MOBLEUINT8)(PRESSURE_PID >> 8);
      
      memcpy(&sensor_Data[9],(void*)&pressureData,4);
      
      /* Format B for Pressure sensor */
      *(sensor_Data+13) = ((data_Length <<1) | 0x01); 
      *(sensor_Data+14) = (MOBLEUINT8)TIME_OF_FLIGHT_PID ;
      *(sensor_Data+15) = (MOBLEUINT8)(TIME_OF_FLIGHT_PID >> 8);
      
      memcpy(&sensor_Data[16],(void*)&distance,4);
      
      *pLength  =20;    
    }  
    else
    {
//    *(sensor_Data+0) = (MOBLEUINT8)prop_ID ;
//     *(sensor_Data+1) = (MOBLEUINT8)(prop_ID >> 8);
//     
//     *pLength =2;
        
      data_Length_UnknownID = 0x0F;
      
      if(prop_ID < 0x0800)     
      {
        *(sensor_Data) = ((prop_ID & 0x07) << 5) | (data_Length_UnknownID << 1) ; 
        *(sensor_Data+1) = (prop_ID >> 3) & 0xFF; 
        
        *pLength=2;
      }
      else
      {
        *(sensor_Data+0) = 0xFF;  //((data_Length <<1) | 0x01) & 0x00; 
        *(sensor_Data+1) = (MOBLEUINT8)prop_ID ;
        *(sensor_Data+2) = (MOBLEUINT8)(prop_ID >> 8);
        
        *pLength = 3;
      }
    }
      //*plength=0;
  }
                                     
  TRACE_M(TF_SENSOR,"the temperature reading from sender in hex 0x%08lx \n\r ", temperatureData);
  TRACE_M(TF_SENSOR,"the pressure reading from sender in hex 0x%08lx \n\r", pressureData );
      
  return MOBLE_RESULT_SUCCESS;
}

                                    
/**
* @brief  Appli_Sensor_Descriptor_Status: This function is callback for Application
*         when sensor get message is received
* @param  sensor_Descriptor: Pointer to the parameters to be send in message
* @param  pLength: Length of the parameters to be sent in response
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Sensor_Descriptor_Status(MOBLEUINT8* sensor_Descriptor , 
                                            MOBLEUINT32* pLength,MOBLEUINT16 prop_ID , 
                                            MOBLEUINT32 length)
{
  Appli_Sensor_DescriptorStatus_t Appli_Sensor_DescriptorStatus1[] = 
  {
    {PRESSURE_PID,0xABC,0xDEF,0x03,0x04,0x05},
    {TEMPERATURE_PID,0xc56,0xd78,0x06,0x07,0x08},
    {TIME_OF_FLIGHT_PID,0xD23,0xE45,0x06,0x07,0x08}
  };
  MOBLEUINT32 tolerance;
  MOBLE_RESULT result = MOBLE_RESULT_FALSE;
  result = Check_Property_ID(Property_ID_Table , prop_ID);
  
  if (result != MOBLE_RESULT_FALSE )
  {
    if(prop_ID == TEMPERATURE_PID)
    {
      tolerance = Appli_Sensor_DescriptorStatus1[0].NegativeTolerance;
      tolerance = (tolerance << 12 ) | 
                  Appli_Sensor_DescriptorStatus1[0].PositiveTolerance;
      
      *(sensor_Descriptor) = Appli_Sensor_DescriptorStatus1[0].Prop_ID;
      *(sensor_Descriptor+1) = Appli_Sensor_DescriptorStatus1[0].Prop_ID >> 8;
      *(sensor_Descriptor+2) = tolerance;
      *(sensor_Descriptor+3) = tolerance >> 8;
      *(sensor_Descriptor+4) = tolerance >> 16;
      *(sensor_Descriptor+5) = Appli_Sensor_DescriptorStatus1[0].SamplingFunction;
      *(sensor_Descriptor+6) = Appli_Sensor_DescriptorStatus1[0].MeasurementPeriod;
      *(sensor_Descriptor+7) = Appli_Sensor_DescriptorStatus1[0].UpdateInterval;
      *pLength = 8;
    }
    else if(prop_ID == PRESSURE_PID)
    {
      tolerance = Appli_Sensor_DescriptorStatus1[1].NegativeTolerance;
      tolerance = (tolerance << 12 ) | Appli_Sensor_DescriptorStatus1[1].PositiveTolerance ;
      
      *(sensor_Descriptor) = Appli_Sensor_DescriptorStatus1[1].Prop_ID;
      *(sensor_Descriptor+1) = Appli_Sensor_DescriptorStatus1[1].Prop_ID >> 8;
      *(sensor_Descriptor+2) = tolerance;
      *(sensor_Descriptor+3) = tolerance >> 8;
      *(sensor_Descriptor+4) = tolerance >> 16;
      *(sensor_Descriptor+5) = Appli_Sensor_DescriptorStatus1[1].SamplingFunction;
      *(sensor_Descriptor+6) = Appli_Sensor_DescriptorStatus1[1].MeasurementPeriod;
      *(sensor_Descriptor+7) = Appli_Sensor_DescriptorStatus1[1].UpdateInterval;
      *pLength =8;
    }
    else if(prop_ID == TIME_OF_FLIGHT_PID)
    {
      tolerance = Appli_Sensor_DescriptorStatus1[2].NegativeTolerance;
      tolerance = (tolerance << 12 ) | Appli_Sensor_DescriptorStatus1[2].PositiveTolerance ;
      
      *(sensor_Descriptor) = Appli_Sensor_DescriptorStatus1[2].Prop_ID;
      *(sensor_Descriptor+1) = Appli_Sensor_DescriptorStatus1[2].Prop_ID >> 8;
      *(sensor_Descriptor+2) = tolerance;
      *(sensor_Descriptor+3) = tolerance >> 8;
      *(sensor_Descriptor+4) = tolerance >> 16;
      *(sensor_Descriptor+5) = Appli_Sensor_DescriptorStatus1[2].SamplingFunction;
      *(sensor_Descriptor+6) = Appli_Sensor_DescriptorStatus1[2].MeasurementPeriod;
      *(sensor_Descriptor+7) = Appli_Sensor_DescriptorStatus1[2].UpdateInterval;
      
      *pLength =8;
    }
  }
  else 
  {
    if(length ==0 )
    {
      tolerance = Appli_Sensor_DescriptorStatus1[0].NegativeTolerance;
      tolerance = (tolerance << 12 ) | Appli_Sensor_DescriptorStatus1[0].PositiveTolerance;
  
      *(sensor_Descriptor) = Appli_Sensor_DescriptorStatus1[0].Prop_ID;
      *(sensor_Descriptor+1) = Appli_Sensor_DescriptorStatus1[0].Prop_ID >> 8;
      *(sensor_Descriptor+2) = tolerance;
      *(sensor_Descriptor+3) = tolerance >> 8;
      *(sensor_Descriptor+4) = tolerance >> 16;
      *(sensor_Descriptor+5) = Appli_Sensor_DescriptorStatus1[0].SamplingFunction;
      *(sensor_Descriptor+6) = Appli_Sensor_DescriptorStatus1[0].MeasurementPeriod;
      *(sensor_Descriptor+7) = Appli_Sensor_DescriptorStatus1[0].UpdateInterval;
      
       tolerance = Appli_Sensor_DescriptorStatus1[1].NegativeTolerance;
       tolerance = (tolerance << 12 ) | Appli_Sensor_DescriptorStatus1[1].PositiveTolerance ;
        
      *(sensor_Descriptor+8) = Appli_Sensor_DescriptorStatus1[1].Prop_ID;
      *(sensor_Descriptor+9) = Appli_Sensor_DescriptorStatus1[1].Prop_ID >> 8;
      *(sensor_Descriptor+10) = tolerance;
      *(sensor_Descriptor+11) = tolerance >> 8;
      *(sensor_Descriptor+12) = tolerance >> 16;
      *(sensor_Descriptor+13) = Appli_Sensor_DescriptorStatus1[1].SamplingFunction;
      *(sensor_Descriptor+14) = Appli_Sensor_DescriptorStatus1[1].MeasurementPeriod;
      *(sensor_Descriptor+15) = Appli_Sensor_DescriptorStatus1[1].UpdateInterval;
      
       tolerance = Appli_Sensor_DescriptorStatus1[2].NegativeTolerance;
       tolerance = (tolerance << 12 ) | Appli_Sensor_DescriptorStatus1[2].PositiveTolerance ;
        
      *(sensor_Descriptor+16) = Appli_Sensor_DescriptorStatus1[2].Prop_ID;
      *(sensor_Descriptor+17) = Appli_Sensor_DescriptorStatus1[2].Prop_ID >> 8;
      *(sensor_Descriptor+18) = tolerance;
      *(sensor_Descriptor+19) = tolerance >> 8;
      *(sensor_Descriptor+20) = tolerance >> 16;
      *(sensor_Descriptor+21) = Appli_Sensor_DescriptorStatus1[2].SamplingFunction;
      *(sensor_Descriptor+22) = Appli_Sensor_DescriptorStatus1[2].MeasurementPeriod;
      *(sensor_Descriptor+23) = Appli_Sensor_DescriptorStatus1[2].UpdateInterval;
      
      *pLength = 24;
    }
    else if(prop_ID != 0x0000)
    {
      *(sensor_Descriptor) = prop_ID >> 8;
      *(sensor_Descriptor+1) = prop_ID;
      *pLength = 2; 
    }
    else
    {
      
    }
  }
  
  //  }
  //else if(result == MOBLE_RESULT_FALSE && length >0)
  //{
  //   *(sensor_Descriptor) = prop_ID >> 8;
  //  *(sensor_Descriptor+1) = prop_ID;
  //  *pLength = 2;  
  //}
  //else
  //{
  //  
  //}  
  return MOBLE_RESULT_SUCCESS;
}


MOBLE_RESULT Appli_Sensor_Column_Status(MOBLEUINT8* sensor_Column , MOBLEUINT32* pLength,MOBLEUINT16 prop_ID , MOBLEUINT32 length)
{
  
  Sensor_Column_param_t Appli_Sensor_ColumnStatus[]=
  {
    {PRESSURE_PID,0xD9AF,0xDE03,0x0405},
    {TEMPERATURE_PID,0xacd2,0x3456,0x6537},
    {TIME_OF_FLIGHT_PID,0x8248,0xDE03,0x0405}    
  };
  
  MOBLEUINT8 flag = 0;
  
  for(int i=0;i< NUMBER_OF_SENSOR ;i++)
  {
    if(prop_ID == Appli_Sensor_ColumnStatus[i].Property_ID)
    {
      flag = 1;
      *sensor_Column= Appli_Sensor_ColumnStatus[i].Property_ID;
        *(sensor_Column+1) = Appli_Sensor_ColumnStatus[i].Property_ID>>8;
      
      *(sensor_Column+2)=Appli_Sensor_ColumnStatus[i].RawValueX;
      *(sensor_Column+3)=Appli_Sensor_ColumnStatus[i].RawValueX;
      
      
      *(sensor_Column+4)=Appli_Sensor_ColumnStatus[i].RawValueWidth;
      *(sensor_Column+5)=Appli_Sensor_ColumnStatus[i].RawValueWidth;
      
      *(sensor_Column+6)=Appli_Sensor_ColumnStatus[i].RawValueY;
      *(sensor_Column+7)=Appli_Sensor_ColumnStatus[i].RawValueY;
      *pLength = 8;
      break;
    }
  }
  
  if(flag == 0)
  {
    *pLength =4;
  }
  
  return MOBLE_RESULT_SUCCESS;
}


MOBLE_RESULT Appli_Sensor_Series_Status(MOBLEUINT8* sensor_Series , MOBLEUINT32* pLength,MOBLEUINT16 prop_ID , MOBLEUINT32 length)
{
  Sensor_Series_param_t Appli_Sensor_SeriesStatus[NUMBER_OF_SENSOR]=
  {
    {PRESSURE_PID,{{0xabcd,0x73ac,0xcdef},{0xacd2,0x2345,0x1234}}},
    {TEMPERATURE_PID,{{0x00C1,0x0071,0x00DE},{0x0003,0x0004,0x0005}}},
    {TIME_OF_FLIGHT_PID,{{0x4528,0xbad3,0xdc12},{0xacd2,0x2345,0x1234}}}
  };
  
  for(int i=0;i<NUMBER_OF_SENSOR;i++)
  {
    if( prop_ID == Appli_Sensor_SeriesStatus[i].Property_ID )
    {
      
      *sensor_Series= Appli_Sensor_SeriesStatus[i].Property_ID;
      *(sensor_Series+1) = Appli_Sensor_SeriesStatus[i].Property_ID>>8;
      for(int y=0 ;y < SENSOR_SERIES_VALUE ;y++)
      {
        *(sensor_Series+2+6*y) =Appli_Sensor_SeriesStatus[i].SeriesData[y].RawValueX>>8;
        *(sensor_Series+3+6*y) =Appli_Sensor_SeriesStatus[i].SeriesData[y].RawValueX;
        *(sensor_Series+4+6*y) =Appli_Sensor_SeriesStatus[i].SeriesData[y].RawColumnWidth>>8;
        *(sensor_Series+5+6*y) =Appli_Sensor_SeriesStatus[i].SeriesData[y].RawColumnWidth;
        *(sensor_Series+6+6*y) =Appli_Sensor_SeriesStatus[i].SeriesData[y].RawValueY>>8;
        *(sensor_Series+7+6*y) =Appli_Sensor_SeriesStatus[i].SeriesData[y].RawValueY;
      }
      *pLength = 2 + 6*SENSOR_SERIES_VALUE ;
      break;
    }
  }
  
return  MOBLE_RESULT_SUCCESS ;

}

#endif

/**
* @brief  Sensor Process Function will continuously monitor the sensors.
*         Function used for the Publishing, data monitoring..
* @param  void
* @retval void
*/ 
void Sensor_Process(void)
{
  
#ifdef ENABLE_SENSOR_PUBLICATION    
  float sensorValue[NUMBER_OF_SENSOR];
  if(ProvisionFlag == 1)
  {
    Read_Sensor_Data(&sensorValue[0]);
    Sensor_Publication_Process(&sensorValue[0], &Property_ID_Table[0]);
  }
#endif
  
  /* Occupancy_Flag become True when ever sensor detect occupancy and get interrupt
     and make flag True to run this routine.
  */ 
  if(Occupancy_Flag == MOBLE_TRUE) 
  {
    if(BLE_waitPeriod(CONTROLLER_WAIT_TIME))
    {
#ifdef ENABLE_SENSOR_PUBLICATION    
      if(ProvisionFlag == 1)
      {
        Read_Sensor_Data(&sensorValue[0]);
        Sensor_Publication_Process(&sensorValue[0], &Property_ID_Table[0]);
      }
#endif
      /* publishing the command for LC Light occupancy set message in the sensor status 
         message .
      */     
      Sensor_LC_Light_Publish();  
      Occupancy_Flag = MOBLE_FALSE;   
    }  
  }  
}

/**
* @brief  Function check for the couupancy in the location and send the status
*         message with the ocuppancy value, when the interrupt is detected.
* @param  void     
* @retval void
*/  
void Sensor_LC_Light_Publish(void)
{
  MOBLEUINT8 occupancyData = 0x1;
  MOBLEUINT8 sensor_Data[5];
  MOBLE_ADDRESS srcAdd;
  MOBLE_RESULT result = MOBLE_RESULT_SUCCESS;
  
  sensor_Data[1] = (MOBLEUINT8)(PRESENCE_DETECTED_PROPERTY << 8);
  sensor_Data[0] = (MOBLEUINT8)PRESENCE_DETECTED_PROPERTY;
  sensor_Data[2] = occupancyData;
  
  srcAdd = BLEMesh_GetAddress();
  
  result = BLEMesh_SetRemotePublication(SENSOR_SERVER_MODEL_ID, srcAdd ,
                                        SENSOR_STATUS , 
                                        sensor_Data,3,
                                        MOBLE_FALSE, MOBLE_FALSE);
   
  if(result)
  {
    TRACE_M(TF_LIGHT_LC,"Publication Error \r\n");
  }
   
}

#if defined ENABLE_SENSOR_PUBLICATION && defined ENABLE_SENSOR_MODEL_SERVER
/**
* @brief  Function read the particular sensor value which are called inside.
* @param  pSensorValue: pointer to the sensor data array.       
* @retval void
*/ 
void Read_Sensor_Data(float *pSensorValue)
{
#if 0
  float temp,press;
  LPS25HB_GetTemperature(&temp);
  pSensorValue[0] = temp;
  LPS25HB_GetPressure(&press); 
  pSensorValue[1] = press;
#endif
}

/**
* @brief  Sensor Publication Process function
* @param  Function will publish the sensor data according to the given conditions.
* @param  void type function pointer.
* @param  pSensorData: Pointer to the sensor data array.
* @param  pProp_ID: Pointer to the Property id of sensor array.
* @retval void
*/
void Sensor_Publication_Process(float* pSensorData, MODEL_Property_IDTableParam_t* pProp_ID)
{
  static MOBLEUINT8 sensor_Count = 0;
  displayFloatToInt_t out_value;
  MOBLEUINT8 devisorValue;
  MOBLEUINT32 publishTime;
  static MOBLEUINT32 cadenceDurationTick[NUMBER_OF_SENSOR];
  static float previousDataValue[NUMBER_OF_SENSOR];
  static PublishingDataFlag_t PublishingDataFlag[NUMBER_OF_SENSOR] = {MOBLE_FALSE};
  
  floatToInt(pSensorData[sensor_Count], &out_value, 2);
  
  /* Taking the timestamp for the cadence publication and making flag high */
  if(PublishingDataFlag[sensor_Count].CadenceDurationFlag == MOBLE_FALSE)
  {
    cadenceDurationTick[sensor_Count] = Clock_Time();
    PublishingDataFlag[sensor_Count].CadenceDurationFlag = MOBLE_TRUE;          
  }
  /* Taking the sensor value and store it for comparing present sensor value with
  particular difference of increasing or decreasing. and making flag high.
  */
  if(PublishingDataFlag[sensor_Count].DeltaDataFlag == MOBLE_FALSE)
  {
    previousDataValue[sensor_Count] = pSensorData[sensor_Count];
    PublishingDataFlag[sensor_Count].DeltaDataFlag = MOBLE_TRUE;
  }
  /*
  This condition is checking for the difference of present sensor value 
  with prestored sensor value with user defined difference,if this condition 
  is true then it publish the sensor data.And making the delta flag low again.
  */       
  if((pSensorData[sensor_Count] >= (previousDataValue[sensor_Count] + Sensor_CadenceSet[sensor_Count].triggerDeltaUp)) ||
     (pSensorData[sensor_Count] <= (previousDataValue[sensor_Count] - Sensor_CadenceSet[sensor_Count].triggerDeltaDown)))
  {        
    MOBLEUINT16 prop_id = pProp_ID[sensor_Count].Property_ID;
    
    SensorDataPublish((MOBLEUINT32*)&pSensorData[sensor_Count] , &prop_id);
        
    PublishingDataFlag[sensor_Count].DeltaDataFlag = MOBLE_FALSE;
    TRACE_M(TF_SENSOR,"previous value data %.3f \r\n",previousDataValue[sensor_Count]);
    TRACE_M(TF_SENSOR,"Delta publication of data %.3f\r\n",*((float*)&pSensorData[sensor_Count]));
    sensor_Count++;
  }
  /*
  This condition is continuously checking the sensor value range, if that 
  value is within the user defined range then publishing duration or rate will
  be divided by user definedcadence devisor value and rate of publishing will 
  become high.And making the cadence flag low again.
  */
  if(((out_value.out_int <= Sensor_CadenceSet[sensor_Count].FastCadenceHigh) && 
      (out_value.out_int >= Sensor_CadenceSet[sensor_Count].FastCadenceLow)) ||
     (Sensor_CadenceSet[sensor_Count].FastCadenceHigh < Sensor_CadenceSet[sensor_Count].FastCadenceLow))
  {
    devisorValue = (MOBLEUINT8)pow(2 ,Sensor_CadenceSet[sensor_Count].FastCadenceDevisor);
    publishTime = SENSOR_PUBLISH_PERIOD/devisorValue;  
    
    if(((Clock_Time()- cadenceDurationTick[sensor_Count]) >= publishTime))
    {                   
      MOBLEUINT16 prop_id = pProp_ID[sensor_Count].Property_ID;

      SensorDataPublish((MOBLEUINT32*)&pSensorData[sensor_Count] , &prop_id);                 
      PublishingDataFlag[sensor_Count].CadenceDurationFlag = MOBLE_FALSE;           
      TRACE_M(TF_SENSOR,"Cadence publication of data %.2f \r\n",*((float*)&pSensorData[sensor_Count])); 
      sensor_Count++;
    } 
  }
  else
  {
    publishTime = SENSOR_PUBLISH_PERIOD ;     
    
    if(((Clock_Time()- cadenceDurationTick[sensor_Count]) >= SENSOR_PUBLISH_PERIOD))
    {            
      MOBLEUINT16 prop_id = pProp_ID[sensor_Count].Property_ID;

      SensorDataPublish((MOBLEUINT32*)&pSensorData[sensor_Count] , &prop_id);
      
      PublishingDataFlag[sensor_Count].CadenceDurationFlag = MOBLE_FALSE;                
      TRACE_M(TF_SENSOR,"Regular publication of data %.3f \r\n",*((float*)&pSensorData[sensor_Count]));
    }  
  } 
  if(sensor_Count > 1)
  {
   sensor_Count = 0;
  }

}

/**
* @brief  BLEMesh Sensor Publication function
* @param  Function will decide the publish address and element.
* @param  pSensor_Value: Pointer to the sensor data array 
* @param  pProp_ID: pointer to the property id of sensor array.
* @retval void
*/
void SensorDataPublish(MOBLEUINT32 *pSensor_Value , MOBLEUINT16* pProp_ID)
{
  MOBLEUINT32 length;
  MOBLEUINT8 sensor_Data[8];
  MOBLE_ADDRESS srcAdd;
  MOBLE_RESULT result = MOBLE_RESULT_SUCCESS;
  
  srcAdd = BLEMesh_GetAddress();
  
  switch(*pProp_ID)
  {
    case TEMPERATURE_PID:
      { 
        /*(prop_Id_Temp & 0x07) << 5) | (Len <<1) Format A 
        Property calculation is done like above line
        */
        sensor_Data[0] = ((TEMPERATURE_PID & 0x07) << 5) | (3 <<1) ; 
        sensor_Data[1] = (TEMPERATURE_PID >> 3) & 0xFF; 
        
        memcpy(&sensor_Data[2],(void*)&pSensor_Value[0],4);
        length  =6;  
        break;
      }
    case PRESSURE_PID:
      {
        /* Format B for Pressure sensor */
        sensor_Data[0] = ((0x03 <<1) | 0x01); 
        sensor_Data[1] = (MOBLEUINT8)PRESSURE_PID ;
        sensor_Data[2] = (MOBLEUINT8)(PRESSURE_PID >> 8);
        
        memcpy(&sensor_Data[3],(void*)&pSensor_Value[0],4);  
        length  =7;     
        break;
      }
    case TIME_OF_FLIGHT_PID:  
      {
        sensor_Data[0] = ((0x03 <<1) | 0x01); 
        sensor_Data[1] = (MOBLEUINT8)TIME_OF_FLIGHT_PID ;
        sensor_Data[2] = (MOBLEUINT8)(TIME_OF_FLIGHT_PID >> 8);
        
        memcpy(&sensor_Data[3],(void*)&pSensor_Value[0],4);  
        length  =7;     
        break;
      }
    default:
      break;
  }
  
  result = BLEMesh_SetRemotePublication(SENSOR_SERVER_MODEL_ID, srcAdd,
                                        SENSOR_STATUS , 
                                        sensor_Data,length,
                                        MOBLE_FALSE, MOBLE_FALSE);
  
  if(result)
  {
    TRACE_M(TF_SENSOR,"Publication Error \r\n");
  }
  
}

#endif

#ifdef ENABLE_SENSOR_MODEL_SERVER
#if 0
/**
* @brief  Appli_Sensor_GetSettingStatus: This function is callback for Application
*         when sensor setting numbers status message is to be provided
* @param  pSetting_Status: Pointer to the status message
* @retval MOBLE_RESULT
*/ 
/*
MOBLE_RESULT Appli_Sensor_GetSettingStatus(MOBLEUINT8* pSetting_Status)                                        
{
  
  *pSetting_Status = Appli_Sensor_SettingSet.Property_ID;
  *(pSetting_Status+1) = Appli_Sensor_SettingSet.Property_ID >> 8;
  *(pSetting_Status+2) = Appli_Sensor_SettingSet.Sensor_Setting_ID; 
  *(pSetting_Status+3) = Appli_Sensor_SettingSet.Sensor_Setting_ID >> 8;
  
  return MOBLE_RESULT_SUCCESS; 
}*/
#endif

/**
* @brief  Appli_Sensor_GetSetting_IDStatus: This function is callback for Application
*         when sensor setting numbers and row value status message is to be provided
* @param  pSetting_Status: Pointer to the status message
* @retval MOBLE_RESULT
*/ 

MOBLE_RESULT Appli_Sensor_GetSetting_IDStatus(MOBLEUINT8* pSetting_Status , MOBLEUINT16 prop_ID)                                        
{
  for(int i=0 ;i<NUMBER_OF_SENSOR;i++)
  {
    if(prop_ID == Appli_Sensor_SettingSet[i].Property_ID)
    {
      *pSetting_Status = Appli_Sensor_SettingSet[i].Property_ID;
      *(pSetting_Status+1) = Appli_Sensor_SettingSet[i].Property_ID >> 8;      
      *(pSetting_Status+2) = Appli_Sensor_SettingSet[i].Sensor_Setting_ID; 
      *(pSetting_Status+3) = Appli_Sensor_SettingSet[i].Sensor_Setting_ID >> 8;
      *(pSetting_Status+4) = Appli_Sensor_SettingSet[i].Sensor_Setting_Access; 
      
      if(Appli_Sensor_SettingSet[i].Sensor_Setting_Access == 0x03)
      {
        *(pSetting_Status+5) = Appli_Sensor_SettingSet[i].Sensor_Setting_Value;
        *(pSetting_Status+6) = Appli_Sensor_SettingSet[i].Sensor_Setting_Value >> 8;
      }
      else
      {
        
      }
    }
  } 
  
  return MOBLE_RESULT_SUCCESS; 
}

#endif

/**
* @brief  Check_Property_ID: This function is used for checking the Property id 
*         of sensor available in table.
* @param  prop_ID_Table: address of the property id table array.
* @param  prop_ID:received property id of sensor.
* @retval MOBLE_RESULT
*/
MOBLE_RESULT Check_Property_ID(const MODEL_Property_IDTableParam_t prop_ID_Table[], 
                               MOBLEUINT16 prop_ID)
{
  MOBLE_RESULT result = MOBLE_RESULT_FALSE;
  
  for(MOBLEUINT8 i=0; i<NUMBER_OF_SENSOR; i++)    
  {
    if(prop_ID_Table[i].Property_ID == prop_ID)
    {       
      result =  MOBLE_RESULT_SUCCESS;
      break;
    }
  }     
  
  return result;
}

/**
* @brief  Appli_Sensor_Init: This function is callback for Initialisation of 
*         Application interface
* @param  void 
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Sensor_Init(void)                                        
{
#if 0
  LPS25HB_Init(&InitStructure);
#endif   
  return MOBLE_RESULT_SUCCESS;
}


/**
 * @}
 */

/**
 * @}
 */

/******************* (C) COPYRIGHT 2019 STMicroelectronics *****END OF FILE****/

