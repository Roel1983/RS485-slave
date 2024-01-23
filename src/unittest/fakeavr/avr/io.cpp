#include "io.h"

uint8_t DDRB;
uint8_t PORTB;

void FakeIoReset() {
	DDRB  = 0;
	PORTB = 0;
}
