#include "Macros.h"

#include "CommandLib.h"

command_t<COMMAND_TYPE_BROADCAST, uint8_t> cmd_request_data;
command_t<COMMAND_TYPE_BROADCAST, uint8_t> cmd_request_data_nop_response;

command_t<COMMAND_TYPE_BROADCAST, uint16_t> cmd_broadcast;
command_t<COMMAND_TYPE_DEVICE,    uint16_t> cmd_device;
command_t<COMMAND_TYPE_STRIP,     uint16_t> cmd_strip;
command_t<COMMAND_TYPE_BROADCAST, uint16_t> cmd_broadcast_isr_allowed;

#ifdef UNITTEST

COMMAND_INFO_DECL command_info_t command_infos[] = {
	command_info_t(cmd_request_data, OnReceive_cmd_request_data),
	command_info_t(cmd_request_data_nop_response, OnReceive_cmd_request_data_nop_response),
	command_info_t(cmd_broadcast, OnReceive_cmd_broadcast),
	command_info_t(cmd_device,    OnReceive_cmd_device),
	command_info_t(cmd_strip,     OnReceive_cmd_strip),
	command_info_t(cmd_broadcast_isr_allowed, OnReceive_cmd_broadcast_isr_allowed, COMMAND_FIRE_ALLOWED_FROM_ISR)
};

void CommandLibReset() {
	for(int i = 0; i < ARRAY_SIZE(command_infos); i++) {
		command_infos[i].command.state = COMMAND_STATE_UNLOCKED;
	}
}
#endif
