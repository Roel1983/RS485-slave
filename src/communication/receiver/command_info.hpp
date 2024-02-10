#ifndef COMMUNICATION_COMMANDINFO_H_
#define COMMUNICATION_COMMANDINFO_H_

#include "../command_types.hpp"
#include "command.hpp"
#include "command_notifier.hpp"

#ifdef UNITTEST
#define COMMAND_INFO_DECL 
#else
#define COMMAND_INFO_DECL constexpr
#endif

using namespace communication;

namespace communication {
namespace receiver {

struct CommandInfo{
	CommandType  type;
	uint16_t     block_size;
	bool         is_allow_notify_from_isr;
	CommandBase& command;

private:	
	void*        function;
	bool (*on_received)(void *func, const uint8_t relative_block_nr, const uint8_t * const block);

public:	
	template <
		CommandType _command_type,
		typename T
	> COMMAND_INFO_DECL CommandInfo(
		Command<_command_type, T>& _command,
		typename CommandNotifierTrait<T, commandTypeGetBlockCount(_command_type)>::Notifier::OnReceivedFunc func,
		bool _is_allow_notify_from_isr = false
	) : type(_command_type)
	  , block_size(Command<_command_type, T>::block_size)
	  , is_allow_notify_from_isr(_is_allow_notify_from_isr)
	  , command(_command)
	  , function((void*)func)
	  , on_received(CommandNotifierTrait<T, commandTypeGetBlockCount(_command_type)>::Notifier::onReceived)
	{}
	
	inline bool onReceived(uint8_t relative_block_nr, const uint8_t * const block) const {
		return on_received(function, relative_block_nr, block);
	}
};
CommandInfo* CommandGetInfo(const uint8_t command_id);

}} // End of: communication::receiver

#endif // COMMUNICATION_COMMANDINFO_H_
