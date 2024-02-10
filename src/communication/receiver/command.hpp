#ifndef COMMUNICATION_COMMAND_H_
#define COMMUNICATION_COMMAND_H_

#include "../command_types.hpp"

using namespace communication;

namespace communication {
namespace receiver {

enum CommandLock {
	COMMAND_LOCK_NONE,
	COMMAND_LOCK_WRITE,
	COMMAND_LOCK_READ,
};

struct CommandBase{
	CommandLock lock;
	uint8_t processed_block_bits;
	uint8_t __attribute__ ((aligned)) buffer[0];
} ;

template <
	CommandType   _command_type,
	typename      T
> struct Command : public CommandBase {
	using Type = T;
	static constexpr CommandType command_type = _command_type;
	static constexpr uint8_t     block_size   = sizeof(T);
	T buffer[commandTypeGetBlockCount(_command_type)];
};

template <
	CommandType _command_type
> struct Command<_command_type, void> : public CommandBase {
	using Type = void;
	static constexpr CommandType command_type = _command_type;
	static constexpr uint8_t     block_size   = 0;
};

}} // End of: communication::receiver

#endif  // COMMUNICATION_COMMAND_H_
