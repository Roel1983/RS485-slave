#ifndef COMM_PRIVATE_H_
#define COMM_PRIVATE_H_

#include <stdint.h>

#include "Command.h"

typedef enum {
	COMM_STATE_PREAMBLE,
	COMM_STATE_COMMAND_ID,
	COMM_STATE_BLOCK_NR,
	COMM_STATE_BLOCK_COUNT,
	COMM_STATE_CRC
} comm_state_t;

typedef struct {
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
} comm_recv_isr_t;

typedef enum {
	COMM_SEND_PREAMBLE,
	COMM_SEND_COMMAND_ID,
	COMM_SEND_CRC
} comm_send_state_t;

typedef struct {
	comm_send_state_t state;
	uint8_t  preamble_count;
	uint8_t* pos;
	uint8_t  count;
	uint8_t  crc;
} comm_send_isr_t;

typedef enum {
	COMM_SEND_STRATEGY_SEND_ON_DEMAND,
	COMM_SEND_STRATEGY_SEND_AT_WILL
} comm_send_strategy_t;

static constexpr uint32_t comm_baudrate       = 115200;
static constexpr uint8_t  comm_preamble_byte  = 0x55;
static constexpr uint8_t  comm_preamble_count = 2;
static constexpr active_high_pin_t comm_tx_en_pin{PIN17};

#endif
