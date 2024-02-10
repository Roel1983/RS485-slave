#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h> // Debug

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
	sei();
	
	DDRB   |= _BV(5); // Debug
	DDRB   |= _BV(4); // Debug
}

volatile bool b = false;

void Loop() {
	
	b = true;
	char buffer[] = "hello world"; 
	communication::sender::Command command;
	command.id             = 0x66;
	command.payload_length = strlen(buffer);
	command.payload_buffer = (uint8_t*)buffer;
	communication::sender::send(command);
	
	_delay_ms(1000);
}

void communication::sender::onSendComplete() {
	if (b) {
		b = false;
		char buffer[] = "moon"; 
		communication::sender::Command command;
		command.id             = 0x10;
		command.payload_length = strlen(buffer);
		command.payload_buffer = (uint8_t*)buffer;
		communication::sender::send(command);
	}
}
