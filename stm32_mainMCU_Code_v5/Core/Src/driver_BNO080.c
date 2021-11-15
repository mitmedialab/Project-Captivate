/**
 ******************************************************************************
 * File Name           : driver_BNO080.hpp
 * Description        : Header for Lights.
 ******************************************************************************

 *
 ******************************************************************************
 */

/* includes -----------------------------------------------------------*/
#include <driver_BNO080.h>
#include "math.h"
#include "stm32wbxx_hal.h"
#include "i2c.h"
#include "stdint.h"
#include <string.h>
#include "inertial_sensing.h"
#include "inter_processor_comms.h"
#include "master_thread.h"
#include "task.h"
//#include "cmsis_os2.h"

#include "captivate_config.h"

/* typedef -----------------------------------------------------------*/

/* defines -----------------------------------------------------------*/
#define PACKET_LSB		0
#define PACKET_MSB		1
#define CHANNEL_NUM		2
#define SEQUENCE_NUM	3

/* macros ------------------------------------------------------------*/

/* function prototypes -----------------------------------------------*/

/* variables -----------------------------------------------*/
struct stepData stepSample;
struct stabilityData stabilitySample;
struct activityData activitySample;
struct rotationData rotSample;
//struct genericThreeAxisData accSample[ACC_GYRO_QUEUE_SIZE][ACC_GYRO_PACKET_SIZE];
//struct genericThreeAxisData gyroSample[ACC_GYRO_QUEUE_SIZE][ACC_GYRO_PACKET_SIZE];

GenericThreeAxisPayload accSample[ACC_GYRO_QUEUE_SIZE];
GenericThreeAxisPayload gyroSample[ACC_GYRO_QUEUE_SIZE];


struct genericThreeAxisData *accSamplePtr;
struct genericThreeAxisData *gyroSamplePtr;


volatile uint16_t accPacketIdx = 0;
volatile uint16_t gyroPacketIdx = 0;

volatile uint16_t accQueueIdx = 0;
volatile uint16_t gyroQueueIdx = 0;



//Registers
const uint8_t CHANNEL_COMMAND = 0;
const uint8_t CHANNEL_EXECUTABLE = 1;
const uint8_t CHANNEL_CONTROL = 2;
const uint8_t CHANNEL_REPORTS = 3;
const uint8_t CHANNEL_WAKE_REPORTS = 4;
const uint8_t CHANNEL_GYRO = 5;

//Global Variables
uint8_t shtpHeader[4]; //Each packet has a header of 4 bytes
uint8_t shtpData[MAX_PACKET_SIZE];
uint8_t sequenceNumber[6] = { 0, 0, 0, 0, 0, 0 }; //There are 6 com channels. Each channel has its own seqnum
uint8_t commandSequenceNumber = 0;//Commands have a seqNum as well. These are inside command packet, the header uses its own seqNum per channel
uint32_t metaData[MAX_METADATA_SIZE];//There is more than 10 words in a metadata record but we'll stop at Q point 3

uint8_t receiveBuffer[I2C_BUFFER_LENGTH] = { 0 };

//These Q values are defined in the datasheet but can also be obtained by querying the meta data records
//See the read metadata example for more info
int16_t rotationVector_Q1 = 14;
int16_t accelerometer_Q1 = 8;
int16_t linear_accelerometer_Q1 = 8;
int16_t gyro_Q1 = 9;
int16_t magnetometer_Q1 = 4;

uint8_t outPacket[32];

//private:
//Variables
//	TwoWire *_i2cPort;		//The generic connection to user's chosen I2C hardware
uint8_t _deviceAddress = 0x4A << 1; //Keeps track of I2C address. setI2CAddress changes this.

//	Stream *_debugPort;			 //The stream to send debug messages to if enabled. Usually Serial.
//bool _printDebug = False; //Flag to print debugging variables

//	SPIClass *_spiPort;			 //The generic connection to user's chosen SPI hardware
unsigned long _spiPortSpeed; //Optional user defined port speed
uint8_t _cs;				 //Pins needed for SPI
uint8_t _wake;
uint8_t _int;
GPIO_TypeDef *_intPort;
uint8_t _rst;

//These are the raw sensor values (without Q applied) pulled from the user requested Input Report
uint16_t rawAccelX, rawAccelY, rawAccelZ, accelAccuracy;
uint16_t rawLinAccelX, rawLinAccelY, rawLinAccelZ, accelLinAccuracy;
uint16_t rawGyroX, rawGyroY, rawGyroZ, gyroAccuracy;
uint16_t rawMagX, rawMagY, rawMagZ, magAccuracy;
uint16_t rawQuatI, rawQuatJ, rawQuatK, rawQuatReal, rawQuatRadianAccuracy,
		quatAccuracy;
uint16_t stepCount;
uint32_t timeStamp;
uint8_t stabilityClassifier;
uint8_t activityClassifier;
uint8_t *_activityConfidences;//Array that store the confidences of the 9 possible activities
uint8_t calibrationStatus;					//Byte R0 of ME Calibration Response
uint16_t memsRawAccelX, memsRawAccelY, memsRawAccelZ; //Raw readings from MEMS sensor
uint16_t memsRawGyroX, memsRawGyroY, memsRawGyroZ;//Raw readings from MEMS sensor
uint16_t memsRawMagX, memsRawMagY, memsRawMagZ;	//Raw readings from MEMS sensor

/* Functions Definition ------------------------------------------------------*/
//Attempt communication with the device
//Return true if we got a 'Polo' back from Marco
bool IMU_begin(uint8_t deviceAddress, uint8_t intPin, GPIO_TypeDef *intPort) {
	_deviceAddress = deviceAddress << 1; //If provided, store the I2C address from user
//	_i2cPort = &wirePort;			//Grab which port the user wants us to use
	_int = intPin;//Get the pin that the user wants to use for interrupts. By default, it's NULL and we'll not use it in dataAvailable() function.
	_intPort = intPort;

	//We expect caller to begin their I2C port, with the speed of their choice external to the library
	//But if they forget, we start the hardware here.
	//_i2cPort->begin();

	//Begin by resetting the IMU
	IMU_softReset();

	//Check communication with device
	shtpData[0] = SHTP_REPORT_PRODUCT_ID_REQUEST; //Request the product ID and reset info
	shtpData[1] = 0;							  //Reserved

	//Transmit packet on channel 2, 2 bytes
	IMU_sendPacket(CHANNEL_CONTROL, 2);

	//Now we wait for response
	if (IMU_receivePacket() == true) {
		if (shtpData[0] == SHTP_REPORT_PRODUCT_ID_RESPONSE) {
			return (true);
		}
	}

	return (false); //Something went wrong
}

