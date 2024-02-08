#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdint.h>

#include "../../macros.hpp"
#include "../communication.hpp"

#include "sender.hpp"

namespace communitation {
namespace sender {

typedef enum {
	STATE_IDLE
} State;

struct Isr {
	volatile State    state;
	uint8_t           crc;
	uint8_t           preamble_count;
	uint8_t           write_byte_count;
	const uint8_t *   write_byte_pos;
};
volatile Command command;

Isr isr;

PRIVATE INLINE bool sendBody();

#ifdef UNITTEST
void teardown() {
	
}
#endif

void setup() {
	
}

bool send(Command& new_command) {
	// TODO
	return false;
}

ISR(USART_UDRE_vect) {
	if (sendBody()) {
		return;
	}
	switch (isr.state) {
	
	}
}

PRIVATE INLINE bool sendBody() {
	if (!isr.write_byte_count) {
		return false;
	}
	--isr.write_byte_count;
	uint8_t data_byte = *(isr.write_byte_pos++);
	isr.crc -= data_byte;
	UDR0 = data_byte;
	return true;
}

}}
