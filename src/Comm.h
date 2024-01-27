#ifndef COMM_H
#define COMM_H

#include <stdint.h>

typedef enum {
	COMM_ERROR_NONE,
	COMM_ERROR_SIGNAL,
	COMM_ERROR_DATA,
	COMM_ERROR_BUSY
} comm_error_t;
#ifndef UNITTEST
static_assert(sizeof(comm_error_t) == 1, "");
#endif 

#ifdef UNITTEST
void CommReset();
#endif

void CommBegin();
void CommLoop();

void CommSetDeviceNr(uint8_t device_nr);
void CommSetStripNr(uint8_t strip_nr);

comm_error_t CommGetError();

#endif