//bool IMU_beginSPI(uint8_t user_CSPin, uint8_t user_WAKPin, uint8_t user_INTPin, uint8_t user_RSTPin, uint32_t spiPortSpeed)
//{
////	_i2cPort = NULL; //This null tells the send/receive functions to use SPI
////
////	//Get user settings
////	_spiPort = &spiPort;
////	_spiPortSpeed = spiPortSpeed;
////	if (_spiPortSpeed > 3000000)
////		_spiPortSpeed = 3000000; //BNO080 max is 3MHz
////
////	_cs = user_CSPin;
////	_wake = user_WAKPin;
////	_int = user_INTPin;
////	_rst = user_RSTPin;
////
////	pinMode(_cs, OUTPUT);
////	pinMode(_wake, OUTPUT);
////	pinMode(_int, INPUT_PULLUP);
////	pinMode(_rst, OUTPUT);
////
////	digitalWrite(_cs, HIGH); //Deselect BNO080
////
////	//Configure the BNO080 for SPI communication
////	digitalWrite(_wake, HIGH); //Before boot up the PS0/WAK pin must be high to enter SPI mode
////	digitalWrite(_rst, LOW);   //Reset BNO080
////	delay(2);				   //Min length not specified in datasheet?
////	digitalWrite(_rst, HIGH);  //Bring out of reset
////
////	//Wait for first assertion of INT before using WAK pin. Can take ~104ms
////	waitForSPI();
////
////	//if(wakeBNO080() == false) //Bring IC out of sleep after reset
////	//  Serial.println("BNO080 did not wake up");
////
////	_spiPort->begin(); //Turn on SPI hardware
////
////	//At system startup, the hub must send its full advertisement message (see 5.2 and 5.3) to the
////	//host. It must not send any other data until this step is complete.
////	//When BNO080 first boots it broadcasts big startup packet
////	//Read it and dump it
////	waitForSPI(); //Wait for assertion of INT before reading advert message.
////	receivePacket();
////
////	//The BNO080 will then transmit an unsolicited Initialize Response (see 6.4.5.2)
////	//Read it and dump it
////	waitForSPI(); //Wait for assertion of INT before reading Init response
////	receivePacket();
////
////	//Check communication with device
////	shtpData[0] = SHTP_REPORT_PRODUCT_ID_REQUEST; //Request the product ID and reset info
////	shtpData[1] = 0;							  //Reserved
////
////	//Transmit packet on channel 2, 2 bytes
////	sendPacket(CHANNEL_CONTROL, 2);
////
////	//Now we wait for response
////	waitForSPI();
////	if (receivePacket() == true)
////	{
////		if (shtpData[0] == SHTP_REPORT_PRODUCT_ID_RESPONSE)
////			return (true);
////	}
//
//	return (false); //Something went wrong
//}

//Calling this function with nothing sets the debug port to Serial
//You can also call it with other streams like Serial1, SerialUSB, etc.
//void IMU_enableDebugging(Stream &debugPort)
//{
////	_debugPort = &debugPort;
////	_printDebug = true;
//}

//Updates the latest variables if possible
//Returns false if new readings are not available
bool IMU_dataAvailable(void) {
	//If we have an interrupt pin connection available, check if data is available.
	//If int pin is not set, then we'll rely on receivePacket() to timeout
	//See issue 13: https://github.com/sparkfun/SparkFun_BNO080_Arduino_Library/issues/13
	if (_int != 255) {
		if (HAL_GPIO_ReadPin(_intPort, _int) == GPIO_PIN_SET)
			return (false);
//		if (digitalRead(_int) == HIGH)
//			return (false);
	}

//	if(HAL_GPIO_ReadPin(_intPort, _int) == GPIO_PIN_SET)
//	{
	if (IMU_receivePacket() == true) {
		//Check to see if this packet is a sensor reporting its data to us
		if (shtpHeader[2]
				== CHANNEL_REPORTS&& shtpData[0] == SHTP_REPORT_BASE_TIMESTAMP) {
			IMU_parseInputReport(); //This will update the rawAccelX, etc variables depending on which feature report is found
			return (true);
		} else if (shtpHeader[2] == CHANNEL_CONTROL) {
			IMU_parseCommandReport(); //This will update responses to commands, calibrationStatus, etc.
			return (true);
		}
	}
//	}
	return (false);
}

//This function pulls the data from the command response report

//Unit responds with packet that contains the following:
//shtpHeader[0:3]: First, a 4 byte header
//shtpData[0]: The Report ID
//shtpData[1]: Sequence number (See 6.5.18.2)
//shtpData[2]: Command
//shtpData[3]: Command Sequence Number
//shtpData[4]: Response Sequence Number
//shtpData[5 + 0]: R0
//shtpData[5 + 1]: R1
//shtpData[5 + 2]: R2
//shtpData[5 + 3]: R3
//shtpData[5 + 4]: R4
//shtpData[5 + 5]: R5
//shtpData[5 + 6]: R6
//shtpData[5 + 7]: R7
//shtpData[5 + 8]: R8
void IMU_parseCommandReport(void) {
	if (shtpData[0] == SHTP_REPORT_COMMAND_RESPONSE) {
		//The BNO080 responds with this report to command requests. It's up to use to remember which command we issued.
		uint8_t command = shtpData[2]; //This is the Command byte of the response

		if (command == COMMAND_ME_CALIBRATE) {
			calibrationStatus = shtpData[5 + 0]; //R0 - Status (0 = success, non-zero = fail)
		}
	} else {
		//This sensor report ID is unhandled.
		//See reference manual to add additional feature reports as needed
	}

	//TODO additional feature reports may be strung together. Parse them all.
}

//This function pulls the data from the input report
//The input reports vary in length so this function stores the various 16-bit values as globals

