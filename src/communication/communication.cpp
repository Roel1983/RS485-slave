#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdint.h>

#include "../macros.hpp"
#include "../timestamp.hpp"
#include "receiver/receiver.hpp"
#include "sender/sender.hpp"

#include "send_strategy.hpp"
#include "command_types.hpp"
#include "communication.hpp"

namespace communication {

enum SendBufferLock {
	SEND_BUFFER_UNLOCKED_FOR_WRITING,
	SEND_BUFFER_WRITE_LOCKED,
	SEND_BUFFER_UNLOCKED_FOR_READING,
	SEND_BUFFER_READ_LOCKED,
	SEND_BUFFER_CANCELED_LOCKED,
};
struct SendBuffer {
	volatile SendBufferLock lock;
	uint8_t        command_id;
	bool           is_addressable;
	uint8_t        block_nr;
	uint8_t        payload_size;
	PayloadWritter payload_writer;
};

PRIVATE constexpr uint8_t max_send_command_size = 40;
PRIVATE volatile SendBuffer send_buffer = { SEND_BUFFER_UNLOCKED_FOR_WRITING };
PRIVATE volatile uint32_t send_timestamp;

PRIVATE constexpr uint32_t send_timeout = 2000;

PRIVATE INLINE bool send(
	uint8_t        command_id,
	bool           is_addressable,
	uint8_t        block_nr,
	uint8_t        payload_size,
	PayloadWritter payload_writer);
PRIVATE bool sendFromBuffer(uint8_t *max_length);

#ifdef UNITTEST
void tearDown() {
	receiver::tearDown();
	sender::tearDown();
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
	       
	send_strategy::setup();
	receiver::setup();
	sender::setup();
}

void loop() {
	send_strategy::loop();
	receiver::loop();
	
	cli();
	if (send_buffer.lock == SEND_BUFFER_UNLOCKED_FOR_READING) {
		uint32_t current_timetamp = timestamp::getMsTimestamp();
		if (current_timetamp - send_timestamp > send_timeout) {
			send_buffer.lock = SEND_BUFFER_CANCELED_LOCKED;
		}
	};
	sei();
	if (send_buffer.lock == SEND_BUFFER_CANCELED_LOCKED) {
		uint8_t length = 0;
		send_buffer.lock = send_buffer.payload_writer(true, length, nullptr)
			? SEND_BUFFER_UNLOCKED_FOR_WRITING
			: SEND_BUFFER_UNLOCKED_FOR_READING;
	}	
}

bool sendBroadcast(
	uint8_t        command_id,
	uint8_t        payload_size,
	PayloadWritter payload_writer
) {
	return send(command_id, false,	0, payload_size, payload_writer);
}
	
bool sendAddressable(
	uint8_t        command_id,
	uint8_t        block_nr,
	uint8_t        payload_size,
	PayloadWritter payload_writer
) {
	return send(command_id, true, block_nr, payload_size, payload_writer);
}

PRIVATE INLINE bool send(
	uint8_t        command_id,
	bool           is_addressable,
	uint8_t        block_nr,
	uint8_t        payload_size,
	PayloadWritter payload_writer
) {
	if (payload_size >= max_send_command_size) {
		return false;
	}
	{ 	
		cli();
		if (send_buffer.lock != SEND_BUFFER_UNLOCKED_FOR_WRITING) {
			sei();
			return false;
		}
		send_buffer.lock = SEND_BUFFER_WRITE_LOCKED;
		send_timestamp   = timestamp::getMsTimestamp();
		sei();
	}
	
	send_buffer.command_id     = command_id;
	send_buffer.is_addressable = is_addressable;
	send_buffer.block_nr       = block_nr;
	send_buffer.payload_size   = payload_size;
	send_buffer.payload_writer = payload_writer;
	
	send_buffer.lock = SEND_BUFFER_UNLOCKED_FOR_READING;
	if (send_strategy::get() == send_strategy::STRATEGY_SEND_AT_WILL && !sender::is_sending()) {
		(void)sendFromBuffer(nullptr);
	}
	return true;	
}

PRIVATE bool sendFromBuffer(uint8_t *max_length) {
	static communication::sender::Command send_command;
	
	{
		cli();
		if (send_buffer.lock != SEND_BUFFER_UNLOCKED_FOR_READING) {
			sei();
			if(max_length) *max_length = 0;
			return false;
		}
		send_buffer.lock = SEND_BUFFER_READ_LOCKED;
		sei();
	}
	if (max_length && send_buffer.payload_size > *max_length) {
		if(max_length) *max_length = send_buffer.payload_size;
		send_buffer.lock = SEND_BUFFER_UNLOCKED_FOR_READING;
		return false;
	}
	
	static uint8_t buffer[max_send_command_size + 1];
	
	if (send_buffer.is_addressable) {
		buffer[0] = send_buffer.block_nr;
		send_command.payload_buffer = &buffer[1];
	} else {
		send_command.payload_buffer = &buffer[0];
	}
	send_command.payload_length = send_buffer.payload_size;
	if (!send_buffer.payload_writer(false, send_command.payload_length, send_command.payload_buffer)) {
		if(max_length) *max_length = 0;
		send_buffer.lock = SEND_BUFFER_UNLOCKED_FOR_WRITING;
		return false;
	};
	send_command.id = send_buffer.command_id;
	send_buffer.lock = SEND_BUFFER_UNLOCKED_FOR_WRITING;
	communication::sender::send(send_command);
	return true;
}

void onStrategyChangedToSendAtWill() {
	if(!sender::is_sending()) {
		(void)sendFromBuffer(nullptr);
	}
}

void onSendComplete() {
	if (send_strategy::get() == send_strategy::STRATEGY_SEND_AT_WILL) {
		(void)sendFromBuffer(nullptr);
	}
}

PRIVATE void sendRequestToSendResponse(uint8_t requested_length) {
	static uint8_t _requested_length;
	static communication::sender::Command request_to_send_response = {
		.id             = 0x01,
		.payload_length = sizeof(_requested_length),
		.payload_buffer = &_requested_length
	};

	_requested_length = requested_length; 
	communication::sender::send(request_to_send_response);
}

struct RequestToSendCommand {
	uint8_t unique_id;
	uint8_t max_length;
};
communication::receiver::Command<COMMAND_TYPE_BROADCAST, RequestToSendCommand> request_to_send_command;
bool onRequestToSendCommand(const RequestToSendCommand& payload) {
	send_strategy::setOnDemand();
	if (payload.unique_id == commandTypeGetBlockNr(COMMAND_TYPE_UNIQUE_ID)) {
		uint8_t length = payload.max_length;
		if(!sendFromBuffer(&length)) {
			sendRequestToSendResponse(length);
		}
	}
	return true;
};
communication::receiver::CommandInfo request_to_send_command_info(
	request_to_send_command,
	onRequestToSendCommand,
	true);

} // namespace communitation
