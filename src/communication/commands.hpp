#ifndef COMMUNICATION_COMMANDS_H_
#define COMMUNICATION_COMMANDS_H_

#include <stdint.h>

#include "receiver/command_info.hpp"

const CommandInfo * const CommandGetInfoGet(const uint8_t command_id);

#endif // COMMUNICATION_COMMANDS_H_