//Unit responds with packet that contains the following:
//shtpHeader[0:3]: First, a 4 byte header
//shtpData[0:4]: Then a 5 byte timestamp of microsecond clicks since reading was taken
//shtpData[5 + 0]: Then a feature report ID (0x01 for Accel, 0x05 for Rotation Vector)
//shtpData[5 + 1]: Sequence number (See 6.5.18.2)
//shtpData[5 + 2]: Status
//shtpData[3]: Delay
//shtpData[4:5]: i/accel x/gyro x/etc
//shtpData[6:7]: j/accel y/gyro y/etc
//shtpData[8:9]: k/accel z/gyro z/etc
//shtpData[10:11]: real/gyro temp/etc
//shtpData[12:13]: Accuracy estimate
void IMU_parseInputReport(void) {
	//Calculate the number of data bytes in this packet
	int16_t dataLength = ((uint16_t) shtpHeader[1] << 8 | shtpHeader[0]);
	dataLength &= ~(1 << 15); //Clear the MSbit. This bit indicates if this package is a continuation of the last.
	//Ignore it for now. TODO catch this as an error and exit

	dataLength -= 4; //Remove the header bytes from the data count

	timeStamp = ((uint32_t) shtpData[4] << (8 * 3))
			| ((uint32_t) shtpData[3] << (8 * 2))
			| ((uint32_t) shtpData[2] << (8 * 1))
			| ((uint32_t) shtpData[1] << (8 * 0));

	uint8_t status = shtpData[5 + 2] & 0x03; //Get status bits
	uint16_t data1 = (uint16_t) shtpData[5 + 5] << 8 | shtpData[5 + 4];
	uint16_t data2 = (uint16_t) shtpData[5 + 7] << 8 | shtpData[5 + 6];
	uint16_t data3 = (uint16_t) shtpData[5 + 9] << 8 | shtpData[5 + 8];
	uint16_t data4 = 0;
	uint16_t data5 = 0; //We would need to change this to uin32_t to capture time stamp value on Raw Accel/Gyro/Mag reports

	if (dataLength - 5 > 9) {
		data4 = (uint16_t) shtpData[5 + 11] << 8 | shtpData[5 + 10];
	}
	if (dataLength - 5 > 11) {
		data5 = (uint16_t) shtpData[5 + 13] << 8 | shtpData[5 + 12];
	}

	//Store these generic values to their proper global variable
	if (shtpData[5] == SENSOR_REPORTID_ACCELEROMETER) {
		accelAccuracy = status;
		rawAccelX = data1;
		rawAccelY = data2;
		rawAccelZ = data3;
	} else if (shtpData[5] == SENSOR_REPORTID_LINEAR_ACCELERATION) {


		accelLinAccuracy = status;
		rawLinAccelX = data1;
		rawLinAccelY = data2;
		rawLinAccelZ = data3;
	} else if (shtpData[5] == SENSOR_REPORTID_GYROSCOPE) {
		gyroAccuracy = status;
		rawGyroX = data1;
		rawGyroY = data2;
		rawGyroZ = data3;
	} else if (shtpData[5] == SENSOR_REPORTID_GYROSCOPE_UNCAL) {


		gyroAccuracy = status;
		rawGyroX = data1;
		rawGyroY = data2;
		rawGyroZ = data3;

	} else if (shtpData[5] == SENSOR_REPORTID_MAGNETIC_FIELD) {
		magAccuracy = status;
		rawMagX = data1;
		rawMagY = data2;
		rawMagZ = data3;
	} else if (shtpData[5] == SENSOR_REPORTID_ROTATION_VECTOR
			|| shtpData[5] == SENSOR_REPORTID_GAME_ROTATION_VECTOR) {
		// put rotation sample in queue for message packing
		rotSample.tick_ms = HAL_GetTick();
		rotSample.quatI = IMU_qToFloat(data1, rotationVector_Q1);
		;
		rotSample.quatJ = IMU_qToFloat(data2, rotationVector_Q1);
		;
		rotSample.quatK = IMU_qToFloat(data3, rotationVector_Q1);
		;
		rotSample.quatReal = IMU_qToFloat(data4, rotationVector_Q1);
		rotSample.quatRadianAccuracy = IMU_qToFloat(data5, 12);

		osMessageQueuePut(rotationSampleQueueHandle, &rotSample, 0U, 0);

		quatAccuracy = status;
		rawQuatI = data1;
		rawQuatJ = data2;
		rawQuatK = data3;
		rawQuatReal = data4;
		rawQuatRadianAccuracy = data5; //Only available on rotation vector, not game rot vector
	} else if (shtpData[5] == SENSOR_REPORTID_STEP_COUNTER) {
		// put step cnt sample in queue for message packing
		stepSample.tick_ms = HAL_GetTick();
		stepSample.stepCount = data3;

//		osMessageQueuePut(stepSampleQueueHandle, &stepSample, 0U, osWaitForever);

		stepCount = data3; //Bytes 8/9
	} else if (shtpData[5] == SENSOR_REPORTID_STABILITY_CLASSIFIER) {

		// put stability sample in queue for message packing
		stabilitySample.tick_ms = HAL_GetTick();
		stabilitySample.stabilityClass = shtpData[5 + 4];

//		osMessageQueuePut(stabilitySampleQueueHandle, &stabilitySample, 0U, osWaitForever);

		stabilityClassifier = shtpData[5 + 4]; //Byte 4 only
	} else if (shtpData[5] == SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER) {
		activitySample.tick_ms = HAL_GetTick();

		//Load activity classification confidences into the array
		for (uint8_t x = 0; x < 9; x++)	//Hardcoded to max of 9. TODO - bring in array size
			_activityConfidences[x] = shtpData[5 + 6 + x]; //5 bytes of timestamp, byte 6 is first confidence byte

		activityClassifier = shtpData[5 + 5]; //Most likely state

		// put activity sample in queue for message packing
		memcpy(activitySample.activityConfidence, _activityConfidences, 9);

//		test = osMessageQueueGetCount(activitySampleQueueHandle);
		osMessageQueuePut(activitySampleQueueHandle, &activitySample, 0U, 0);
//		test = osMessageQueueGetCount(activitySampleQueueHandle);

	} else if (shtpData[5] == SENSOR_REPORTID_RAW_ACCELEROMETER) {
		// put acceleration sample in queue for message packing
		accSample[accQueueIdx].data[accPacketIdx].imu_tick_ms = data5;
		accSample[accQueueIdx].data[accPacketIdx].tick_ms = HAL_GetTick();
		accSample[accQueueIdx].data[accPacketIdx].x = data1
		;
		accSample[accQueueIdx].data[accPacketIdx].y = data2
		;
		accSample[accQueueIdx].data[accPacketIdx].z = data3
		;
		accSample[accQueueIdx].data[accPacketIdx].accuracy = status;

		accPacketIdx++;

		if(accPacketIdx >= MAX_THREE_AXIS_PAYLOAD_ENTRIES){
			accPacketIdx = 0;
			if(osOK == osMessageQueuePut(accSampleQueueHandle, ( void * ) &accSample[accQueueIdx], 0U, 0)){
				accQueueIdx += 1;

				if(accQueueIdx >= ACC_GYRO_QUEUE_SIZE){
					accQueueIdx = 0;
				}

			}
		}


		memsRawAccelX = data1;
		memsRawAccelY = data2;
		memsRawAccelZ = data3;
	} else if (shtpData[5] == SENSOR_REPORTID_RAW_GYROSCOPE) {
		// put acceleration sample in queue for message packing
		gyroSample[gyroQueueIdx].data[gyroPacketIdx].imu_tick_ms = data5;
		gyroSample[gyroQueueIdx].data[gyroPacketIdx].tick_ms = HAL_GetTick();
		gyroSample[gyroQueueIdx].data[gyroPacketIdx].x = data1
		;
		gyroSample[gyroQueueIdx].data[gyroPacketIdx].y = data2
		;
		gyroSample[gyroQueueIdx].data[gyroPacketIdx].z = data3
		;
		gyroSample[gyroQueueIdx].data[gyroPacketIdx].accuracy = IMU_qToFloat(status, 12);

		gyroPacketIdx++;

		if(gyroPacketIdx >= MAX_THREE_AXIS_PAYLOAD_ENTRIES){
			gyroPacketIdx = 0;
			if(osOK == osMessageQueuePut(gyroSampleQueueHandle, ( void * ) &gyroSample[accQueueIdx], 0U, 0)){
				gyroQueueIdx += 1;

				if(gyroQueueIdx >= ACC_GYRO_QUEUE_SIZE){
					gyroQueueIdx = 0;
				}
			}

		}

		memsRawGyroX = data1;
		memsRawGyroY = data2;
		memsRawGyroZ = data3;
	} else if (shtpData[5] == SENSOR_REPORTID_RAW_MAGNETOMETER) {
		memsRawMagX = data1;
		memsRawMagY = data2;
		memsRawMagZ = data3;
	} else if (shtpData[5] == SHTP_REPORT_COMMAND_RESPONSE) {
//		Serial.println("!");
		//The BNO080 responds with this report to command requests. It's up to use to remember which command we issued.
		uint8_t command = shtpData[5 + 2]; //This is the Command byte of the response

		if (command == COMMAND_ME_CALIBRATE) {
			//Serial.println("ME Cal report found!");
			calibrationStatus = shtpData[5 + 5]; //R0 - Status (0 = success, non-zero = fail)
		}
	} else {
		//This sensor report ID is unhandled.
		//See reference manual to add additional feature reports as needed
	}

	//TODO additional feature reports may be strung together. Parse them all.
}

