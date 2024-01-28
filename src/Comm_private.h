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
	COMM_SEND_CRC
} comm_send_state_t;

typedef struct {
	comm_send_state_t state;
	uint8_t  preamble_count;
	uint8_t* pos;
	uint8_t  count;
	uint8_t  crc;
} comm_send_isr_t;

#endif
