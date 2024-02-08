#include "../macros.hpp"
#include "receiver/command_info.hpp"

#include "commands.hpp"

Command<COMMAND_TYPE_BROADCAST, uint16_t> my_command;
bool onMyCommand(uint16_t) {return true;};
CommandInfo c1(my_command, onMyCommand);

struct Foo{int a; int b;};
Command<COMMAND_TYPE_BROADCAST, Foo> my_command_2;
bool onMyCommand2(const Foo&) {return true;};
CommandInfo c2(my_command_2, onMyCommand2);

Command<COMMAND_TYPE_STRIP, uint16_t> my_command_3;
bool onMyCommand_3(uint8_t, uint16_t) {return true;};
CommandInfo c3(my_command_3, onMyCommand_3);


PRIVATE COMMAND_INFO_DECL CommandInfo command_infos[] = {
	CommandInfo(my_command, onMyCommand),
	CommandInfo(my_command_2, onMyCommand2)
};

const CommandInfo * const CommandGetInfoGet(const uint8_t command_id) {
	if (command_id >= ARRAY_SIZE(command_infos)) {
		return nullptr;
	}
	return nullptr;
}
