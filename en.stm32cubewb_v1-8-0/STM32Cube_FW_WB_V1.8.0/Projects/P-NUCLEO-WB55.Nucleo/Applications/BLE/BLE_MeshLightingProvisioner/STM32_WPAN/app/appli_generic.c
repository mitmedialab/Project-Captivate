/**
  ******************************************************************************
  * @file    appli_generic.c
  * @author  BLE Mesh Team
  * @brief   Application interface for Generic Mesh Models 
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
#include "app_conf.h"
#include "hal_common.h"
#include "types.h"
#include "ble_mesh.h"
#include "appli_mesh.h"
#include "mesh_cfg.h"
#include "generic.h"
#include "light.h"
#include "appli_generic.h"
#include "appli_light.h"
#include "common.h"
#include "mesh_cfg_usr.h"
#include "appli_nvm.h"

/** @addtogroup ST_BLE_Mesh
 *  @{
 */

/** @addtogroup Application_Mesh_Models
 *  @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
   
extern MOBLEUINT8 RestoreFlag;
extern MOBLEUINT16 IntensityValue;
extern MOBLEUINT8 IntensityFlag;
extern MOBLEUINT8 PowerOnOff_flag;
extern Appli_LightPwmValue_t Appli_LightPwmValue;
Appli_Generic_OnOffSet AppliOnOffSet;
Appli_Generic_LevelSet AppliLevelSet;
Appli_Generic_PowerOnOffSet AppliPowerOnSet;
Appli_Generic_DefaultTransitionSet AppliDefaultTransitionSet;

/* Private function prototypes -----------------------------------------------*/
MOBLE_RESULT Appli_Generic_Move_Set(Generic_LevelStatus_t* pdeltaMoveParam, 
                                    MOBLEUINT8 OptionalValid);
MOBLE_RESULT Appli_Generic_Delta_Set(Generic_LevelStatus_t* pdeltalevelParam, 
                                     MOBLEUINT8 OptionalValid);
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
#ifdef ENABLE_GENERIC_MODEL_SERVER_ONOFF
/******************************************************************************/

/**
* @brief  Appli_Generic_OnOff_Set: This function is callback for Application
*          when Generic OnOff message is received
* @param  pGeneric_OnOffParam: Pointer to the parameters received for message
* @param  OptionalValid: Flag to inform about the validity of optional parameters 
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Generic_OnOff_Set(Generic_OnOffStatus_t* pGeneric_OnOffParam, 
                                     MOBLEUINT8 OptionalValid)
{
  AppliOnOffSet.Present_OnOff = pGeneric_OnOffParam->Present_OnOff_State;
  AppliOnOffSet.Present_OnOffValue = pGeneric_OnOffParam->Present_OnOff_Value;
   

  /* This condition is applicable when user want to on off the light with some default 
   * transition value, or optionalValid =IN_TRANSITION ,  transition is in progress.
   */
  if((OptionalValid == DEFAULT_TRANSITION) || (OptionalValid == IN_TRANSITION))
  {
    Appli_LightPwmValue.IntensityValue = AppliOnOffSet.Present_OnOffValue;
    Light_UpdateLedValue(LOAD_STATE ,Appli_LightPwmValue);
    if(AppliOnOffSet.Present_OnOff == APPLI_LED_ON)
    {
      AppliOnOffSet.TargetValue = PWM_TIME_PERIOD;
    }
    else
    {
      AppliOnOffSet.TargetValue = APPLI_LED_OFF;
    }

    if(AppliOnOffSet.Present_OnOffValue > 16000)
    {
      BSP_LED_On(LED_BLUE);
    }
    else
    {
      BSP_LED_Off(LED_BLUE);
    }
  }  
  else
  {
    if((AppliOnOffSet.Present_OnOff == APPLI_LED_ON) && (OptionalValid == NO_TRANSITION))
    { 
      Appli_LightPwmValue.IntensityValue = PWM_TIME_PERIOD;
      Light_UpdateLedValue(LOAD_STATE , Appli_LightPwmValue);
      BSP_LED_On(LED_BLUE);
    }
    else
    {  
      Appli_LightPwmValue.IntensityValue = PWM_VALUE_OFF;
      Light_UpdateLedValue(RESET_STATE , Appli_LightPwmValue);
      BSP_LED_Off(LED_BLUE);
    } 
  }

  TRACE_M(TF_SERIAL_CTRL,"#8202%02hx!\n\r",AppliOnOffSet.Present_OnOff);

  /* set the flag value for NVM store */
  RestoreFlag = GENERIC_ON_OFF_NVM_FLAG;
  
  AppliNvm_SaveMessageParam();
    
  return MOBLE_RESULT_SUCCESS;
}


