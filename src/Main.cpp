#include <avr/interrupt.h>

#include "Comm.h"

void Setup();
void Loop();

#ifndef UNITTEST
int main (void)
{
	Setup();
	while(1) {
		Loop();
	}
}
#endif

void Setup() {
	CommBegin();
	sei();
}

void Loop() {
}

