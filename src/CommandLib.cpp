#include "CommandLib.h"

command_t<COMMAND_TYPE_BROADCAST, uint16_t> command1;
command_t<COMMAND_TYPE_DEVICE, uint16_t> command2;
command_t<COMMAND_TYPE_STRIP, uint16_t> command3;

void OnCommand1(uint16_t& value) {
}

void OnCommand2(uint16_t& value) {
}

void OnCommand3(uint8_t from_index, uint8_t to_index, uint16_t values[4]) {
	
}
