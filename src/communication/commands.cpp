#include <avr/io.h> // Debug

#include "../macros.hpp"
#include "receiver/command_info.hpp"

#include "communication.hpp"
#include "commands.hpp"

using namespace communication;

namespace communication {

communication::receiver::Command<COMMAND_TYPE_BROADCAST, uint8_t> orange_led_command;
bool onOrangeLedCommand(uint8_t payload) {
	DDRB |= _BV(3); // Debug
	PORTB ^= _BV(3);
	return true;
};
communication::receiver::CommandInfo orange_led_command_info(
	orange_led_command,
	onOrangeLedCommand);

PRIVATE COMMAND_INFO_DECL const receiver::CommandInfo* command_infos[] = {
	&request_to_send_command_info,
	nullptr,
	&orange_led_command_info
};

const receiver::CommandInfo * const getCommandInfo(const uint8_t command_id) {
	if (command_id >= ARRAY_SIZE(command_infos)) {
		return nullptr;
	}
	return command_infos[command_id];
}

uint8_t getCommandInfoCount() {
	return ARRAY_SIZE(command_infos);
}

} // End of: namespace communication
