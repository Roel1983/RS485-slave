#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include <string.h>

#include "communication/communication.hpp"
#include "communication/sender/sender.hpp"

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
	communication::setup();
	
	/*
	PORTB |=  (_BV(0) | _BV(1));
	PORTD |=  (_BV(4) | _BV(7) | _BV(6));
	const uint8_t unique_id = 
		((PORTD & _BV(4)) ? 0 : 0b00001) |
		((PORTD & _BV(6)) ? 0 : 0b00010) |
		((PORTD & _BV(7)) ? 0 : 0b00100) |
		((PORTB & _BV(0)) ? 0 : 0b01000) |
		((PORTB & _BV(1)) ? 0 : 0b10000);*/
	
	communication::commandTypeSetBlockNr(COMMAND_TYPE_UNIQUE_ID, 0);
	sei();
}

void Loop() {
	communication::loop();
}
