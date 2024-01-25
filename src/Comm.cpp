#include "Comm.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#define ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))

typedef enum {
	COMM_STATE_PREAMBLE,
	COMM_STATE_COMMAND_ID,
	COMM_STATE_BLOCK_NR,
	COMM_STATE_BLOCK_COUNT,
	COMM_STATE_CRC
} comm_state_t;

constexpr uint32_t comm_baudrate       = 115200;
constexpr uint8_t  comm_preamble_byte  = 0x55;
constexpr uint8_t  comm_preamble_count = 2;

static volatile comm_error_t comm_error = COMM_ERROR_NONE;

// Start command.h

typedef enum {
	COMMAND_TYPE_BROADCAST,
	COMMAND_TYPE_STRIP,
} command_type_t;
static constexpr uint8_t command_type_block_counts[] = {1, 4};

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

struct command_info_t {
	command_type_t  type;
	command_base_t& command;
	uint8_t         block_size;

	template <
		command_type_t _type,
		typename       T
	> constexpr command_info_t(command_t<_type, T>& _command)
	: type(_type)
	, command(_command)
	, block_size(sizeof(T)) {
	}
};

command_t<COMMAND_TYPE_BROADCAST, uint16_t> command1;

constexpr command_info_t command_infos[] = {
	command_info_t(command1)
};

// End command.h



static struct {
	uint8_t               crc;
	comm_state_t          state;
	uint8_t               read_byte_count;
	union {
		uint8_t*          read_byte_pos;
		uint8_t           preamble_count;
		uint8_t           block_nr;
	};
	uint16_t              skip_byte_count;
	uint16_t              skip_byte_after_read_count;
	const command_info_t* command_info;
} comm_isr;

static void CommSetupUsart();
static inline void CommIsrRaiseError(comm_error_t error);
static inline void CommIsrReceivePreamble(const uint8_t data_byte);
static inline void CommIsrReceiveCommandId(const uint8_t data_byte);
static inline void CommIsrReceiveCommandIdBroadCast();
static inline void CommIsrReceiveBlockNr(const uint8_t data_byte);
static inline void CommIsrReceiveBlockCount(const uint8_t data_byte);

static inline uint8_t CommIsrGetMyBlockNr(command_type_t command_type);

void CommBegin() {
	CommSetupUsart();
}

static void CommSetupUsart() {
	constexpr uint16_t baudrate_prescaler = (F_CPU / (8UL * comm_baudrate)) - 1;
	
	UBRR0H  = baudrate_prescaler >> 8;
	UBRR0L  = baudrate_prescaler;
	
	UCSR0A |= (1<<U2X0);
	
	UCSR0C = (0<<UMSEL00)
	       | (0<<UPM00)
	       | (0<<USBS0)
	       | (3<<UCSZ00);
	
	UCSR0B = (1<<RXEN0)
	       | (1<<TXEN0)
	       | (1<<TXCIE0)
	       | (1<<RXCIE0);

}

void CommLoop() {
}

comm_error_t CommGetLastError() {
	const comm_error_t error = comm_error;
	comm_error = COMM_ERROR_NONE;
	return error;
}

ISR(USART_RX_vect) {
	uint8_t data_byte = UDR0;
	if((UCSR0A & ((1 << FE0) | (1 << DOR0) | (1 << UPE0))) != 0) {
		CommIsrRaiseError(COMM_ERROR_SIGNAL);
		comm_isr.state = COMM_STATE_PREAMBLE;
	} else {
		comm_isr.crc += data_byte;
		
		if (comm_isr.skip_byte_count--) {
			return;
		}
		if (comm_isr.read_byte_count--) {
			*(comm_isr.read_byte_pos++) = data_byte;
			return;
		}
		if (comm_isr.skip_byte_after_read_count--) {
			comm_isr.skip_byte_count = comm_isr.skip_byte_after_read_count;
			comm_isr.skip_byte_after_read_count = 0;
			return;
		}
		
		switch (comm_isr.state) {
		case COMM_STATE_PREAMBLE:
			CommIsrReceivePreamble(data_byte);
			return;
		case COMM_STATE_COMMAND_ID:
			CommIsrReceiveCommandId(data_byte);
			return;
		case COMM_STATE_BLOCK_NR:
			CommIsrReceiveBlockNr(data_byte);
			return;
		case COMM_STATE_BLOCK_COUNT:
			CommIsrReceiveBlockCount(data_byte);
			return;
		}
	}
}