//Return the rotation vector quaternion I
float IMU_getQuatI() {
	float quat = IMU_qToFloat(rawQuatI, rotationVector_Q1);
	return (quat);
}

//Return the rotation vector quaternion J
float IMU_getQuatJ() {
	float quat = IMU_qToFloat(rawQuatJ, rotationVector_Q1);
	return (quat);
}

//Return the rotation vector quaternion K
float IMU_getQuatK() {
	float quat = IMU_qToFloat(rawQuatK, rotationVector_Q1);
	return (quat);
}

//Return the rotation vector quaternion Real
float IMU_getQuatReal() {
	float quat = IMU_qToFloat(rawQuatReal, rotationVector_Q1);
	return (quat);
}

//Return the rotation vector accuracy
float IMU_getQuatRadianAccuracy() {
	float quat = IMU_qToFloat(rawQuatRadianAccuracy, 12);
	return (quat);
}

//Return the acceleration component
uint8_t IMU_getQuatAccuracy() {
	return (quatAccuracy);
}

//Return the acceleration component
float IMU_getAccelX() {
	float accel = IMU_qToFloat(rawAccelX, accelerometer_Q1);
	return (accel);
}

//Return the acceleration component
float IMU_getAccelY() {
	float accel = IMU_qToFloat(rawAccelY, accelerometer_Q1);
	return (accel);
}

//Return the acceleration component
float IMU_getAccelZ() {
	float accel = IMU_qToFloat(rawAccelZ, accelerometer_Q1);
	return (accel);
}

//Return the acceleration component
uint8_t IMU_getAccelAccuracy() {
	return (accelAccuracy);
}

// linear acceleration, i.e. minus gravity

//Return the acceleration component
float IMU_getLinAccelX() {
	float accel = IMU_qToFloat(rawLinAccelX, linear_accelerometer_Q1);
	return (accel);
}

//Return the acceleration component
float IMU_getLinAccelY() {
	float accel = IMU_qToFloat(rawLinAccelY, linear_accelerometer_Q1);
	return (accel);
}

//Return the acceleration component
float IMU_getLinAccelZ() {
	float accel = IMU_qToFloat(rawLinAccelZ, linear_accelerometer_Q1);
	return (accel);
}

//Return the acceleration component
uint8_t IMU_getLinAccelAccuracy() {
	return (accelLinAccuracy);
}

//Return the gyro component
float IMU_getGyroX() {
	float gyro = IMU_qToFloat(rawGyroX, gyro_Q1);
	return (gyro);
}

//Return the gyro component
float IMU_getGyroY() {
	float gyro = IMU_qToFloat(rawGyroY, gyro_Q1);
	return (gyro);
}

//Return the gyro component
float IMU_getGyroZ() {
	float gyro = IMU_qToFloat(rawGyroZ, gyro_Q1);
	return (gyro);
}

//Return the gyro component
uint8_t IMU_getGyroAccuracy() {
	return (gyroAccuracy);
}

//Return the magnetometer component
float IMU_getMagX() {
	float mag = IMU_qToFloat(rawMagX, magnetometer_Q1);
	return (mag);
}

//Return the magnetometer component
float IMU_getMagY() {
	float mag = IMU_qToFloat(rawMagY, magnetometer_Q1);
	return (mag);
}

//Return the magnetometer component
float IMU_getMagZ() {
	float mag = IMU_qToFloat(rawMagZ, magnetometer_Q1);
	return (mag);
}

//Return the mag component
uint8_t IMU_getMagAccuracy() {
	return (magAccuracy);
}

//Return the step count
uint16_t IMU_getStepCount() {
	return (stepCount);
}

//Return the stability classifier
uint8_t IMU_getStabilityClassifier() {
	return (stabilityClassifier);
}

//Return the activity classifier
uint8_t IMU_getActivityClassifier() {
	return (activityClassifier);
}

//Return the time stamp
uint32_t IMU_getTimeStamp() {
	return (timeStamp);
}

//Return raw mems value for the accel
int16_t IMU_getRawAccelX() {
	return (memsRawAccelX);
}
//Return raw mems value for the accel
int16_t IMU_getRawAccelY() {
	return (memsRawAccelY);
}
//Return raw mems value for the accel
int16_t IMU_getRawAccelZ() {
	return (memsRawAccelZ);
}

//Return raw mems value for the gyro
int16_t IMU_getRawGyroX() {
	return (memsRawGyroX);
}
int16_t IMU_getRawGyroY() {
	return (memsRawGyroY);
}
int16_t IMU_getRawGyroZ() {
	return (memsRawGyroZ);
}

//Return raw mems value for the mag
int16_t IMU_getRawMagX() {
	return (memsRawMagX);
}
int16_t IMU_getRawMagY() {
	return (memsRawMagY);
}
int16_t IMU_getRawMagZ() {
	return (memsRawMagZ);
}

//Given a record ID, read the Q1 value from the metaData record in the FRS (ya, it's complicated)
//Q1 is used for all sensor data calculations
int16_t IMU_getQ1(uint16_t recordID) {
	//Q1 is always the lower 16 bits of word 7
	uint16_t q = IMU_readFRSword(recordID, 7) & 0xFFFF; //Get word 7, lower 16 bits
	return (q);
}

//Given a record ID, read the Q2 value from the metaData record in the FRS
//Q2 is used in sensor bias
int16_t IMU_getQ2(uint16_t recordID) {
	//Q2 is always the upper 16 bits of word 7
	uint16_t q = IMU_readFRSword(recordID, 7) >> 16; //Get word 7, upper 16 bits
	return (q);
}

//Given a record ID, read the Q3 value from the metaData record in the FRS
//Q3 is used in sensor change sensitivity
int16_t IMU_getQ3(uint16_t recordID) {
	//Q3 is always the upper 16 bits of word 8
	uint16_t q = IMU_readFRSword(recordID, 8) >> 16; //Get word 8, upper 16 bits
	return (q);
}