/******************************************************************************/
#endif  /* #ifdef ENABLE_GENERIC_MODEL_SERVER_ONOFF */
/******************************************************************************/


/**
* @brief  Appli_Generic_OnOff_Status: This function is callback for Application
*          when Generic OnOff message is received
* @param  pOnOff_status: Pointer to the parameters received for message
* @param  plength: length of the data 
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Generic_OnOff_Status(MOBLEUINT8 const *pOnOff_status, MOBLEUINT32 plength)
{
  MOBLEUINT8 i;
  
  TRACE_M(TF_GENERIC,"Appli_Generic_OnOff_Status callback received \r\n");
  
  TRACE_M(TF_SERIAL_CTRL,"#8204! \n\r");
  for(i = 0; i < plength; i++)
  {
    if(i == 0)
      TRACE_M(TF_SERIAL_CTRL,"Present OnOff value: %d\n\r", pOnOff_status[i]);
    else if(i == 1)
      TRACE_M(TF_SERIAL_CTRL,"Target OnOff value: %d\n\r", pOnOff_status[i]);
    else if(i == 2)
      TRACE_M(TF_SERIAL_CTRL,"Remaining Time value: %d\n\r", pOnOff_status[i]);
  }
  
  return MOBLE_RESULT_SUCCESS;
}

/******************************************************************************/
#ifdef ENABLE_GENERIC_MODEL_SERVER_LEVEL
/******************************************************************************/

/**
* @brief  Appli_Generic_Level_Set: This function is callback for Application
*          when Generic Level message is received
* @param  plevelParam: Pointer to the parameters message
* @param  OptionalValid: Flag to inform about the validity of optional parameters 
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Generic_Level_Set(Generic_LevelStatus_t* plevelParam, 
                                     MOBLEUINT8 OptionalValid)
{
  MOBLEUINT16 duty;
  static MOBLEUINT16 previousIntensity = 0;
   
  AppliLevelSet.Present_Level16= plevelParam->Present_Level16;
  if(AppliLevelSet.Present_Level16 <= 0)
  {
    AppliLevelSet.Present_Level16 = 0;
  }
  
  IntensityValue =  AppliLevelSet.Present_Level16;
  
  if(((IntensityValue > previousIntensity) && (IntensityValue <PWM_TIME_PERIOD)) ||
     IntensityValue == INTENSITY_LEVEL_ZERO)
  {
    IntensityFlag = MOBLE_FALSE;
  }
  else
  {
    IntensityFlag = MOBLE_TRUE;
  }
  previousIntensity = IntensityValue;     
  
  if(AppliLevelSet.Present_Level16 < 0x00)
  {
     AppliLevelSet.Present_Level16 = 0;
  }
  duty = PwmValueMapping(AppliLevelSet.Present_Level16 , 0x7FFF ,0x0000); 
  Appli_LightPwmValue.IntensityValue = duty;
  Light_UpdateLedValue(LOAD_STATE , Appli_LightPwmValue);

  TRACE_M(TF_SERIAL_CTRL,"#8206%04hx!\n\r",AppliLevelSet.Present_Level16);

  /* set the flag value for NVM store */
  RestoreFlag = GENERIC_LEVEL_NVM_FLAG;

  AppliNvm_SaveMessageParam();
    
  /* For demo, if Level is more than 100, switch ON the LED */
  if(AppliLevelSet.Present_Level16 >= 50)
  {
    BSP_LED_On(LED_BLUE);
  }
  else
  {
    BSP_LED_Off(LED_BLUE);
  }
  
  return MOBLE_RESULT_SUCCESS;
}

