//#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdint.h>

#include "../macros.hpp"
#include "receiver/receiver.hpp"
#include "sender/sender.hpp"

#include "communication.hpp"

namespace communitation {

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
	
	UCSR0B = (1<<RXEN0)
	       | (1<<TXEN0)
	       | (1<<TXCIE0)
	       | (1<<RXCIE0);
	
	receiver::setup();
	sender::setup();
}

void loop() {
	// TODO
}

} // namespace communitation
