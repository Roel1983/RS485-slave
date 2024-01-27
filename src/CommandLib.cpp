#include "Macros.h"

#include "CommandLib.h"

command_t<COMMAND_TYPE_BROADCAST, uint16_t> cmd_broadcast;
command_t<COMMAND_TYPE_DEVICE,    uint16_t> cmd_device;
command_t<COMMAND_TYPE_STRIP,     uint16_t> cmd_strip;

#ifdef UNITTEST

COMMAND_INFO_DECL command_info_t command_infos[] = {
	command_info_t(cmd_broadcast, OnReceive_cmd_broadcast),
	command_info_t(cmd_device,    OnReceive_cmd_device),
	command_info_t(cmd_strip,     OnReceive_cmd_strip)
};

void CommandLibReset() {
	for(int i = 0; i < ARRAY_SIZE(command_infos); i++) {
		command_infos[i].command.state = COMMAND_STATE_UNLOCKED;
	}
}
#else

void OnReceive_cmd_broadcast(uint16_t& value) {
}

void OnReceive_cmd_device(uint16_t& value) {
}

void OnReceive_cmd_strip(uint8_t from_index, uint8_t to_index, uint16_t values[4]) {
	
}

#endif


