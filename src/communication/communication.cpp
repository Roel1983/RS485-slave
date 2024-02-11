//#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdint.h>

#include "../macros.hpp"
#include "receiver/receiver.hpp"
#include "sender/sender.hpp"

#include "communication.hpp"

namespace communication {

#ifdef UNITTEST
void teardown() {
	receiver::teardown();
	sender::teardown();
}
#endif

void setup() {
	constexpr uint16_t baudrate_prescaler = (F_CPU / (8UL * BAUDRATE)) - 1;
	
	UBRR0H  = baudrate_prescaler >> 8;
	UBRR0L  = baudrate_prescaler;
	
	UCSR0A |= (1<<U2X0);
	
	UCSR0C = (0<<UMSEL00)
	       | (0<<UPM00)
	       | (0<<USBS0)
	       | (3<<UCSZ00);
	       
    receiver::setup();
	sender::setup();
}

void loop() {
	// TODO
}

communication::receiver::Command<COMMAND_TYPE_BROADCAST, uint16_t> request_to_send_command;
bool onRequestToSendCommand(uint16_t) {
	PORTB |= _BV(4); PORTB &= ~_BV(4);// DEBUG
	return true;
};

} // namespace communitation