/**
* @brief  Appli_Generic_Delta_Set: This function is callback for Application
*          when Generic Level Delta message is received
* @param  pdeltalevelParam: Pointer to the parameters message
* @param  OptionalValid: Flag to inform about the validity of optional parameters 
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Generic_Delta_Set(Generic_LevelStatus_t* pdeltalevelParam, 
                                     MOBLEUINT8 OptionalValid)
{
  
  AppliLevelSet.Present_Level16 = pdeltalevelParam->Present_Level16;
  
  /* For demo, if Level is more than 50, switch ON the LED */
  if (AppliLevelSet.Present_Level16 >= 50)
  {
    Appli_LightPwmValue.IntensityValue = PWM_TIME_PERIOD;
    Light_UpdateLedValue(LOAD_STATE , Appli_LightPwmValue);
    BSP_LED_On(LED_BLUE);
  }
  else
  {
    Light_UpdateLedValue(RESET_STATE , Appli_LightPwmValue);
    BSP_LED_Off(LED_BLUE);
  }
  TRACE_M(TF_SERIAL_CTRL,"#8206!\n\r");
  
  return MOBLE_RESULT_SUCCESS;
}


/**
* @brief  Appli_Generic_Move_Set: This function is callback for Application
*          when Generic Level Move message is received
* @param  pdeltaMoveParam: Pointer to the parameters message
* @param  OptionalValid: Flag to inform about the validity of optional parameters 
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Generic_Move_Set(Generic_LevelStatus_t* pdeltaMoveParam, 
                                               MOBLEUINT8 OptionalValid)
{
  AppliLevelSet.Last_delta_level     = pdeltaMoveParam->Last_delta_level;
  AppliLevelSet.Last_Level_TID       = pdeltaMoveParam->Last_Level_TID;
  AppliLevelSet.Last_Present_Level16 = pdeltaMoveParam->Last_Present_Level16;
  AppliLevelSet.Present_Level16      = pdeltaMoveParam->Present_Level16;
  AppliLevelSet.RemainingTime        = pdeltaMoveParam->RemainingTime;
  AppliLevelSet.Target_Level16       = pdeltaMoveParam->Target_Level16;
  
   TRACE_M(TF_SERIAL_CTRL,"#8206!\n\r");
  
  return MOBLE_RESULT_SUCCESS;
  
}

/******************************************************************************/
#endif   /* ENABLE_GENERIC_MODEL_SERVER_LEVEL */
/******************************************************************************/

  /**
* @brief  Appli_Generic_Level_Status: This function is callback for Application
*          when Generic Level Move message is received
* @param  plevel_status: Pointer to the parameters message
* @param  plength: length of data 
* @retval MOBLE_RESULT
*/
MOBLE_RESULT Appli_Generic_Level_Status(MOBLEUINT8 const *plevel_status, 
                                        MOBLEUINT32 plength)
{
#ifdef CFG_DEBUG_TRACE
  MOBLEUINT8 i;
  MOBLEUINT8 level = 0;
#endif  
  
  TRACE_M(TF_GENERIC,"Generic_Level_Status callback received \r\n");
  
  TRACE_M(TF_SERIAL_CTRL,"#8208! \n\r");
      
#ifdef CFG_DEBUG_TRACE
  for(i = 0; i < plength; i++)
  {
    if((i == 0) || (i == 2))
      level = plevel_status[i];
    else if(i == 1)
      TRACE_M(TF_GENERIC,"Present Level value: %d\n\r", 
              (plevel_status[i]<<8)|level);
    else if(i == 3)
      TRACE_M(TF_GENERIC,"Target Level value: %d\n\r", 
              (plevel_status[i]<<8)|level);
    else if(i == 4)
      TRACE_M(TF_GENERIC,"Remaining Time value: %d\n\r", plevel_status[i]);
  }
#endif  
  return MOBLE_RESULT_SUCCESS;
}

 
/******************************************************************************/
#ifdef ENABLE_GENERIC_MODEL_SERVER_POWER_ONOFF
/******************************************************************************/
 
