#include <assert.h>
#include <stdint.h>

#include "../macros.hpp"

#include "command_types.hpp"

PRIVATE uint8_t block_nr[COMMAND_TYPE_COUNT] = {0};

uint8_t CommandTypeGetBlockNr(const CommandType type) {
	assert(type >= 0 && type < COMMAND_TYPE_COUNT);
	
	return block_nr[type];
}

void CommandTypeSetBlockNr(const CommandType type, const uint8_t nr) {
	block_nr[type] = nr;
}
