#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdint.h>
#include <string.h>

#include "../../macros.hpp"
#include "../communication.hpp"
#include "../command_types.hpp"

#include "hal.hpp"
#include "sender.hpp"

using namespace communication;

namespace communication {
namespace sender {

typedef enum {
	STATE_IDLE,
	STATE_PREAMBLE,
	STATE_SENDER_UNIQUE_ID,
	STATE_LENGTH,
	STATE_COMMAND_ID,
	STATE_CRC,
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

PRIVATE INLINE void sendPreamble();
PRIVATE INLINE void sendSenderUniqueId();
PRIVATE INLINE void sendLength();
PRIVATE INLINE void sendCommandId();
PRIVATE INLINE bool sendBody();
PRIVATE INLINE void sendCrc();

#ifdef UNITTEST
void teardown() {
	memset((void*)&command, 0, sizeof(command));
	memset(&isr, 0, sizeof(isr));
}
#endif

void setup() {
	memset(&isr, 0, sizeof(isr));
	hal::setup();
}

bool send(Command& new_command) {
	if (isr.state != STATE_IDLE) {
		return false;
	}
	memcpy((void*)&command, &new_command, sizeof(command));
	isr.preamble_count = 0;
	isr.state          = STATE_PREAMBLE;
	
	hal::txEnable();
	hal::enableReadyToSendInterrupt();
	return true;
}

PRIVATE INLINE void hal::onReadyToSendNextByte() {
	if (sendBody()) {
		return;
	}
	switch (isr.state) {
	case STATE_IDLE:
		return;
	case STATE_PREAMBLE:
		sendPreamble();
		return;
	case STATE_SENDER_UNIQUE_ID:
		sendSenderUniqueId();
		return;
	case STATE_LENGTH:
		sendLength();
		return;
	case STATE_COMMAND_ID:
		sendCommandId();
		return;
	case STATE_CRC:
		sendCrc();
		return;
	}
}

PRIVATE INLINE void sendPreamble() {
	if (++isr.preamble_count >= PREAMBLE_COUNT) {
		isr.state = STATE_SENDER_UNIQUE_ID;
	}
	hal::sendByte(PREAMBLE_BYTE);
}

PRIVATE INLINE void sendSenderUniqueId() {
	const uint8_t sender_unique_id = commandTypeGetBlockNr(COMMAND_TYPE_UNIQUE);
	isr.crc   = sender_unique_id;
	hal::sendByte(sender_unique_id);
	isr.state = STATE_LENGTH;
}

PRIVATE INLINE void sendLength() {
	const uint8_t length = command.payload_length + 1;
	assert((length & EXTENDED_PAYLOAD_LENGHT_MASK) == 0);
	isr.crc   += length;
	hal::sendByte(length);
	isr.state  = STATE_COMMAND_ID;
}

PRIVATE INLINE void sendCommandId() {
	isr.write_byte_count = command.payload_length;
	isr.write_byte_pos   = command.payload_buffer;
	isr.crc             += command.id;
	hal::sendByte(command.id);
	isr.state            = STATE_CRC;
}

PRIVATE INLINE bool sendBody() {
	if (!isr.write_byte_count) {
		return false;
	}
	--isr.write_byte_count;
	uint8_t data_byte = *(isr.write_byte_pos++);
	isr.crc -= data_byte;
	hal::sendByte(data_byte);
	return true;
}

PRIVATE INLINE void sendCrc() {
	hal::sendByte(0x00 - isr.crc);
	hal::disableReadyToSendInterrupt();
}

ISR(USART_TX_vect) {
	isr.state = STATE_IDLE;
	onSendComplete();
	if (isr.state != STATE_PREAMBLE) {
		hal::txDisable();
	}
}

PRIVATE void __onSendComplete() __attribute__ ((unused));
PRIVATE void __onSendComplete() {}
void onSendComplete() __attribute__ ((weak, alias ("_ZN13communication6senderL16__onSendCompleteEv")));

}}
