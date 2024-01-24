#include <string.h> 

#include "Comm.h"

uint8_t DeviceIdGet() {
	return 4;
}
uint8_t StripOffsetGet() {
	return 16;
}

typedef enum {
	COMM_RECEIVE_STATE_SYNC,
	COMM_RECEIVE_STATE_COMMAND_ID,
	COMM_RECEIVE_STATE_ADDRESS,
	COMM_RECEIVE_STATE_COUNT,
	COMM_RECEIVE_STATE_CRC,
} comm_receive_state_t;

typedef struct  {
	uint8_t (*get_addr)(void);
	uint8_t addr_count;
} command_type_info_t;

typedef enum {
	COMMAND_TYPE_BROADCAST,
	COMMAND_TYPE_DEVICE,
	COMMAND_TYPE_STRIP
} command_type_t;

typedef enum {
	COMMAND_STATE_UNLOCKED,
	COMMAND_STATE_WRITE_LOCKED,
	COMMAND_STATE_READ_LOCKED,
} command_state_t;

typedef const struct {
	const command_type_t type;
	uint8_t * const      receive_buffer;
	const uint8_t        receive_buffer_size;
	uint8_t * const      state;
} comm_receive_command_t;

PRIVATE const uint8_t COMM_RECEIVE_PREAMBLE_BYTE  = 0xcc;
PRIVATE const uint8_t COMM_RECEIVE_PREAMBLE_COUNT = 2;

PRIVATE uint8_t   comm_receive_crc               = 0;
PRIVATE uint16_t  comm_receive_skip_before_count = 0;
PRIVATE uint16_t  comm_receive_skip_after_count  = 0;
PRIVATE uint16_t  comm_receive_write_count       = 0;
PRIVATE uint8_t  *comm_receive_write_pos         = 0;
PRIVATE uint8_t   comm_receive_address           = 0;

PRIVATE uint8_t   comm_receive_error_state       = 0;

PRIVATE comm_receive_state_t comm_receive_state            = COMM_RECEIVE_STATE_SYNC;
PRIVATE uint8_t              comm_receive_preamble_counter = 0;

PRIVATE const command_type_info_t cmd_type_infos[] = {
	{DeviceIdGet, 1},
	{StripOffsetGet, 4},
};

PRIVATE comm_receive_command_t *comm_receive_command = 0;

PRIVATE uint8_t command1_state = COMMAND_STATE_UNLOCKED;
PRIVATE uint8_t command1_buffer[4];
PRIVATE void (*command1_on_recv_func)(uint8_t buffer[4]) = 0;
PRIVATE const comm_receive_command_t command1 = {
	COMMAND_TYPE_BROADCAST,
	command1_buffer,
	sizeof(command1_buffer),
	&command1_state
};

PRIVATE uint8_t command2_state = COMMAND_STATE_UNLOCKED;
PRIVATE uint8_t command2_buffer[4];
PRIVATE void (*command2_on_recv_func)(uint8_t buffer[4]) = 0;
PRIVATE const comm_receive_command_t command2 = {
	COMMAND_TYPE_DEVICE,
	command2_buffer,
	sizeof(command2_buffer),
	&command2_state
};

PRIVATE uint8_t command3_state = COMMAND_STATE_UNLOCKED;
PRIVATE uint8_t command3_buffer[2*4];
PRIVATE void (*command3_on_recv_func)(uint8_t buffer[1]) = 0;
PRIVATE const comm_receive_command_t command3 = {
	COMMAND_TYPE_STRIP,
	command3_buffer,
	2,
	&command3_state
};

PRIVATE const comm_receive_command_t* comm_receive_commands[] = {	
	&command1,
	&command2,
	&command3,
};

#ifdef UNITTEST
void CommReceiveUnittestReset() {
	comm_receive_crc               = 0;
	comm_receive_skip_before_count = 0;
	comm_receive_skip_after_count  = 0;
	comm_receive_write_count       = 0;
	comm_receive_write_pos         = 0;
	comm_receive_address           = 0;
	
	comm_receive_error_state       = 0;

	comm_receive_state            = COMM_RECEIVE_STATE_SYNC;
	comm_receive_preamble_counter = 0;

	comm_receive_command = 0;

	command1_state = COMMAND_STATE_UNLOCKED;
	memset(command1_buffer, 0, 4);
	command1_on_recv_func = 0;

	command2_state = COMMAND_STATE_UNLOCKED;
	memset(command2_buffer, 0, 4);
	command2_on_recv_func = 0;

	command3_state = COMMAND_STATE_UNLOCKED;
	memset(command3_buffer, 0, 2*4);
	command3_on_recv_func = 0;
}
#endif

