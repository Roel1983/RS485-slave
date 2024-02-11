#include <avr/io.h> // Debug

#include "../macros.hpp"
#include "receiver/command_info.hpp"

#include "communication.hpp"
#include "commands.hpp"

using namespace communication;

namespace communication {

struct Foo{int a; int b;};
receiver::Command<COMMAND_TYPE_BROADCAST, Foo> my_2_command;
bool onMy2Command(const Foo&) {return true;};
receiver::CommandInfo my_2_command_info(my_2_command, onMy2Command);

receiver::Command<COMMAND_TYPE_STRIP, uint16_t> my_command_3;
bool onMyCommand_3(uint8_t, uint16_t) {return true;};

PRIVATE COMMAND_INFO_DECL const receiver::CommandInfo* command_infos[] = {
	&request_to_send_command_info,
	nullptr,
	&my_2_command_info
};

const receiver::CommandInfo * const commandGetInfoGet(const uint8_t command_id) {
	if (command_id >= ARRAY_SIZE(command_infos)) {
		return nullptr;
	}
	return command_infos[command_id];
}

} // End of: namespace communication
