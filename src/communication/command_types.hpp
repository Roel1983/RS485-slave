#ifndef COMMUNICATION_COMMANDTYPES_H_
#define COMMUNICATION_COMMANDTYPES_H_

#include <assert.h>
#include <stdint.h>

namespace communication {
	
enum CommandType {
	COMMAND_TYPE_BROADCAST,
	COMMAND_TYPE_UNIQUE_ID,
	COMMAND_TYPE_DEVICE,
	COMMAND_TYPE_GROUP,
	COMMAND_TYPE_SUN,
	COMMAND_TYPE_STRIP,
};
constexpr uint8_t COMMAND_TYPE_COUNT = 6;

constexpr uint8_t commandTypeGetBlockCount(const CommandType type) {
	assert(type >= 0 && type < COMMAND_TYPE_COUNT);
	return (type == COMMAND_TYPE_STRIP) ? 4 : 1;
}

uint8_t commandTypeGetBlockNr   (const CommandType type);
void    commandTypeSetBlockNr   (const CommandType type, const uint8_t nr);

} // namespace communication

#endif  // COMMUNICATION_COMMANDTYPES_H_