void CommReceiveByte(uint8_t b) {
	
	comm_receive_crc += b;
	
	if (comm_receive_skip_before_count) {
		--comm_receive_skip_before_count;
		return;
	}
	if (comm_receive_write_count) {
		--comm_receive_write_count;
		*(comm_receive_write_pos++) = b;
		return;
	}
	if (comm_receive_skip_after_count) {
		comm_receive_skip_before_count = comm_receive_skip_after_count - 1;
		comm_receive_skip_after_count = 0;
		return;
	}
	
	switch(comm_receive_state) {
	case COMM_RECEIVE_STATE_SYNC:
		
		if(b != COMM_RECEIVE_PREAMBLE_BYTE) {
			comm_receive_preamble_counter = 0;
			if(!comm_receive_error_state) {
				comm_receive_error_state = COMM_RECEIVE_ERROR_STATE_UNSYNC;
			}
			return;
		}
		if (++comm_receive_preamble_counter >= COMM_RECEIVE_PREAMBLE_COUNT) {
			comm_receive_preamble_counter = 0;
			comm_receive_crc              = 0;
			comm_receive_state            = COMM_RECEIVE_STATE_COMMAND_ID;
		}
		return;
	case COMM_RECEIVE_STATE_COMMAND_ID:
	{
		uint8_t cmd_id = b;
		if (cmd_id >= sizeof(comm_receive_commands)/sizeof(comm_receive_commands[0])) {
			if(!comm_receive_error_state) {
				comm_receive_error_state = COMM_RECEIVE_ERROR_STATE_COMMAND_ID;
			}
			comm_receive_state = COMM_RECEIVE_STATE_SYNC;
			return;
		}
		comm_receive_command = comm_receive_commands[cmd_id];
		
		if (comm_receive_command->type == COMMAND_TYPE_BROADCAST) {
			if (*(comm_receive_command->state) != COMMAND_STATE_UNLOCKED) {
				comm_receive_skip_before_count = comm_receive_command->receive_buffer_size;
				comm_receive_state = COMM_RECEIVE_STATE_CRC;
				return;
			}
			*(comm_receive_command->state) = COMMAND_STATE_WRITE_LOCKED;
			
			comm_receive_write_pos   = comm_receive_command->receive_buffer;
			comm_receive_write_count = comm_receive_command->receive_buffer_size;
			comm_receive_state = COMM_RECEIVE_STATE_CRC;
			return;
		}
		comm_receive_state = COMM_RECEIVE_STATE_ADDRESS;
		return;
	}
	case COMM_RECEIVE_STATE_ADDRESS:
		comm_receive_address = b;
		comm_receive_state   = COMM_RECEIVE_STATE_COUNT;
		return;
	case COMM_RECEIVE_STATE_COUNT:
	{
		uint8_t count = b;
		
		const command_type_info_t& cmd_type_info = cmd_type_infos[comm_receive_command->type - 1];
		uint8_t my_addr       = cmd_type_info.get_addr();
		uint8_t my_addr_count = cmd_type_info.addr_count;
		uint8_t block_size    = comm_receive_command->receive_buffer_size;
		
		if (*(comm_receive_command->state) != COMMAND_STATE_UNLOCKED) {
			comm_receive_skip_before_count = b * block_size;
			comm_receive_state = COMM_RECEIVE_STATE_CRC;
			return;
		}
		
		uint8_t skip_before_read;
		uint8_t read;
		uint8_t skip_after_read;
		
		if (comm_receive_address <= my_addr) {
			skip_before_read = my_addr - comm_receive_address;
			if (skip_before_read > count) {
				skip_before_read = count;
			} else {
				read = count - skip_before_read;
				if (read > my_addr_count) {
					skip_after_read = read - my_addr_count;
					comm_receive_skip_after_count = skip_after_read * block_size;
					read = my_addr_count;
				}
				comm_receive_write_count = read * block_size;
				comm_receive_write_pos   = comm_receive_command->receive_buffer;
			}
			comm_receive_skip_before_count = skip_before_read * block_size;
		} else {
			uint8_t my_addr_end = my_addr + my_addr_count;
			read = (comm_receive_address < my_addr_end)
					? my_addr_end - comm_receive_address
					: 0;
			comm_receive_write_count = read * block_size;
			comm_receive_write_pos   = comm_receive_command->receive_buffer + (comm_receive_address - my_addr) * block_size;
			
			skip_after_read = count - read;
			comm_receive_skip_after_count = skip_after_read * block_size;
		}
		if(comm_receive_write_count) {
			*(comm_receive_command->state) = COMMAND_STATE_WRITE_LOCKED;
		}
		comm_receive_state = COMM_RECEIVE_STATE_CRC;
		return;
	}
	case COMM_RECEIVE_STATE_CRC:
		if (comm_receive_crc != 0) {
			*(comm_receive_command->state) = COMMAND_STATE_UNLOCKED;
			if(!comm_receive_error_state) {
				comm_receive_error_state = COMM_RECEIVE_ERROR_STATE_CRC;
			}
			comm_receive_state = COMM_RECEIVE_STATE_SYNC;
			return;
		}
		if(*(comm_receive_command->state) == COMMAND_STATE_WRITE_LOCKED) {
			*(comm_receive_command->state) = COMMAND_STATE_READ_LOCKED;
		}
		comm_receive_state = COMM_RECEIVE_STATE_SYNC;
		return;
		
	}	
}

void CommReceiveLoop() {
	if (command1_state == COMMAND_STATE_READ_LOCKED) {
		if(command1_on_recv_func) {
			command1_on_recv_func(command1_buffer);
		}
		command1_state = COMMAND_STATE_UNLOCKED;
	}
}
