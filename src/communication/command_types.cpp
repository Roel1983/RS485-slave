#include <assert.h>
#include <stdint.h>

#include "../macros.hpp"

#include "command_types.hpp"

namespace communication {
	
PRIVATE uint8_t block_nr[COMMAND_TYPE_COUNT] = {0};

uint8_t commandTypeGetBlockNr(const CommandType type) {
	assert(type > 0 && type <= COMMAND_TYPE_COUNT);
	
	return block_nr[type - 1];
}

void commandTypeSetBlockNr(const CommandType type, const uint8_t nr) {
	assert(type > 0 && type <= COMMAND_TYPE_COUNT);
	block_nr[type - 1] = nr;
}

} // namespace communication