static inline void CommIsrReceivePreamble(const uint8_t data_byte) {
	if (data_byte != comm_preamble_byte) {
		comm_isr.preamble_count = 0;
		CommIsrRaiseError(COMM_ERROR_DATA);
		return;
	}
	if (++comm_isr.preamble_count >= comm_preamble_count) {
		comm_isr.preamble_count = 0;
		comm_isr.crc            = 0;
		comm_isr.state          = COMM_STATE_COMMAND_ID;
	}
}

static inline void CommIsrReceiveCommandId(const uint8_t data_byte) {
	const uint8_t command_id = data_byte;
	
	if (command_id >= ARRAY_SIZE(command_infos)) {
		CommIsrRaiseError(COMM_ERROR_DATA);
		comm_isr.state = COMM_STATE_PREAMBLE;
		return;	
	}
	comm_isr.command_info = &command_infos[command_id];
	
	if (comm_isr.command_info->type == COMMAND_TYPE_BROADCAST) {
		CommIsrReceiveCommandIdBroadCast();
		return;
	}
	comm_isr.state = COMM_STATE_BLOCK_NR;
}

static inline void CommIsrReceiveCommandIdBroadCast() {
	command_base_t& command(comm_isr.command_info->command);
	
	if (command.state == COMMAND_STATE_UNLOCKED) {
		comm_isr.skip_byte_count = comm_isr.command_info->block_size;
		comm_isr.state = COMM_STATE_PREAMBLE;
		return;
	}
	command.state = COMMAND_STATE_WRITE_LOCKED;
	
	comm_isr.read_byte_pos   = command.buffer;
	comm_isr.read_byte_count = comm_isr.command_info->block_size;
	comm_isr.state           = COMM_STATE_CRC;
}

static inline void CommIsrReceiveBlockNr(const uint8_t data_byte) {
	comm_isr.block_nr  = data_byte;
	comm_isr.state     = COMM_STATE_BLOCK_COUNT;
}

static inline void CommIsrReceiveBlockCount(const uint8_t data_byte) {
	const uint8_t block_count = data_byte;
	
	command_base_t& command(comm_isr.command_info->command);
	if (command.state != COMMAND_STATE_UNLOCKED) {
		comm_isr.skip_byte_count = block_count * comm_isr.command_info->block_size;
		comm_isr.state = COMM_STATE_PREAMBLE;
		
		CommIsrRaiseError(COMM_ERROR_BUSY);
		return;
	}
	
	const command_type_t command_type = comm_isr.command_info->type;
	uint8_t my_block_nr    = CommIsrGetMyBlockNr(command_type);
	uint8_t my_block_count = CommandTypeGetBlockCount(command_type);
	
	uint8_t skip_before_read_block_count;
	uint8_t read_block_count;
	uint8_t skip_after_read_block_count;
	
	uint8_t block_size = comm_isr.command_info->block_size;
	
	if (comm_isr.block_nr <= my_block_nr) {
		skip_before_read_block_count = my_block_nr - comm_isr.block_nr;
		if (skip_before_read_block_count > my_block_count) {
			skip_before_read_block_count = my_block_count;
		} else {
			read_block_count = my_block_count - skip_before_read_block_count;
			if (read_block_count > my_block_count) {
				skip_after_read_block_count = read_block_count - my_block_count;
				comm_isr.skip_byte_after_read_count = skip_after_read_block_count * block_size;
				read_block_count = my_block_count;
			}
			comm_isr.read_byte_count = read_block_count * block_size;
			comm_isr.read_byte_pos   = comm_isr.command_info->command.buffer;
		}
		comm_isr.skip_byte_count = skip_before_read_block_count * block_size;
	}
	
}

static inline void CommIsrRaiseError(comm_error_t error) {
	if (comm_error == COMM_ERROR_NONE) {
		comm_error = COMM_ERROR_SIGNAL;
	}
}

volatile union {
	struct {
		uint8_t device_nr;
		uint8_t strip_nr;
	};
	uint8_t by_type[];
} comm_my_block_nrs = {{
	.device_nr = 0,
	.strip_nr  = 0
}};

void CommSetDeviceNr(uint8_t device_nr) {
	cli();
	comm_my_block_nrs.device_nr = device_nr;
	sei();
}

void CommSetStripNr(uint8_t strip_nr) {
	cli();
	comm_my_block_nrs.strip_nr = strip_nr;
	sei();
}

static inline uint8_t CommIsrGetMyBlockNr(command_type_t command_type) {
	return comm_my_block_nrs.by_type[command_type - 1];
}
