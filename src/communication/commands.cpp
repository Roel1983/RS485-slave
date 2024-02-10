#include "../macros.hpp"
#include "receiver/command_info.hpp"

#include "commands.hpp"

namespace communication {

receiver::Command<COMMAND_TYPE_BROADCAST, uint16_t> my_command;
bool onMyCommand(uint16_t) {return true;};
receiver::CommandInfo c1(my_command, onMyCommand);

struct Foo{int a; int b;};
receiver::Command<COMMAND_TYPE_BROADCAST, Foo> my_command_2;
bool onMyCommand2(const Foo&) {return true;};
receiver::CommandInfo c2(my_command_2, onMyCommand2);

receiver::Command<COMMAND_TYPE_STRIP, uint16_t> my_command_3;
bool onMyCommand_3(uint8_t, uint16_t) {return true;};
receiver::CommandInfo c3(my_command_3, onMyCommand_3);

PRIVATE COMMAND_INFO_DECL receiver::CommandInfo command_infos[] = {
	receiver::CommandInfo(my_command, onMyCommand),
	receiver::CommandInfo(my_command_2, onMyCommand2)
};

const receiver::CommandInfo * const commandGetInfoGet(const uint8_t command_id) {
	if (command_id >= ARRAY_SIZE(command_infos)) {
		return nullptr;
	}
	return nullptr;
}

} // End of: namespace communication
