#ifndef COMM_H
#define COMM_H

#include "stdint.h"

typedef enum {
	COMM_ERROR_NONE,
	COMM_ERROR_SIGNAL,
	COMM_ERROR_DATA,
	COMM_ERROR_BUSY
} comm_error_t;
static_assert(sizeof(comm_error_t) == 1, "");

void CommBegin();
void CommLoop();

void CommSetDeviceNr(uint8_t device_nr);
void CommSetStripNr(uint8_t strip_nr);

comm_error_t CommGetLastError();

#endif
