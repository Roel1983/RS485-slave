#ifndef COMMAND_LIB_H
#define COMMAND_LIB_H

#include "Command.h"

#ifdef UNITTEST
void CommandLibReset();
#endif

extern command_t<COMMAND_TYPE_BROADCAST, uint16_t> cmd_broadcast;
bool OnReceive_cmd_broadcast(const uint16_t& block);
constexpr uint8_t COMMAND_ID_cmd_broadcast = 0;

extern command_t<COMMAND_TYPE_DEVICE, uint16_t> cmd_device;
bool OnReceive_cmd_device(const uint16_t& block);
constexpr uint8_t COMMAND_ID_cmd_device = 1;

extern command_t<COMMAND_TYPE_STRIP, uint16_t> cmd_strip;
bool OnReceive_cmd_strip(uint8_t relative_block_nr, const uint16_t& value);
constexpr uint8_t COMMAND_ID_cmd_strip = 2;

extern command_t<COMMAND_TYPE_BROADCAST, uint16_t> cmd_broadcast_isr_allowed;
bool OnReceive_cmd_broadcast_isr_allowed(const uint16_t& block);
constexpr uint8_t COMMAND_ID_cmd_broadcast_isr_allowed = 3;

#ifdef UNITTEST
extern COMMAND_INFO_DECL command_info_t command_infos[4];
#else
COMMAND_INFO_DECL command_info_t command_infos[] = {
	command_info_t(cmd_broadcast, OnReceive_cmd_broadcast),
	command_info_t(cmd_device   , OnReceive_cmd_device),
	command_info_t(cmd_strip    , OnReceive_cmd_strip),
	command_info_t(cmd_broadcast_isr_allowed, OnReceive_cmd_broadcast_isr_allowed, COMMAND_FIRE_ALLOWED_FROM_ISR)
};
#endif

#endif
