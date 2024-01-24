#ifndef COMM_H
#define COMM_H

#include "stdint.h"

#ifdef UNITTEST
#define PRIVATE
#else
#define PRIVATE static
#endif

#define COMMMAND_TEST 0

typedef enum {
	COMM_RECEIVE_ERROR_STATE_NONE,
	COMM_RECEIVE_ERROR_STATE_UNSYNC,
	COMM_RECEIVE_ERROR_STATE_COMMAND_ID,
	COMM_RECEIVE_ERROR_STATE_CRC
} comm_receive_error_state_t;

#ifdef UNITTEST
void CommReceiveUnittestReset();
#endif

void CommReceiveLoop();

void CommReceiveByte(uint8_t b);

#endif
