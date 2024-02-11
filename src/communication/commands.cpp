#include <avr/io.h> // Debug

#include "../macros.hpp"
#include "receiver/command_info.hpp"

#include "communication.hpp"
#include "commands.hpp"

using namespace communication;

namespace communication {

struct Foo{int a; int b;};
receiver::Command<COMMAND_TYPE_BROADCAST, Foo> my_command_2;
bool onMyCommand2(const Foo&) {return true;};

receiver::Command<COMMAND_TYPE_STRIP, uint16_t> my_command_3;
bool onMyCommand_3(uint8_t, uint16_t) {return true;};

PRIVATE COMMAND_INFO_DECL receiver::CommandInfo command_infos[] = {
	request_to_send_command_info,
	receiver::CommandInfo(my_command_2, onMyCommand2)
};

const receiver::CommandInfo * const commandGetInfoGet(const uint8_t command_id) {
	if (command_id >= ARRAY_SIZE(command_infos)) {
		return nullptr;
	}
	return &command_infos[command_id];
}

} // End of: namespace communication
