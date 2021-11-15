/*
 * network_test.c
 *
 *  Created on: Mar 2, 2021
 *      Author: patrick
 */
#include "network_test.h"
#include "captivate_config.h"
#include "app_thread.h"
#include "master_thread.h"


struct NetworkTestPacket testPacket;

#ifdef NETWORK_TEST
void NetworkTestTask(void *argument){

	uint32_t startTime = 0;
	uint32_t waitTime = 0;

	testPacket.uniqueID = HAL_GetDEVID();
	testPacket.packetCnt = 0;

	while(1){
		startTime = HAL_GetTick();

		/* SEND PACKET */
		APP_THREAD_NetworkTestBorderPacket(&testPacket);

		/* DELAY UNTIL NEXT PACKET SEND */
		// add delay to wait for next transmission period
		waitTime = NETWORK_TEST_PERIOD - (HAL_GetTick() - startTime);
		// if wait time is less than zero (i.e. the border packet send took longer than NETWORK_TEST_PERIOD)
		// or greater than the allotted NETWORK_TEST_PERIOD
		if ((waitTime <= 0) || (waitTime > NETWORK_TEST_PERIOD)) {
			waitTime = 0; //set to zero (i.e. dont wait)
		} else {
			osDelay(waitTime);
		}

		/* INCREMENT PACKET COUNT */
		testPacket.packetCnt += 1;
	}

}
#endif
