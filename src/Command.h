#ifndef COMMAND_H_
#define COMMAND_H_

#include <stdint.h>

#ifdef UNITTEST
#define COMMAND_INFO_DECL 
#else
#define COMMAND_INFO_DECL constexpr
#endif

typedef enum {
	COMMAND_TYPE_BROADCAST,
	COMMAND_TYPE_DEVICE,
	COMMAND_TYPE_STRIP,
} command_type_t;
static constexpr uint8_t command_type_block_counts[] = {1, 1, 4};

constexpr uint8_t CommandTypeGetBlockCount(const int type) {
	return command_type_block_counts[type];
};

typedef enum {
	COMMAND_STATE_UNLOCKED,
	COMMAND_STATE_WRITE_LOCKED,
	COMMAND_STATE_READ_LOCKED
} command_state_t;

struct command_base_t {
	command_state_t state;
	uint8_t         block_from;
	uint8_t         block_to;
	uint8_t         buffer[0];
};

template <
	command_type_t _type,
	typename       T
> struct command_t : public command_base_t {
	using Type = T;
	static constexpr command_type_t type = _type;
	
	T buffer[CommandTypeGetBlockCount(_type)];
};

template<typename T>
struct on_received_single_block {
	typedef void (*function_t)(T& block);
};

template<typename T, uint8_t N>
struct on_received_multi_block {
	typedef void (*function_t)(uint8_t block_from, uint8_t block_to, T block[N]);
};

template<typename T, uint8_t N>
struct on_received_trait {
	using on_received_function_t = typename on_received_multi_block<T, N>::function_t;
};

template<typename T>
struct on_received_trait<T, 1> {
	using on_received_function_t = typename on_received_single_block<T>::function_t;
};

struct command_info_t {
	command_type_t  type;
	uint8_t         block_size;
	command_base_t& command;
	union {
		void* _needed_for_mem_initializer;
		void (*single_block)(const void* buffer);
		void (*multi_block)(uint8_t block_from, uint8_t block_to, const void* buffer);
	} on_received_function;

	template <
		command_type_t _type,
		typename       T
	> COMMAND_INFO_DECL command_info_t(
		command_t<_type, T>& _command,
		typename on_received_trait<T, CommandTypeGetBlockCount(_type)>::on_received_function_t function
	) : type(_type)
	  , block_size(sizeof(T))
	  , command(_command)
	  , on_received_function{ reinterpret_cast<void*>(function) }
	{}
};

#endif

