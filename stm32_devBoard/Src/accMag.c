/**
  ******************************************************************************
  * File Name          : accMag.c
  * Description        : This file provides code for the LSM303 Acceleromter + Magnetometer
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "accMag.h"
#include "gpio.h"
#include "stm32wbxx_hal.h"
#include "spi.h"
#include "i2c.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"

/*
 * DEFINES
 */

static float _lsm303Accel_MG_LSB     = 0.001F;   // 1, 2, 4 or 12 mg per lsb
static float _lsm303Mag_Gauss_LSB_XY = 1100.0F;  // Varies with gain
static float _lsm303Mag_Gauss_LSB_Z  = 980.0F;   // Varies with gain
/*
 * THREADS
 */

void accMagThread(void){
  uint8_t acc_sample[6] = {0};
  setupAcc();

  while(1){
    sampleAcc(acc_sample);
    osDelay(100);
  }
}

/*
 * ACCELEROMETER HELPER FUNCTIONS
 */

//bool initAcc(void){
//
//  // Enable the accelerometer at 100Hz
//  LSM303_writeRegister8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A, 0x57);
//
//  // LSM303DLHC has no WHOAMI register so read CTRL_REG1_A back to check
//  // if we are connected or not
//  uint8_t reg1_a = read8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A);
//  if (reg1_a != 0x57)
//  {
//    return false;
//  }
//
//  return true;
//}

void setupAcc(void){
  // data rate : 25 Hz
  uint8_t packet = ACC_SAMPLE_RATE_25 | ACC_X_EN | ACC_Z_EN | ACC_Y_EN;
  LSM303_writeRegister8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A, &packet);

  bool temp_val = false;
  uint8_t reg1_a = 0;
  LSM303_readRegister8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A, &reg1_a);

  if (reg1_a != 0x57)
  {
    bool temp_val = true;
  }
}

void sampleAcc(uint8_t *sample){
  // data rate : 25 Hz
  HAL_I2C_Mem_Read_IT(&hi2c1, LSM303_ADDRESS_ACCEL << 1, LSM303_REGISTER_MAG_OUT_X_H_M, 1, sample, 6);
}

/*
 * MAGNETOMETER HELPER FUNCTIONS
 */

bool initMag(void){
  // Enable the magnetometer
  LSM303_writeRegister8(LSM303_ADDRESS_MAG, LSM303_REGISTER_MAG_MR_REG_M, 0x00);

  // LSM303DLHC has no WHOAMI register so read CRA_REG_M to check
  // the default value (0b00010000/0x10)
  uint8_t reg1_a = 0;
  LSM303_readRegister8(LSM303_ADDRESS_MAG, LSM303_REGISTER_MAG_CRA_REG_M, &reg1_a);
  if (reg1_a != 0x10)
  {
    return false;
  }

  // Set the gain to a known level
  setMagGain(LSM303_MAGGAIN_1_3);

  return true;
}

void setMagGain(lsm303MagGain gain)
{
  LSM303_writeRegister8(LSM303_ADDRESS_MAG, LSM303_REGISTER_MAG_CRB_REG_M, (uint8_t)gain);

//  magGain = gain;

  switch(gain)
  {
    case LSM303_MAGGAIN_1_3:
      _lsm303Mag_Gauss_LSB_XY = 1100;
      _lsm303Mag_Gauss_LSB_Z  = 980;
      break;
    case LSM303_MAGGAIN_1_9:
      _lsm303Mag_Gauss_LSB_XY = 855;
      _lsm303Mag_Gauss_LSB_Z  = 760;
      break;
    case LSM303_MAGGAIN_2_5:
      _lsm303Mag_Gauss_LSB_XY = 670;
      _lsm303Mag_Gauss_LSB_Z  = 600;
      break;
    case LSM303_MAGGAIN_4_0:
      _lsm303Mag_Gauss_LSB_XY = 450;
      _lsm303Mag_Gauss_LSB_Z  = 400;
      break;
    case LSM303_MAGGAIN_4_7:
      _lsm303Mag_Gauss_LSB_XY = 400;
      _lsm303Mag_Gauss_LSB_Z  = 355;
      break;
    case LSM303_MAGGAIN_5_6:
      _lsm303Mag_Gauss_LSB_XY = 330;
      _lsm303Mag_Gauss_LSB_Z  = 295;
      break;
    case LSM303_MAGGAIN_8_1:
      _lsm303Mag_Gauss_LSB_XY = 230;
      _lsm303Mag_Gauss_LSB_Z  = 205;
      break;
  }
}

/*
 * Helper Functions
 */

void LSM303_writeRegister8(uint8_t addr, uint8_t reg, uint8_t value) {
//  addr = 0x48;
  while(HAL_I2C_Mem_Write(&hi2c1, addr<<1, reg, 1, &value, 1, 1000) != HAL_OK);
}

void LSM303_readRegister8(uint8_t addr, uint8_t reg, uint8_t *packet) {
  HAL_I2C_Mem_Read(&hi2c1, addr<<1, reg, 1, packet, 1, 10000);
}