//Given a record ID, read the resolution value from the metaData record in the FRS for a given sensor
float IMU_getResolution(uint16_t recordID) {
	//The resolution Q value are 'the same as those used in the sensor's input report'
	//This should be Q1.
	int16_t Q = IMU_getQ1(recordID);

	//Resolution is always word 2
	uint32_t value = IMU_readFRSword(recordID, 2); //Get word 2

	float resolution = IMU_qToFloat(value, Q);

	return (resolution);
}

//Given a record ID, read the range value from the metaData record in the FRS for a given sensor
float IMU_getRange(uint16_t recordID) {
	//The resolution Q value are 'the same as those used in the sensor's input report'
	//This should be Q1.
	int16_t Q = IMU_getQ1(recordID);

	//Range is always word 1
	uint32_t value = IMU_readFRSword(recordID, 1); //Get word 1

	float range = IMU_qToFloat(value, Q);

	return (range);
}

//Given a record ID and a word number, look up the word data
//Helpful for pulling out a Q value, range, etc.
//Use readFRSdata for pulling out multi-word objects for a sensor (Vendor data for example)
uint32_t IMU_readFRSword(uint16_t recordID, uint8_t wordNumber) {
	if (IMU_readFRSdata(recordID, wordNumber, 1) == true) //Get word number, just one word in length from FRS
		return (metaData[0]);						  //Return this one word

	return (0); //Error
}

//Ask the sensor for data from the Flash Record System
//See 6.3.6 page 40, FRS Read Request
void IMU_frsReadRequest(uint16_t recordID, uint16_t readOffset,
		uint16_t blockSize) {
	shtpData[0] = SHTP_REPORT_FRS_READ_REQUEST; //FRS Read Request
	shtpData[1] = 0;							//Reserved
	shtpData[2] = (readOffset >> 0) & 0xFF;		//Read Offset LSB
	shtpData[3] = (readOffset >> 8) & 0xFF;		//Read Offset MSB
	shtpData[4] = (recordID >> 0) & 0xFF;		//FRS Type LSB
	shtpData[5] = (recordID >> 8) & 0xFF;		//FRS Type MSB
	shtpData[6] = (blockSize >> 0) & 0xFF;		//Block size LSB
	shtpData[7] = (blockSize >> 8) & 0xFF;		//Block size MSB

	//Transmit packet on channel 2, 8 bytes
	IMU_sendPacket(CHANNEL_CONTROL, 8);
}

//Given a sensor or record ID, and a given start/stop bytes, read the data from the Flash Record System (FRS) for this sensor
//Returns true if metaData array is loaded successfully
//Returns false if failure
bool IMU_readFRSdata(uint16_t recordID, uint8_t startLocation,
		uint8_t wordsToRead) {
	uint8_t spot = 0;

	//First we send a Flash Record System (FRS) request
	IMU_frsReadRequest(recordID, startLocation, wordsToRead); //From startLocation of record, read a # of words

	//Read bytes until FRS reports that the read is complete
	while (1) {
		//Now we wait for response
		while (1) {
			uint8_t counter = 0;
			while (IMU_receivePacket() == false) {
				if (counter++ > 100)
					return (false); //Give up
				HAL_Delay(1);
			}

			//We have the packet, inspect it for the right contents
			//See page 40. Report ID should be 0xF3 and the FRS types should match the thing we requested
			if (shtpData[0] == SHTP_REPORT_FRS_READ_RESPONSE)
				if (((uint16_t) shtpData[13] << 8 | shtpData[12]) == recordID)
					break; //This packet is one we are looking for
		}

		uint8_t dataLength = shtpData[1] >> 4;
		uint8_t frsStatus = shtpData[1] & 0x0F;

		uint32_t data0 = (uint32_t) shtpData[7] << 24
				| (uint32_t) shtpData[6] << 16 | (uint32_t) shtpData[5] << 8
				| (uint32_t) shtpData[4];
		uint32_t data1 = (uint32_t) shtpData[11] << 24
				| (uint32_t) shtpData[10] << 16 | (uint32_t) shtpData[9] << 8
				| (uint32_t) shtpData[8];

		//Record these words to the metaData array
		if (dataLength > 0) {
			metaData[spot++] = data0;
		}
		if (dataLength > 1) {
			metaData[spot++] = data1;
		}

		if (spot >= MAX_METADATA_SIZE) {
//			if (_printDebug == true)
//				_debugPort->println(F("metaData array over run. Returning."));
			return (true); //We have run out of space in our array. Bail.
		}

		if (frsStatus == 3 || frsStatus == 6 || frsStatus == 7) {
			return (true); //FRS status is read completed! We're done!
		}
	}
}

//Send command to reset IC
//Read all advertisement packets from sensor
//The sensor has been seen to reset twice if we attempt too much too quickly.
//This seems to work reliably.
void IMU_softReset(void) {
	shtpData[0] = 1; //Reset

	//Attempt to start communication with sensor
	IMU_sendPacket(CHANNEL_EXECUTABLE, 1); //Transmit packet on channel 1, 1 byte

	//Read all incoming data and flush it
	HAL_Delay(200);
	while (IMU_receivePacket() == true)
		;
	HAL_Delay(200);
	while (IMU_receivePacket() == true)
		;
}

//Get the reason for the last reset
//1 = POR, 2 = Internal reset, 3 = Watchdog, 4 = External reset, 5 = Other
uint8_t IMU_resetReason() {
	shtpData[0] = SHTP_REPORT_PRODUCT_ID_REQUEST; //Request the product ID and reset info
	shtpData[1] = 0;							  //Reserved

	//Transmit packet on channel 2, 2 bytes
	IMU_sendPacket(CHANNEL_CONTROL, 2);

	//Now we wait for response
	if (IMU_receivePacket() == true) {
		if (shtpData[0] == SHTP_REPORT_PRODUCT_ID_RESPONSE) {
			return (shtpData[1]);
		}
	}

	return (0);
}

//Given a register value and a Q point, convert to float
//See https://en.wikipedia.org/wiki/Q_(number_format)
float IMU_qToFloat(int16_t fixedPointValue, uint8_t qPoint) {
	float qFloat = fixedPointValue;
	qFloat *= pow(2, qPoint * -1);
	return (qFloat);
}

//Sends the packet to enable the rotation vector
void IMU_enableRotationVector(uint16_t timeBetweenReports) {
	IMU_setFeatureCommand_2(SENSOR_REPORTID_ROTATION_VECTOR,
			timeBetweenReports);
}

//Sends the packet to enable the rotation vector
void IMU_enableGameRotationVector(uint16_t timeBetweenReports) {
	IMU_setFeatureCommand_2(SENSOR_REPORTID_GAME_ROTATION_VECTOR,
			timeBetweenReports);
}

//Sends the packet to enable the accelerometer
void IMU_enableAccelerometer(uint16_t timeBetweenReports) {
	IMU_setFeatureCommand_2(SENSOR_REPORTID_ACCELEROMETER, timeBetweenReports);
}