/**
* @brief  Appli_Generic_PowerOnOff_Set: This function is callback for Application
*           when Generic Power on off set message is received
* @param  pPowerOnOffParam: Pointer to the parameters message
* @param  OptionalValid: Flag to inform about the validity of optional parameters 
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Generic_PowerOnOff_Set(Generic_PowerOnOffParam_t* pPowerOnOffParam, 
                                         MOBLEUINT8 OptionalValid)
{ 
  AppliPowerOnSet.PowerOnState = pPowerOnOffParam->PowerOnOffState;
  
  TRACE_M(TF_SERIAL_CTRL,"#8213!\n\r");

  /* set the flag value for NVM store */
  //RestoreFlag = GENERIC_ON_OFF_NVM_FLAG;

  AppliNvm_SaveMessageParam();
    
  return MOBLE_RESULT_SUCCESS;
}

/******************************************************************************/
#endif  /* ENABLE_GENERIC_MODEL_SERVER_POWER_ONOFF */
/******************************************************************************/

/**
* @brief  Appli_Generic_PowerOnOff_Set: This function is callback for Application
*           when Generic Power on off set message is received
* @param   powerOnOff_status: Pointer to the parameters message
* @param  plength: length of data 
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Generic_PowerOnOff_Status(MOBLEUINT8 const *powerOnOff_status , 
                                             MOBLEUINT32 plength) 
{  
  MOBLEUINT8 i;

  TRACE_M(TF_GENERIC,"Generic_PowerOnOff_Status callback received \r\n"); 
  
  TRACE_M(TF_SERIAL_CTRL,"#8212! \n\r");

  for(i = 0; i < plength; i++)
  {
    if(i == 0)
      TRACE_M(TF_SERIAL_CTRL,"On Power up value: %d\n\r", 
              powerOnOff_status[i]);
  }

  return MOBLE_RESULT_SUCCESS;
}


/******************************************************************************/
#ifdef ENABLE_GENERIC_MODEL_SERVER_DEFAULT_TRANSITION_TIME
/******************************************************************************/

/**
* @brief  Appli_Generic_DefaultTransitionTime_Set: This function is callback for Application
*          when Generic Power on off set message is received
* @param  pDefaultTimeParam: Pointer to the parameters message
* @param  OptionalValid: Flag to inform about the validity of optional parameters 
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Generic_DefaultTransitionTime_Set(Generic_DefaultTransitionParam_t* pDefaultTimeParam, 
                                         MOBLEUINT8 OptionalValid)
{
  
  AppliDefaultTransitionSet.DefaultTransitionTime = pDefaultTimeParam->DefaultTransitionTime;
  
  return MOBLE_RESULT_SUCCESS;
}


/******************************************************************************/
#endif   /* ENABLE_GENERIC_MODEL_SERVER_DEFAULT_TRANSITION_TIME */
/******************************************************************************/


/**
* @brief  Appli_Generic_DefaultTransitionTime_Status: This function is callback for Application
*          when Generic Power on off set message is received
* @param  pTransition_status: Pointer to the parameters message
* @param  plength: length of data 
* @retval MOBLE_RESULT
*/
MOBLE_RESULT Appli_Generic_DefaultTransitionTime_Status(MOBLEUINT8 const *pTransition_status , 
                                                        MOBLEUINT32 plength) 
{  

  TRACE_M(TF_GENERIC,"Generic_DefaultTransitionTime_Status callback received \r\n");
  
  TRACE_M(TF_SERIAL_CTRL,"#8210! \n\r");
  
  return MOBLE_RESULT_SUCCESS;
}



