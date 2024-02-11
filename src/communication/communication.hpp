#ifndef COMMUNICATION_COMMUNICATION_H_
#define COMMUNICATION_COMMUNICATION_H_

#include "stdint.h"

#include "receiver/command.hpp"
#include "receiver/command_info.hpp"

namespace communication {
	
constexpr uint32_t BAUDRATE       = 115200;
constexpr uint8_t  PREAMBLE_BYTE  = 0x55;
constexpr uint8_t  PREAMBLE_COUNT = 2;
constexpr uint8_t  EXTENDED_PAYLOAD_LENGHT_MASK = 0x80;

#ifdef UNITTEST
void teardown();
#endif

void setup();
void loop();

extern communication::receiver::Command<COMMAND_TYPE_BROADCAST, uint16_t> request_to_send_command;
extern bool onRequestToSendCommand(uint16_t);
constexpr communication::receiver::CommandInfo request_to_send_command_info(
	request_to_send_command,
	onRequestToSendCommand, true);

} // namespace communitation

#endif // COMMUNICATION_COMMUNICATION_H_
