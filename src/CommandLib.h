#ifndef COMMAND_LIB_H
#define COMMAND_LIB_H

#include "Command.h"

extern command_t<COMMAND_TYPE_BROADCAST, uint16_t> command1;
void OnCommand1(uint16_t& value);

extern command_t<COMMAND_TYPE_DEVICE, uint16_t> command2;
void OnCommand2(uint16_t& value);

extern command_t<COMMAND_TYPE_STRIP, uint16_t> command3;
void OnCommand3(uint8_t from_index, uint8_t to_index, uint16_t values[4]);

constexpr command_info_t command_infos[] = {
	command_info_t(command1, OnCommand1),
	command_info_t(command2, OnCommand2),
	command_info_t(command3, OnCommand3)
};

#endif