/**
* @brief  Appli_Generic_GetOnOffState: This function is callback for Application
*          when Generic on off status message is to be provided
* @param  pOnOff_status: Pointer to the status message
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Generic_GetOnOffStatus(MOBLEUINT8* pOnOff_Status)                                        
{
  
  *pOnOff_Status = AppliOnOffSet.Present_OnOff;
  TRACE_M(TF_SERIAL_CTRL,"Generic Get OnOff Status: Status %d!\n\r",
          AppliOnOffSet.Present_OnOff);

  return MOBLE_RESULT_SUCCESS; 
}

/**
* @brief  Appli_Generic_GetOnOffValue: This function is callback for Application
          to get the PWM value for the generic on off
* @param  pOnOff_Value: Pointer to the status message
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Generic_GetOnOffValue(MOBLEUINT8* pOnOff_Value)                                        
{
  
  *pOnOff_Value = AppliOnOffSet.Present_OnOffValue;
  *(pOnOff_Value+1) = AppliOnOffSet.Present_OnOffValue >> 8;
  *(pOnOff_Value+2) = AppliOnOffSet.TargetValue;
  *(pOnOff_Value+3) = AppliOnOffSet.TargetValue >> 8;
  TRACE_M(TF_SERIAL_CTRL,"Generic Get OnOff Value: Value %d!\n\r",
          AppliOnOffSet.Present_OnOffValue);
  return MOBLE_RESULT_SUCCESS; 
}

/**
* @brief  Appli_Generic_GetLevelStatus: This function is callback for Application
           when Generic Level status message is to be provided
* @param  pLevel_status: Pointer to the status message
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Generic_GetLevelStatus(MOBLEUINT8* pLevel_Status) 
{ 
   *pLevel_Status = AppliLevelSet.Present_Level16;
   *(pLevel_Status+1) = AppliLevelSet.Present_Level16 >> 8;
   *(pLevel_Status+2) = AppliLevelSet.Target_Level16;
   *(pLevel_Status+3) = AppliLevelSet.Target_Level16 >> 8;
//  TRACE_M(TF_SERIAL_CTRL,"Generic Get Level Status: Value %d!\n\r",
//          *pLevel_Status);
  return MOBLE_RESULT_SUCCESS; 
}

/**
* @brief  Appli_Generic_GetPowerOnOffStatus: This function is callback for Application
*          when Generic Get Power status message is to be provided
* @param  pLevel_status: Pointer to the status message
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Generic_GetPowerOnOffStatus(MOBLEUINT8* pPower_Status) 
{ 
  *pPower_Status = AppliPowerOnSet.PowerOnState;
  TRACE_M(TF_SERIAL_CTRL,"Generic Get OnOff Status: Status %d!\n\r",
          AppliPowerOnSet.PowerOnState);
  
  return MOBLE_RESULT_SUCCESS; 
}

/**
* @brief  Appli_Generic_GetDefaultTransitionStatus: This function is callback for 
*           Application when Generic Level status message is to be provided
* @param  pTransition_Status: Pointer to the status message
* @retval MOBLE_RESULT
*/ 
MOBLE_RESULT Appli_Generic_GetDefaultTransitionStatus(MOBLEUINT8* pTransition_Status) 
{ 
  *pTransition_Status = AppliDefaultTransitionSet.DefaultTransitionTime;
  TRACE_M(TF_SERIAL_CTRL,"Get Default Transition Status: Status %d!\n\r",
          AppliDefaultTransitionSet.DefaultTransitionTime);
  
  return MOBLE_RESULT_SUCCESS; 
}

/**
 * @}
 */

/**
 * @}
 */

/******************* (C) COPYRIGHT 2019 STMicroelectronics *****END OF FILE****/

