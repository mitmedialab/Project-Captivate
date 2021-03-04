/*
 * network_test.h
 *
 *  Created on: Mar 2, 2021
 *      Author: patrick
 */

#ifndef INC_NETWORK_TEST_H_
#define INC_NETWORK_TEST_H_

#include "blink.h"
#include "inter_processor_comms.h"
#include "inertial_sensing.h"
#include "messages.h"

struct NetworkTestPacket {
	struct blinkData blink;
	struct parsedSecondaryProcessorPacket procData;
	struct inertialData inertial;
	VIVEVars pos;
	uint32_t tick_ms;
	uint32_t epoch;
	uint32_t packetCnt;
	uint32_t uniqueID;
};

void NetworkTestTask(void *argument);


#endif /* INC_NETWORK_TEST_H_ */
