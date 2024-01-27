#ifndef COMMAND_LIB_H
#define COMMAND_LIB_H

#include "Command.h"

#ifdef UNITTEST
void CommandLibReset();
#endif

extern command_t<COMMAND_TYPE_BROADCAST, uint16_t> cmd_broadcast;
void OnReceive_cmd_broadcast(uint16_t& block);
constexpr uint8_t COMMAND_ID_cmd_broadcast = 0;

extern command_t<COMMAND_TYPE_DEVICE, uint16_t> cmd_device;
void OnReceive_cmd_device(uint16_t& block);
constexpr uint8_t COMMAND_ID_cmd_device = 1;

extern command_t<COMMAND_TYPE_STRIP, uint16_t> cmd_strip;
void OnReceive_cmd_strip(uint8_t from_index, uint8_t to_index, uint16_t blocks[4]);
constexpr uint8_t COMMAND_ID_cmd_strip = 2;

#ifdef UNITTEST
extern COMMAND_INFO_DECL command_info_t command_infos[3];
#else
COMMAND_INFO_DECL command_info_t command_infos[] = {
	command_info_t(cmd_broadcast, OnReceive_cmd_broadcast),
	command_info_t(cmd_device   , OnReceive_cmd_device),
	command_info_t(cmd_strip    , OnReceive_cmd_strip)
};
#endif

#endif