//Sends the packet to enable the accelerometer
void IMU_enableLinearAccelerometer(uint16_t timeBetweenReports) {
	IMU_setFeatureCommand_2(SENSOR_REPORTID_LINEAR_ACCELERATION,
			timeBetweenReports);
}

//Sends the packet to enable the gyro
void IMU_enableGyro(uint16_t timeBetweenReports) {
	IMU_setFeatureCommand_2(SENSOR_REPORTID_GYROSCOPE, timeBetweenReports);
}

//Sends the packet to enable the gyro
void IMU_enableUncalGyro(uint16_t timeBetweenReports) {
	IMU_setFeatureCommand_2(SENSOR_REPORTID_GYROSCOPE_UNCAL, timeBetweenReports);
}

//Sends the packet to enable the magnetometer
void IMU_enableMagnetometer(uint16_t timeBetweenReports) {
	IMU_setFeatureCommand_2(SENSOR_REPORTID_MAGNETIC_FIELD, timeBetweenReports);
}

//Sends the packet to enable the step counter
void IMU_enableStepCounter(uint16_t timeBetweenReports) {
	IMU_setFeatureCommand_2(SENSOR_REPORTID_STEP_COUNTER, timeBetweenReports);
}

//Sends the packet to enable the Stability Classifier
void IMU_enableStabilityClassifier(uint16_t timeBetweenReports) {
	IMU_setFeatureCommand_2(SENSOR_REPORTID_STABILITY_CLASSIFIER,
			timeBetweenReports);
}

//Sends the packet to enable the raw accel readings
//Note you must enable basic reporting on the sensor as well
void IMU_enableRawAccelerometer(uint16_t timeBetweenReports) {
	IMU_setFeatureCommand_2(SENSOR_REPORTID_RAW_ACCELEROMETER,
			timeBetweenReports);
}

//Sends the packet to enable the raw accel readings
//Note you must enable basic reporting on the sensor as well
void IMU_enableRawGyro(uint16_t timeBetweenReports) {
	IMU_setFeatureCommand_2(SENSOR_REPORTID_RAW_GYROSCOPE, timeBetweenReports);
}

//Sends the packet to enable the raw accel readings
//Note you must enable basic reporting on the sensor as well
void IMU_enableRawMagnetometer(uint16_t timeBetweenReports) {
	IMU_setFeatureCommand_2(SENSOR_REPORTID_RAW_MAGNETOMETER,
			timeBetweenReports);
}

//Sends the packet to enable the various activity classifiers
void IMU_enableActivityClassifier(uint16_t timeBetweenReports,
		uint32_t activitiesToEnable, uint8_t *activityConfidences) {
	_activityConfidences = activityConfidences; //Store pointer to array

	IMU_setFeatureCommand_3(SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER,
			timeBetweenReports, activitiesToEnable);
}

//Sends the commands to begin calibration of the accelerometer
void IMU_calibrateAccelerometer() {
	IMU_sendCalibrateCommand(CALIBRATE_ACCEL);
}

//Sends the commands to begin calibration of the gyro
void IMU_calibrateGyro() {
	IMU_sendCalibrateCommand(CALIBRATE_GYRO);
}

//Sends the commands to begin calibration of the magnetometer
void IMU_calibrateMagnetometer() {
	IMU_sendCalibrateCommand(CALIBRATE_MAG);
}

//Sends the commands to begin calibration of the planar accelerometer
void IMU_calibratePlanarAccelerometer() {
	IMU_sendCalibrateCommand(CALIBRATE_PLANAR_ACCEL);
}

//See 2.2 of the Calibration Procedure document 1000-4044
void IMU_calibrateAll() {
	IMU_sendCalibrateCommand(CALIBRATE_ACCEL_GYRO_MAG);
}

void IMU_endCalibration() {
	IMU_sendCalibrateCommand(CALIBRATE_STOP); //Disables all calibrations
}

//See page 51 of reference manual - ME Calibration Response
//Byte 5 is parsed during the readPacket and stored in calibrationStatus
bool IMU_calibrationComplete() {
	if (calibrationStatus == 0)
		return (true);
	return (false);
}

//Given a sensor's report ID, this tells the BNO080 to begin reporting the values
void IMU_setFeatureCommand_2(uint8_t reportID, uint16_t timeBetweenReports) {
	IMU_setFeatureCommand_3(reportID, timeBetweenReports, 0); //No specific config
}

//Given a sensor's report ID, this tells the BNO080 to begin reporting the values
//Also sets the specific config word. Useful for personal activity classifier
void IMU_setFeatureCommand_3(uint8_t reportID, uint16_t timeBetweenReports,
		uint32_t specificConfig) {
	long microsBetweenReports = (long) timeBetweenReports * 1000L;

	shtpData[0] = SHTP_REPORT_SET_FEATURE_COMMAND;//Set feature command. Reference page 55
	shtpData[1] = reportID;	//Feature Report ID. 0x01 = Accelerometer, 0x05 = Rotation vector
	shtpData[2] = 0;								   //Feature flags
	shtpData[3] = 0;								  //Change sensitivity (LSB)
	shtpData[4] = 0;								  //Change sensitivity (MSB)
	shtpData[5] = (microsBetweenReports >> 0) & 0xFF; //Report interval (LSB) in microseconds. 0x7A120 = 500ms
	shtpData[6] = (microsBetweenReports >> 8) & 0xFF;  //Report interval
	shtpData[7] = (microsBetweenReports >> 16) & 0xFF; //Report interval
	shtpData[8] = (microsBetweenReports >> 24) & 0xFF; //Report interval (MSB)
	shtpData[9] = 0;								   //Batch Interval (LSB)
	shtpData[10] = 0;								   //Batch Interval
	shtpData[11] = 0;								   //Batch Interval
	shtpData[12] = 0;								   //Batch Interval (MSB)
	shtpData[13] = (specificConfig >> 0) & 0xFF;//Sensor-specific config (LSB)
	shtpData[14] = (specificConfig >> 8) & 0xFF;	   //Sensor-specific config
	shtpData[15] = (specificConfig >> 16) & 0xFF;	  //Sensor-specific config
	shtpData[16] = (specificConfig >> 24) & 0xFF;//Sensor-specific config (MSB)

	//Transmit packet on channel 2, 17 bytes
	IMU_sendPacket(CHANNEL_CONTROL, 17);
}

//Tell the sensor to do a command
//See 6.3.8 page 41, Command request
//The caller is expected to set P0 through P8 prior to calling
void IMU_sendCommand(uint8_t command) {
	shtpData[0] = SHTP_REPORT_COMMAND_REQUEST; //Command Request
	shtpData[1] = commandSequenceNumber++;//Increments automatically each function call
	shtpData[2] = command;					   //Command

	//Caller must set these
	/*shtpData[3] = 0; //P0
	 shtpData[4] = 0; //P1
	 shtpData[5] = 0; //P2
	 shtpData[6] = 0;
	 shtpData[7] = 0;
	 shtpData[8] = 0;
	 shtpData[9] = 0;
	 shtpData[10] = 0;
	 shtpData[11] = 0;*/

	//Transmit packet on channel 2, 12 bytes
	IMU_sendPacket(CHANNEL_CONTROL, 12);
}

