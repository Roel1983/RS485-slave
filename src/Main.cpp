#include <avr/interrupt.h>
#include <util/delay.h>

#include <string.h>

#include "Comm.h"
#include "Pins.h"

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
	CommBegin();
	CommSetDeviceNr(4);
	sei();
}

comm_send_message_t<42, uint8_t[12]> send_message_broadcast_response;
comm_send_message_t<66, uint8_t[4]>  send_message_comm_error_message;

void Loop() {
	CommLoop();
	
	comm_error_t comm_error = CommGetError();
	if (comm_error != COMM_ERROR_NONE) {
		if(CommCanUseSendMessage(send_message_comm_error_message)) {
			switch (comm_error) {
			case COMM_ERROR_NONE:
				memcpy((void*)&send_message_comm_error_message.value, "None", 4);
				break;
			case COMM_ERROR_SIGNAL:
				memcpy((void*)&send_message_comm_error_message.value, "Sign", 4);
				break;
			case COMM_ERROR_DATA:
				memcpy((void*)&send_message_comm_error_message.value, "Data", 4);
				break;
			case COMM_ERROR_BUSY:
				memcpy((void*)&send_message_comm_error_message.value, "Busy", 4);
				break;
			default:
				memcpy((void*)&send_message_comm_error_message.value, "othr", 4);
				break;
			}
			CommSend(send_message_comm_error_message);
		}
	}
}

#ifndef UNITTEST

bool OnReceive_cmd_broadcast(const uint16_t& value) {
	if(CommCanUseSendMessage(send_message_broadcast_response)) {
		memcpy((void*)&send_message_broadcast_response.value, "hello moon\n", 12);
		CommSend(send_message_broadcast_response);
	}
	return true;
}

bool OnReceive_cmd_device(const uint16_t& value) {
	return true;
}

bool OnReceive_cmd_strip(uint8_t relative_block_nr, const uint16_t& value) {
	return true;
}

bool OnReceive_cmd_broadcast_isr_allowed(const uint16_t& value) {
	return true;
}

#endif
