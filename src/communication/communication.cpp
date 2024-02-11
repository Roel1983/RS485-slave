#include <avr/io.h>

#include <stdint.h>

#include "../macros.hpp"
#include "receiver/receiver.hpp"
#include "sender/sender.hpp"

#include "command_types.hpp"
#include "communication.hpp"

namespace communication {

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
	       
    receiver::setup();
	sender::setup();
}

void loop() {
	// TODO
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
	if (payload.unique_id == commandTypeGetBlockNr(COMMAND_TYPE_UNIQUE)) {
		sendRequestToSendResponse(0);
	}
	return true;
};
communication::receiver::CommandInfo request_to_send_command_info(
	request_to_send_command,
	onRequestToSendCommand,
	true);

} // namespace communitation
