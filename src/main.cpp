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

static uint32_t last_timestamp;

void Setup() {
	communication::setup();
	timestamp::setup();
	
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
	
	last_timestamp = timestamp::getMsTimestamp();
	DDRB |= _BV(2);
	PORTB |= _BV(2);
	PORTB &= ~_BV(2);
}

void Loop() {
	communication::loop();
	
	uint32_t current_timestamp = timestamp::getMsTimestamp();
	if (current_timestamp - last_timestamp > 1000) {
		PORTB ^= _BV(2);
		last_timestamp = current_timestamp;
		communication::sendBroadcast(
			66,
			5,
			[](bool is_cancled, uint8_t& payload_size, uint8_t *payload_buffer) -> bool {
				memcpy(payload_buffer, "hello", 5);
				return true;
			});
	}
}