void IMU_sendTareNow(void) {
	shtpData[0] = SHTP_REPORT_COMMAND_REQUEST; //Command Request
	shtpData[1] = commandSequenceNumber++;//Increments automatically each function call
	shtpData[2] = 0x03;					   // TareCommand
	shtpData[3] = 0; // Tare Now
	shtpData[4] = 0x07; // All 3 axes
	shtpData[5] = 0; // Rotation Vector

	// RESERVED
	shtpData[6] = 0;
	shtpData[7] = 0;
	shtpData[8] = 0;
	shtpData[9] = 0;
	shtpData[10] = 0;
	shtpData[11] = 0;

	//Transmit packet on channel 2, 12 bytes
	IMU_sendPacket(CHANNEL_CONTROL, 12);
}

void IMU_sendPersistTare(void) {
	shtpData[0] = SHTP_REPORT_COMMAND_REQUEST; //Command Request
	shtpData[1] = commandSequenceNumber++;//Increments automatically each function call
	shtpData[2] = 0x03;					   // TareCommand
	shtpData[3] = 0x01; // Persist Tare

	// RESERVED
	shtpData[4] = 0;
	shtpData[5] = 0;
	shtpData[6] = 0;
	shtpData[7] = 0;
	shtpData[8] = 0;
	shtpData[9] = 0;
	shtpData[10] = 0;
	shtpData[11] = 0;

	//Transmit packet on channel 2, 12 bytes
	IMU_sendPacket(CHANNEL_CONTROL, 12);
}

//This tells the BNO080 to begin calibrating
//See page 50 of reference manual and the 1000-4044 calibration doc
void IMU_sendCalibrateCommand(uint8_t thingToCalibrate) {
	/*shtpData[3] = 0; //P0 - Accel Cal Enable
	 shtpData[4] = 0; //P1 - Gyro Cal Enable
	 shtpData[5] = 0; //P2 - Mag Cal Enable
	 shtpData[6] = 0; //P3 - Subcommand 0x00
	 shtpData[7] = 0; //P4 - Planar Accel Cal Enable
	 shtpData[8] = 0; //P5 - Reserved
	 shtpData[9] = 0; //P6 - Reserved
	 shtpData[10] = 0; //P7 - Reserved
	 shtpData[11] = 0; //P8 - Reserved*/

	for (uint8_t x = 3; x < 12; x++) //Clear this section of the shtpData array
		shtpData[x] = 0;

	if (thingToCalibrate == CALIBRATE_ACCEL)
		shtpData[3] = 1;
	else if (thingToCalibrate == CALIBRATE_GYRO)
		shtpData[4] = 1;
	else if (thingToCalibrate == CALIBRATE_MAG)
		shtpData[5] = 1;
	else if (thingToCalibrate == CALIBRATE_PLANAR_ACCEL)
		shtpData[7] = 1;
	else if (thingToCalibrate == CALIBRATE_ACCEL_GYRO_MAG) {
		shtpData[3] = 1;
		shtpData[4] = 1;
		shtpData[5] = 1;
	} else if (thingToCalibrate == CALIBRATE_STOP)
		; //Do nothing, bytes are set to zero

	//Make the internal calStatus variable non-zero (operation failed) so that user can test while we wait
	calibrationStatus = 1;

	//Using this shtpData packet, send a command
	IMU_sendCommand(COMMAND_ME_CALIBRATE);
}

//Request ME Calibration Status from BNO080
//See page 51 of reference manual
void IMU_requestCalibrationStatus() {
	/*shtpData[3] = 0; //P0 - Reserved
	 shtpData[4] = 0; //P1 - Reserved
	 shtpData[5] = 0; //P2 - Reserved
	 shtpData[6] = 0; //P3 - 0x01 - Subcommand: Get ME Calibration
	 shtpData[7] = 0; //P4 - Reserved
	 shtpData[8] = 0; //P5 - Reserved
	 shtpData[9] = 0; //P6 - Reserved
	 shtpData[10] = 0; //P7 - Reserved
	 shtpData[11] = 0; //P8 - Reserved*/

	for (uint8_t x = 3; x < 12; x++) //Clear this section of the shtpData array
		shtpData[x] = 0;

	shtpData[6] = 0x01; //P3 - 0x01 - Subcommand: Get ME Calibration

	//Using this shtpData packet, send a command
	IMU_sendCommand(COMMAND_ME_CALIBRATE);
}

//This tells the BNO080 to save the Dynamic Calibration Data (DCD) to flash
//See page 49 of reference manual and the 1000-4044 calibration doc
void IMU_saveCalibration() {
	/*shtpData[3] = 0; //P0 - Reserved
	 shtpData[4] = 0; //P1 - Reserved
	 shtpData[5] = 0; //P2 - Reserved
	 shtpData[6] = 0; //P3 - Reserved
	 shtpData[7] = 0; //P4 - Reserved
	 shtpData[8] = 0; //P5 - Reserved
	 shtpData[9] = 0; //P6 - Reserved
	 shtpData[10] = 0; //P7 - Reserved
	 shtpData[11] = 0; //P8 - Reserved*/

	for (uint8_t x = 3; x < 12; x++) //Clear this section of the shtpData array
		shtpData[x] = 0;

	//Using this shtpData packet, send a command
	IMU_sendCommand(COMMAND_DCD); //Save DCD command
}

//Wait a certain time for incoming I2C bytes before giving up
//Returns false if failed
//bool IMU_waitForI2C()
//{
////	for (uint8_t counter = 0; counter < 100; counter++) //Don't got more than 255
////	{
////		if (_i2cPort->available() > 0)
////			return (true);
////		delay(1);
////	}
////
////	if (_printDebug == true)
////		_debugPort->println(F("I2C timeout"));
//	return (false);
//}

////Blocking wait for BNO080 to assert (pull low) the INT pin
////indicating it's ready for comm. Can take more than 104ms
////after a hardware reset
//bool IMU_waitForSPI()
//{
////	for (uint8_t counter = 0; counter < 125; counter++) //Don't got more than 255
////	{
////		if (digitalRead(_int) == LOW)
////			return (true);
////		if (_printDebug == true)
////			_debugPort->println(F("SPI Wait"));
////		delay(1);
////	}
////
////	if (_printDebug == true)
////		_debugPort->println(F("SPI INT timeout"));
//	return (false);
//}

