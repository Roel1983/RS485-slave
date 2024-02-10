#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdint.h>
#include <string.h>

#include "../../macros.hpp"
#include "../communication.hpp"
#include "../command_types.hpp"

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

PRIVATE INLINE void enableReadyToSendInterrupt();
PRIVATE INLINE void disableReadyToSendInterrupt();
PRIVATE INLINE void sendByte(const uint8_t data_byte);
PRIVATE INLINE void txSetup();
PRIVATE INLINE void txEnable();
PRIVATE INLINE void txDisable();

#ifdef UNITTEST
void teardown() {
	memset((void*)&command, 0, sizeof(command));
	comm_send_txen = false;
	memset(&isr, 0, sizeof(isr));
}
#endif

void setup() {
	memset(&isr, 0, sizeof(isr));
	txSetup();
}

bool send(Command& new_command) {
	if (isr.state != STATE_IDLE) {
		return false;
	}
	memcpy((void*)&command, &new_command, sizeof(command));
	isr.preamble_count = 0;
	isr.state          = STATE_PREAMBLE;
	
	txEnable();
	enableReadyToSendInterrupt();
	return true;
}

ISR(USART_UDRE_vect) {
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
	sendByte(PREAMBLE_BYTE);
}

PRIVATE INLINE void sendSenderUniqueId() {
	const uint8_t sender_unique_id = commandTypeGetBlockNr(COMMAND_TYPE_UNIQUE);
	isr.crc   = sender_unique_id;
	sendByte(sender_unique_id);
	isr.state = STATE_LENGTH;
}

PRIVATE INLINE void sendLength() {
	const uint8_t length = command.payload_length + 1;
	assert((length & EXTENDED_PAYLOAD_LENGHT_MASK) == 0);
	isr.crc   += length;
	sendByte(length);
	isr.state  = STATE_COMMAND_ID;
}

PRIVATE INLINE void sendCommandId() {
	isr.write_byte_count = command.payload_length;
	isr.write_byte_pos   = command.payload_buffer;
	isr.crc             += command.id;
	sendByte(command.id);
	isr.state            = STATE_CRC;
}

PRIVATE INLINE bool sendBody() {
	if (!isr.write_byte_count) {
		return false;
	}
	--isr.write_byte_count;
	uint8_t data_byte = *(isr.write_byte_pos++);
	isr.crc -= data_byte;
	sendByte(data_byte);
	return true;
}

PRIVATE INLINE void sendCrc() {
	sendByte(0x00 - isr.crc);
	disableReadyToSendInterrupt();
}

ISR(USART_TX_vect) {
	isr.state = STATE_IDLE;
	onSendComplete();
	if (isr.state != STATE_PREAMBLE) {
		txDisable();
	}
}

PRIVATE INLINE void enableReadyToSendInterrupt() {
	UCSR0B |= _BV(UDRIE0);
}

PRIVATE INLINE void disableReadyToSendInterrupt() {
	UCSR0B &= ~_BV(UDRIE0);
}

PRIVATE INLINE void sendByte(const uint8_t data_byte) {
	UDR0 = data_byte;
}

PRIVATE INLINE void txSetup() {
	DDRC |= _BV(3);
}

PRIVATE INLINE void txEnable() {
	PORTC |= _BV(3); // Possible race condition
}

PRIVATE INLINE void txDisable() {
	PORTC &= ~_BV(3); // Possible race condition
}

PRIVATE void __onSendComplete() __attribute__ ((unused));
PRIVATE void __onSendComplete() {}
void onSendComplete() __attribute__ ((weak, alias ("_ZN13communication6senderL16__onSendCompleteEv")));

}}
