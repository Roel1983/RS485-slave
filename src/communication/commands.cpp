#include <avr/io.h> // Debug

#include "../macros.hpp"
#include "receiver/command_info.hpp"

#include "communication.hpp"
#include "commands.hpp"

using namespace communication;

namespace communication {

PRIVATE COMMAND_INFO_DECL const receiver::CommandInfo* command_infos[] = {
	&request_to_send_command_info,
	nullptr
};

const receiver::CommandInfo * const commandGetInfoGet(const uint8_t command_id) {
	if (command_id >= ARRAY_SIZE(command_infos)) {
		return nullptr;
	}
	return command_infos[command_id];
}

} // End of: namespace communication
