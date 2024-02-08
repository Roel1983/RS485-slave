#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdio.h>
#include <string.h>

#include "Comm.h"
#include "Pins.h"
#include "CommandLib.h"

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
}

void Loop() {
}
