#ifndef COMMUNICATION_COMMANDS_H_
#define COMMUNICATION_COMMANDS_H_

#include <stdint.h>

#include "receiver/command_info.hpp"

namespace communication {

const receiver::CommandInfo * const commandGetInfoGet(const uint8_t command_id);

} // End of: namespace communication

#endif // COMMUNICATION_COMMANDS_H_