//Check to see if there is any new data available
//Read the contents of the incoming packet into the shtpData array
bool IMU_receivePacket(void) {
	if (HAL_GPIO_ReadPin(_intPort, _int) == GPIO_PIN_SET)
		return (false);

	//Ask for four bytes to find out how much data we need to read
	osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
//	taskENTER_CRITICAL();
	HAL_I2C_Master_Receive(&hi2c1, _deviceAddress, shtpHeader, (uint8_t) 4,
			100);
//	while( HAL_I2C_Master_Receive(&hi2c1, _deviceAddress, shtpHeader, (uint8_t) 4, 100) != HAL_OK){
//		osSemaphoreRelease(messageI2C_LockSem);
//		osDelay(100);
//		osSemaphoreAcquire(messageI2C_LockSem, osWaitForever);
//	}
//	taskEXIT_CRITICAL();
	osSemaphoreRelease(messageI2C_LockHandle);

	//Calculate the number of data bytes in this packet
	int16_t dataLength = (((uint16_t) shtpHeader[PACKET_MSB] << 8)
			| shtpHeader[PACKET_LSB]);
	dataLength &= ~(1 << 15); //Clear the MSbit.
	//This bit indicates if this package is a continuation of the last. Ignore it for now.
	//TODO catch this as an error and exit
	if (dataLength == 0) {
		//Packet is empty
		return (false); //All done
	}
	dataLength -= 4; //Remove the header bytes from the data count

	IMU_getData(dataLength);

	return (true); //We're done!
}

//Sends multiple requests to sensor until all data bytes are received from sensor
//The shtpData buffer has max capacity of MAX_PACKET_SIZE. Any bytes over this amount will be lost.
//Arduino I2C read limit is 32 bytes. Header is 4 bytes, so max data we can read per interation is 28 bytes
bool IMU_getData(uint16_t bytesRemaining) {
	uint16_t dataSpot = 0; //Start at the beginning of shtpData array

//	uint8_t receiveBuffer[28] = {0};
//	uint8_t receivePacket[I2C_BUFFER_LENGTH - 4] = {0};

	//Setup a series of chunked 32 byte reads
	while (bytesRemaining > 0) {
		uint16_t numberOfBytesToRead = bytesRemaining;
		if (numberOfBytesToRead > (I2C_BUFFER_LENGTH - 4))
			numberOfBytesToRead = (I2C_BUFFER_LENGTH - 4);

		osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
//		taskENTER_CRITICAL();
		HAL_I2C_Master_Receive(&hi2c1, _deviceAddress, receiveBuffer,
				(uint8_t) (numberOfBytesToRead + 4), 100);
//		while( HAL_I2C_Master_Receive(&hi2c1, _deviceAddress, receiveBuffer, (uint8_t)(numberOfBytesToRead + 4), 100) != HAL_OK){
//			osSemaphoreRelease(messageI2C_LockSem);
//			osDelay(100);
//			osSemaphoreAcquire(messageI2C_LockSem, osWaitForever);
//		}
//		taskEXIT_CRITICAL();
		osSemaphoreRelease(messageI2C_LockHandle);

//		_i2cPort->requestFrom((uint8_t)_deviceAddress, (uint8_t)(numberOfBytesToRead + 4));
//		if (waitForI2C() == false)
//			return (0); //Error

		// first four bytes are header bytes and can be thrown away
		if ((dataSpot + numberOfBytesToRead) < MAX_PACKET_SIZE) {
			memcpy(&(shtpData[dataSpot]), &(receiveBuffer[4]),
					numberOfBytesToRead);
		} else {
			// Do nothing with the data
		}

		// increment data index
		// TODO: this can be changed to receive a larger buffer since this constraint is for the Arduino
		dataSpot += numberOfBytesToRead;

		bytesRemaining -= numberOfBytesToRead;
	}
	return (true); //Done!
}

//Given the data packet, send the header then the data
//Returns false if sensor does not ACK
//TODO - Arduino has a max 32 byte send. Break sending into multi packets if needed.
bool IMU_sendPacket(uint8_t channelNumber, uint8_t dataLength) {
	uint8_t packetLength = dataLength + 4; //Add four bytes for the header

	/*  *********** POPULATE HEADER ****************************** */
	outPacket[0] = packetLength & 0xFF; 				//Packet length LSB
	outPacket[1] = packetLength >> 8; 					//Packet length MSB
	outPacket[2] = channelNumber; 						//Channel number
	outPacket[3] = sequenceNumber[channelNumber]++;	//Send the sequence number, increments with each packet sent, different counter for each channel

	/*  *********** FILL PAYLOAD ********************************* */
	memcpy(&(outPacket[4]), shtpData, dataLength);

	/*  *********** SEND TO IMU ********************************** */
	osSemaphoreAcquire(messageI2C_LockHandle, osWaitForever);
//	taskENTER_CRITICAL();
	HAL_I2C_Master_Transmit(&hi2c1, _deviceAddress, outPacket, packetLength,
			100);
//	while( HAL_I2C_Master_Transmit(&hi2c1, _deviceAddress, outPacket, packetLength, 100) != HAL_OK){
//		osSemaphoreRelease(messageI2C_LockSem);
//		osDelay(100);
//		osSemaphoreAcquire(messageI2C_LockSem, osWaitForever);
//	}
//	taskEXIT_CRITICAL();
	osSemaphoreRelease(messageI2C_LockHandle);

	return (true);
}

////Pretty prints the contents of the current shtp header and data packets
//void IMU_printPacket(void)
//{
////	if (_printDebug == true)
////	{
////		uint16_t packetLength = (uint16_t)shtpHeader[1] << 8 | shtpHeader[0];
////
////		//Print the four byte header
////		_debugPort->print(F("Header:"));
////		for (uint8_t x = 0; x < 4; x++)
////		{
////			_debugPort->print(F(" "));
////			if (shtpHeader[x] < 0x10)
////				_debugPort->print(F("0"));
////			_debugPort->print(shtpHeader[x], HEX);
////		}
////
////		uint8_t printLength = packetLength - 4;
////		if (printLength > 40)
////			printLength = 40; //Artificial limit. We don't want the phone book.
////
////		_debugPort->print(F(" Body:"));
////		for (uint8_t x = 0; x < printLength; x++)
////		{
////			_debugPort->print(F(" "));
////			if (shtpData[x] < 0x10)
////				_debugPort->print(F("0"));
////			_debugPort->print(shtpData[x], HEX);
////		}
////
////		if (packetLength & 1 << 15)
////		{
////			_debugPort->println(F(" [Continued packet] "));
////			packetLength &= ~(1 << 15);
////		}
////
////		_debugPort->print(F(" Length:"));
////		_debugPort->print(packetLength);
////
////		_debugPort->print(F(" Channel:"));
////		if (shtpHeader[2] == 0)
////			_debugPort->print(F("Command"));
////		else if (shtpHeader[2] == 1)
////			_debugPort->print(F("Executable"));
////		else if (shtpHeader[2] == 2)
////			_debugPort->print(F("Control"));
////		else if (shtpHeader[2] == 3)
////			_debugPort->print(F("Sensor-report"));
////		else if (shtpHeader[2] == 4)
////			_debugPort->print(F("Wake-report"));
////		else if (shtpHeader[2] == 5)
////			_debugPort->print(F("Gyro-vector"));
////		else
////			_debugPort->print(shtpHeader[2]);
////
////		_debugPort->println();
////	}
//}

