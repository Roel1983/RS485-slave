#ifndef COMM_H
#define COMM_H

#include "stdint.h"

#define COMMMAND_TEST 0

void CommReceiveLoop();

void CommReceiveByte(uint8_t b);

#endif
