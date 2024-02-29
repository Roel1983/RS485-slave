#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include <string.h>

#include "timestamp.hpp"
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
	timestamp::setup();
	
	DDRB  &= ~((_BV(0) | _BV(1)));
	PORTB |=  ((_BV(0) | _BV(1)));
	DDRD  &= ~((_BV(4) | _BV(7) | _BV(6)));
	PORTD |=  ((_BV(4) | _BV(7) | _BV(6)));
	const uint8_t unique_id = 
		((PIND & _BV(4)) ? 0 : 0b00001) |
		((PIND & _BV(6)) ? 0 : 0b00010) |
		((PIND & _BV(7)) ? 0 : 0b00100) |
		((PINB & _BV(0)) ? 0 : 0b01000) |
		((PINB & _BV(1)) ? 0 : 0b10000);
	communication::commandTypeSetBlockNr(COMMAND_TYPE_UNIQUE_ID, unique_id);
	
	sei();
	
	DDRC &= ~_BV(0);
	PORTC |= _BV(0);
}

bool button_state = false;
void Loop() {
	communication::loop();
	
	if (~PINC & _BV(0)) {
		if (!button_state) {
			button_state = true;
			communication::sendBroadcast(
				02,
				1,
				[](bool is_timeout, uint8_t& payload_size, uint8_t *payload_buffer) -> bool {
					return true;
				});
		}
	} else {
		button_state = false;
	}
}
