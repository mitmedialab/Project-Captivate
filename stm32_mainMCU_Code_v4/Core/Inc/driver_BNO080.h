/**
 ******************************************************************************
 * File Name          : driver_BNO080.h
  * Description        : Header for Lights.
  ******************************************************************************

  *
  ******************************************************************************
 */
#ifndef DRIVER_BNO080_H
#define DRIVER_BNO080_H



#ifdef __cplusplus
extern "C" {
#endif

/* includes -----------------------------------------------------------*/
#include "stdint.h"
#include "cmsis_os.h"
#include "stm32wbxx_hal.h"
#include "driver_BNO080.h"
#include <stdbool.h>
/* typedef -----------------------------------------------------------*/


/* defines -----------------------------------------------------------*/
//0x4B is also possible.
#define BNO080_ADDRESS 0x4A

//Platform specific configurations
#define I2C_BUFFER_LENGTH 32

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//All the ways we can configure or talk to the BNO080, figure 34, page 36 reference manual
//These are used for low level communication with the sensor, on channel 2
#define SHTP_REPORT_COMMAND_RESPONSE 0xF1
#define SHTP_REPORT_COMMAND_REQUEST 0xF2
#define SHTP_REPORT_FRS_READ_RESPONSE 0xF3
#define SHTP_REPORT_FRS_READ_REQUEST 0xF4
#define SHTP_REPORT_PRODUCT_ID_RESPONSE 0xF8
#define SHTP_REPORT_PRODUCT_ID_REQUEST 0xF9
#define SHTP_REPORT_BASE_TIMESTAMP 0xFB
#define SHTP_REPORT_SET_FEATURE_COMMAND 0xFD

//All the different sensors and features we can get reports from
//These are used when enabling a given sensor
#define SENSOR_REPORTID_ACCELEROMETER 0x01
#define SENSOR_REPORTID_GYROSCOPE 0x02
#define SENSOR_REPORTID_MAGNETIC_FIELD 0x03
#define SENSOR_REPORTID_LINEAR_ACCELERATION 0x04
#define SENSOR_REPORTID_ROTATION_VECTOR 0x05
#define SENSOR_REPORTID_GRAVITY 0x06
#define SENSOR_REPORTID_GAME_ROTATION_VECTOR 0x08
#define SENSOR_REPORTID_GEOMAGNETIC_ROTATION_VECTOR 0x09
#define SENSOR_REPORTID_TAP_DETECTOR 0x10
#define SENSOR_REPORTID_STEP_COUNTER 0x11
#define SENSOR_REPORTID_STABILITY_CLASSIFIER 0x13
#define SENSOR_REPORTID_RAW_ACCELEROMETER 0x14
#define SENSOR_REPORTID_RAW_GYROSCOPE 0x15
#define SENSOR_REPORTID_RAW_MAGNETOMETER 0x16
#define SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER 0x1E

//Record IDs from figure 29, page 29 reference manual
//These are used to read the metadata for each sensor type
#define FRS_RECORDID_ACCELEROMETER 0xE302
#define FRS_RECORDID_GYROSCOPE_CALIBRATED 0xE306
#define FRS_RECORDID_MAGNETIC_FIELD_CALIBRATED 0xE309
#define FRS_RECORDID_ROTATION_VECTOR 0xE30B

//Command IDs from section 6.4, page 42
//These are used to calibrate, initialize, set orientation, tare etc the sensor
#define COMMAND_ERRORS 1
#define COMMAND_COUNTER 2
#define COMMAND_TARE 3
#define COMMAND_INITIALIZE 4
#define COMMAND_DCD 6
#define COMMAND_ME_CALIBRATE 7
#define COMMAND_DCD_PERIOD_SAVE 9
#define COMMAND_OSCILLATOR 10
#define COMMAND_CLEAR_DCD 11

#define CALIBRATE_ACCEL 0
#define CALIBRATE_GYRO 1
#define CALIBRATE_MAG 2
#define CALIBRATE_PLANAR_ACCEL 3
#define CALIBRATE_ACCEL_GYRO_MAG 4
#define CALIBRATE_STOP 5

#define MAX_PACKET_SIZE 128 //Packets can be up to 32k but we don't have that much RAM.
#define MAX_METADATA_SIZE 9 //This is in words. There can be many but we mostly only care about the first 9 (Qs, range, etc)

//class BNO080
//{
//public:
bool IMU_begin(uint8_t deviceAddress, uint8_t intPin, GPIO_TypeDef* intPort); //By default use the default I2C addres, and use Wire port, and don't declare an INT pin
bool IMU_beginSPI(uint8_t user_CSPin, uint8_t user_WAKPin, uint8_t user_INTPin, uint8_t user_RSTPin, uint32_t spiPortSpeed);

//	void enableDebugging(Stream &debugPort = Serial); //Turn on debug printing. If user doesn't specify then Serial will be used.

void IMU_softReset();	  //Try to reset the IMU via software
uint8_t IMU_resetReason(); //Query the IMU for the reason it last reset

float IMU_qToFloat(int16_t fixedPointValue, uint8_t qPoint); //Given a Q value, converts fixed point floating to regular floating point number

bool IMU_waitForI2C(); //Delay based polling for I2C traffic
bool IMU_waitForSPI(); //Delay based polling for INT pin to go low
bool IMU_receivePacket(void);
bool IMU_getData(uint16_t bytesRemaining); //Given a number of bytes, send the requests in I2C_BUFFER_LENGTH chunks
bool IMU_sendPacket(uint8_t channelNumber, uint8_t dataLength);
void IMU_printPacket(void); //Prints the current shtp header and data packets

void IMU_enableRotationVector(uint16_t timeBetweenReports);
void IMU_enableGameRotationVector(uint16_t timeBetweenReports);
void IMU_enableAccelerometer(uint16_t timeBetweenReports);
void IMU_enableLinearAccelerometer(uint16_t timeBetweenReports);
void IMU_enableGyro(uint16_t timeBetweenReports);
void IMU_enableMagnetometer(uint16_t timeBetweenReports);
void IMU_enableStepCounter(uint16_t timeBetweenReports);
void IMU_enableStabilityClassifier(uint16_t timeBetweenReports);
void IMU_enableActivityClassifier(uint16_t timeBetweenReports, uint32_t activitiesToEnable, uint8_t* activityConfidences);
void IMU_enableRawAccelerometer(uint16_t timeBetweenReports);
void IMU_enableRawGyro(uint16_t timeBetweenReports);
void IMU_enableRawMagnetometer(uint16_t timeBetweenReports);

bool IMU_dataAvailable(void);
void IMU_parseInputReport(void);   //Parse sensor readings out of report
void IMU_parseCommandReport(void); //Parse command responses out of report

float IMU_getQuatI();
float IMU_getQuatJ();
float IMU_getQuatK();
float IMU_getQuatReal();
float IMU_getQuatRadianAccuracy();
uint8_t IMU_getQuatAccuracy();

float IMU_getAccelX();
float IMU_getAccelY();
float IMU_getAccelZ();
uint8_t IMU_getAccelAccuracy();

float IMU_getLinAccelX();
float IMU_getLinAccelY();
float IMU_getLinAccelZ();
uint8_t IMU_getLinAccelAccuracy();

float IMU_getGyroX();
float IMU_getGyroY();
float IMU_getGyroZ();
uint8_t IMU_getGyroAccuracy();

float IMU_getMagX();
float IMU_getMagY();
float IMU_getMagZ();
uint8_t IMU_getMagAccuracy();

void IMU_calibrateAccelerometer();
void IMU_calibrateGyro();
void IMU_calibrateMagnetometer();
void IMU_calibratePlanarAccelerometer();
void IMU_calibrateAll();
void IMU_endCalibration();
void IMU_saveCalibration();
void IMU_requestCalibrationStatus(); //Sends command to get status
bool IMU_calibrationComplete();   //Checks ME Cal response for byte 5, R0 - Status

uint32_t IMU_getTimeStamp();
uint16_t IMU_getStepCount();
uint8_t IMU_getStabilityClassifier();
uint8_t IMU_getActivityClassifier();

int16_t IMU_getRawAccelX();
int16_t IMU_getRawAccelY();
int16_t IMU_getRawAccelZ();

int16_t IMU_getRawGyroX();
int16_t IMU_getRawGyroY();
int16_t IMU_getRawGyroZ();

int16_t IMU_getRawMagX();
int16_t IMU_getRawMagY();
int16_t IMU_getRawMagZ();

void IMU_sendTareNow(void);
void IMU_sendPersistTare(void);

void IMU_setFeatureCommand_2(uint8_t reportID, uint16_t timeBetweenReports);
void IMU_setFeatureCommand_3(uint8_t reportID, uint16_t timeBetweenReports, uint32_t specificConfig);
void IMU_sendCommand(uint8_t command);
void IMU_sendCalibrateCommand(uint8_t thingToCalibrate);

//Metadata functions
int16_t IMU_getQ1(uint16_t recordID);
int16_t IMU_getQ2(uint16_t recordID);
int16_t IMU_getQ3(uint16_t recordID);
float IMU_getResolution(uint16_t recordID);
float IMU_getRange(uint16_t recordID);
uint32_t IMU_readFRSword(uint16_t recordID, uint8_t wordNumber);
void IMU_frsReadRequest(uint16_t recordID, uint16_t readOffset, uint16_t blockSize);
bool IMU_readFRSdata(uint16_t recordID, uint8_t startLocation, uint8_t wordsToRead);



//};

/* macros ------------------------------------------------------------*/


/* function prototypes -----------------------------------------------*/


/* variables -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/
//void InertialSensingTask(void);
//void Setup_BNO080(void);



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

/*************************************************************
 *
 * FREERTOS WRAPPER FUNCTIONS
 *
*************************************************************/





#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
