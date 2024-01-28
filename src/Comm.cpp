#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef UNITTEST
#include <cstring>
#endif

#include "CommandLib.h"
#include "Macros.h"

#include "Comm.h"
#include "Comm_private.h"

static constexpr uint32_t comm_baudrate       = 115200;
static constexpr uint8_t  comm_preamble_byte  = 0x55;
static constexpr uint8_t  comm_preamble_count = 2;

PRIVATE volatile comm_error_t               comm_error = COMM_ERROR_NONE;
PRIVATE comm_recv_isr_t                     comm_recv_isr = {0};
PRIVATE comm_send_isr_t                     comm_send_isr;
PRIVATE volatile bool                       comm_send_txen          = false;	
PRIVATE comm_send_message_base_t * volatile comm_send_message_begin = nullptr;
PRIVATE comm_send_message_base_t * volatile comm_send_message_end   = nullptr;

static volatile union {
	struct __attribute__((packed)) {
		uint8_t device_nr;
		uint8_t strip_nr;
	};
	uint8_t by_type[0];
} comm_my_block_nrs = {{
	.device_nr = 0,
	.strip_nr  = 0
}};

PRIVATE INLINE void CommIsrRaiseError(comm_error_t error);
PRIVATE INLINE void CommFire(const command_info_t& command_info);
PRIVATE INLINE void CommIsrReceivePreamble(const uint8_t data_byte);
PRIVATE INLINE void CommIsrReceiveCommandId(const uint8_t data_byte);
PRIVATE INLINE void CommIsrReceiveCommandIdBroadCast();
PRIVATE INLINE void CommIsrReceiveBlockNr(const uint8_t data_byte);
PRIVATE INLINE void CommIsrReceiveBlockCount(const uint8_t data_byte);
PRIVATE INLINE void CommIsrReceiveCrc(const uint8_t data_byte);

PRIVATE INLINE uint8_t CommIsrGetMyBlockNr(command_type_t command_type);

#ifdef UNITTEST
void CommReset() {
	comm_error = COMM_ERROR_NONE;
	memset((void*)&comm_my_block_nrs, 0x00, sizeof(comm_my_block_nrs));
	memset(&comm_recv_isr, 0x00, sizeof(comm_recv_isr));
}
#endif

void CommBegin() {
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
	for (uint8_t i = 0; i < ARRAY_SIZE(command_infos); i++) {
		const command_info_t& command_info(command_infos[i]);
		command_base_t& command(command_info.command);
		if (command.state == COMMAND_STATE_READ_LOCKED) {
			CommFire(command_info);
		}
	}
}

comm_error_t CommGetError() {
	const comm_error_t error = comm_error;
	comm_error = COMM_ERROR_NONE;
	return error;
}

ISR(USART_RX_vect) {
	uint8_t data_byte = UDR0;
	if((UCSR0A & ((1 << FE0) | (1 << DOR0) | (1 << UPE0))) != 0) {
		CommIsrRaiseError(COMM_ERROR_SIGNAL);
		comm_recv_isr.state = COMM_STATE_PREAMBLE;
	} else {
		comm_recv_isr.crc += data_byte;
		
		if (comm_recv_isr.skip_byte_count) {
			--comm_recv_isr.skip_byte_count;
			return;
		}
		if (comm_recv_isr.read_byte_count) {
			--comm_recv_isr.read_byte_count;
			*(comm_recv_isr.read_byte_pos++) = data_byte;
			return;
		}
		if (comm_recv_isr.skip_byte_after_read_count) {
			comm_recv_isr.skip_byte_count = comm_recv_isr.skip_byte_after_read_count - 1;
			comm_recv_isr.skip_byte_after_read_count = 0;
			return;
		}
		
		switch (comm_recv_isr.state) {
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
		case COMM_STATE_CRC:
			CommIsrReceiveCrc(data_byte);
			return;
		default:
			comm_recv_isr.state = COMM_STATE_PREAMBLE;
		}
	}
}

PRIVATE INLINE void CommIsrReceivePreamble(const uint8_t data_byte) {
	if (data_byte != comm_preamble_byte) {
		comm_recv_isr.preamble_count = 0;
		CommIsrRaiseError(COMM_ERROR_DATA);
		return;
	}
	if (++comm_recv_isr.preamble_count >= comm_preamble_count) {
		comm_recv_isr.preamble_count = 0;
		comm_recv_isr.crc            = 0;
		comm_recv_isr.state          = COMM_STATE_COMMAND_ID;
	}
}

PRIVATE INLINE void CommIsrReceiveCommandId(const uint8_t data_byte) {
	const uint8_t command_id = data_byte;
	
	if (command_id >= ARRAY_SIZE(command_infos)) {
		CommIsrRaiseError(COMM_ERROR_DATA);
		comm_recv_isr.state = COMM_STATE_PREAMBLE;
		return;	
	}
	comm_recv_isr.command_info = &command_infos[command_id];
	
	if (comm_recv_isr.command_info->type == COMMAND_TYPE_BROADCAST) {
		CommIsrReceiveCommandIdBroadCast();
		return;
	}
	comm_recv_isr.state = COMM_STATE_BLOCK_NR;
}

PRIVATE INLINE void CommIsrReceiveCommandIdBroadCast() {
	command_base_t& command(comm_recv_isr.command_info->command);
	
	if (command.state == COMMAND_STATE_READ_LOCKED) {
		comm_recv_isr.skip_byte_count = comm_recv_isr.command_info->block_size;
		comm_recv_isr.read_byte_count = 0;
		comm_recv_isr.skip_byte_after_read_count = 0;
		comm_recv_isr.state           = COMM_STATE_CRC;
		CommIsrRaiseError(COMM_ERROR_BUSY);
		return;
	}
	command.state = COMMAND_STATE_WRITE_LOCKED;
	
	comm_recv_isr.read_byte_pos   = command.buffer;
	comm_recv_isr.read_byte_count = comm_recv_isr.command_info->block_size;
	comm_recv_isr.state           = COMM_STATE_CRC;
}

PRIVATE INLINE void CommIsrReceiveBlockNr(const uint8_t data_byte) {
	comm_recv_isr.block_nr  = data_byte;
	comm_recv_isr.state     = COMM_STATE_BLOCK_COUNT;
}

PRIVATE INLINE void CommIsrReceiveBlockCount(const uint8_t data_byte) {
	const uint8_t block_count = data_byte;
	
	command_base_t& command(comm_recv_isr.command_info->command);
	if (command.state == COMMAND_STATE_READ_LOCKED) {
		comm_recv_isr.skip_byte_count = block_count * comm_recv_isr.command_info->block_size;
		comm_recv_isr.read_byte_count = 0;
		comm_recv_isr.skip_byte_after_read_count = 0;
		comm_recv_isr.state = COMM_STATE_CRC;
		
		CommIsrRaiseError(COMM_ERROR_BUSY);
		return;
	}
	
	const command_type_t command_type = comm_recv_isr.command_info->type;
	uint8_t my_block_nr    = CommIsrGetMyBlockNr(command_type);
	uint8_t my_block_count = CommandTypeGetBlockCount(command_type);
	
	uint8_t skip_before_read_block_count;
	uint8_t read_block_count;
	uint8_t skip_after_read_block_count;
	
	uint8_t block_size = comm_recv_isr.command_info->block_size;
	
	if (comm_recv_isr.block_nr <= my_block_nr) {
		skip_before_read_block_count = my_block_nr - comm_recv_isr.block_nr;
		if (skip_before_read_block_count >= block_count) {
			skip_before_read_block_count = block_count;
			comm_recv_isr.read_byte_count = 0;
			comm_recv_isr.skip_byte_after_read_count = 0;
		} else {
			read_block_count = block_count - skip_before_read_block_count;
			if (read_block_count > my_block_count) {
				skip_after_read_block_count = read_block_count - my_block_count;
				comm_recv_isr.skip_byte_after_read_count = skip_after_read_block_count * block_size;
				read_block_count = my_block_count;
			} else {
				comm_recv_isr.skip_byte_after_read_count = 0;
			}
			comm_recv_isr.read_byte_count = read_block_count * block_size;
			comm_recv_isr.read_byte_pos   = comm_recv_isr.command_info->command.buffer;
			command.processed_block_bits = (1 << read_block_count) - 1;
		}
		comm_recv_isr.skip_byte_count = skip_before_read_block_count * block_size;
	} else {
		uint8_t my_end_block_nr = my_block_nr + my_block_count;
		read_block_count = (comm_recv_isr.block_nr < my_end_block_nr)
				? my_end_block_nr - comm_recv_isr.block_nr
				: 0;
		if (read_block_count >= block_count) {
			read_block_count = block_count;
			comm_recv_isr.skip_byte_after_read_count = 0;
			comm_recv_isr.skip_byte_count = 0;
		} else {
			skip_after_read_block_count = block_count - read_block_count;
			comm_recv_isr.skip_byte_after_read_count = skip_after_read_block_count * block_size;
			comm_recv_isr.skip_byte_count = 0;
		}
		uint8_t read_block_offset = (comm_recv_isr.block_nr - my_block_nr);
		command.processed_block_bits = ((1 << read_block_count) - 1) << read_block_offset;
		
		comm_recv_isr.read_byte_count = read_block_count * block_size;
		comm_recv_isr.read_byte_pos   = comm_recv_isr.command_info->command.buffer
		                         + read_block_offset * block_size;
		
	}
	if (comm_recv_isr.read_byte_count) {
		command.state = COMMAND_STATE_WRITE_LOCKED;
	}
	comm_recv_isr.state = COMM_STATE_CRC;
}

PRIVATE INLINE void CommIsrReceiveCrc(const uint8_t data_byte) {
	const command_info_t& command_info(*comm_recv_isr.command_info);
	command_base_t&       command(command_info.command);
	
	if (comm_recv_isr.crc != 0) {
		command.state = COMMAND_STATE_UNLOCKED;
		CommIsrRaiseError(COMM_ERROR_DATA);
		comm_recv_isr.state = COMM_STATE_PREAMBLE;
		return;
	}
	if (command.state == COMMAND_STATE_WRITE_LOCKED) {
		command.state = COMMAND_STATE_READ_LOCKED;
		
		if (command_info.fire == COMMAND_FIRE_ALLOWED_FROM_ISR) {
			CommFire(command_info);
		}
	}
	comm_recv_isr.state = COMM_STATE_PREAMBLE;
}

PRIVATE INLINE void CommFire(const command_info_t& command_info) {
	command_base_t& command(command_info.command);
	if (CommandTypeGetBlockCount(command_info.type) == 1) {
		if (command_info.on_received_function.single_block(*command.buffer)) {
			command.state = COMMAND_STATE_UNLOCKED;
		}
	} else {
		uint8_t relative_block_nr = 0;
		uint8_t processed_block_bits = command.processed_block_bits;
		uint8_t processed_block_bit = 0b1;
		while(processed_block_bits) {
			if (processed_block_bits & processed_block_bit && 
					command_info.on_received_function.multi_block(
						relative_block_nr,
						command.buffer[relative_block_nr * command_info.block_size]))
			{
				command.processed_block_bits &= ~processed_block_bit;
			}
			processed_block_bits &= ~processed_block_bit;
			relative_block_nr++;
			processed_block_bit <<= 1;
		}
		if (!command.processed_block_bits) {
			command.state = COMMAND_STATE_UNLOCKED;
		}
	}
}

PRIVATE INLINE void CommIsrRaiseError(comm_error_t error) {
	if (comm_error == COMM_ERROR_NONE) {
		comm_error = error;
	}
}

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

PRIVATE INLINE uint8_t CommIsrGetMyBlockNr(command_type_t command_type) {
	return comm_my_block_nrs.by_type[command_type - 1];
}

PRIVATE INLINE void CommTxEnable() {
	// TX_EN = true
	comm_send_txen = true;
}

PRIVATE INLINE void CommTxDisable() {
	comm_send_txen = false;
}

ISR(USART_TX_vect) 
{
	if (!comm_send_txen) {
		//TX_EN = false;
	}
}

void CommSend(
	uint8_t command_id, 
	comm_send_message_base_t& send_message,	
	uint8_t size
) {
	send_message.next       = nullptr;
	send_message.size       = size;
	send_message.command_id = command_id;
	
	UCSR0B &= ~(1<<UDRIE0);
	if(comm_send_message_end) {
		comm_send_message_end->next = &send_message;
	}
	comm_send_message_end = &send_message;
	CommTxEnable();
	
	UCSR0B |= (1<<UDRIE0);
}

ISR(USART_UDRE_vect)
{
	if(comm_send_isr.count) {
		--comm_send_isr.count;
		uint8_t b = *(comm_send_isr.pos++);
		comm_send_isr.crc -= b;
		UDR0 = b;
		return;
	}
	
	switch(comm_send_isr.state) {
	case COMM_SEND_PREAMBLE:
		if(!comm_send_message_begin) {
			CommTxDisable();
			UCSR0B &= ~(1<<UDRIE0);
			return;
		}
		CommTxEnable();
		UDR0 = comm_preamble_byte;
		if(++comm_send_isr.preamble_count >= comm_preamble_count) {
			comm_send_isr.pos   = &comm_send_message_begin->value[0];
			comm_send_isr.count = comm_send_message_begin->size;
			comm_send_isr.crc   = 0;
			comm_send_isr.state = COMM_SEND_CRC;
		}
		return;
	case COMM_SEND_CRC:
		UDR0 = comm_send_isr.crc;
		
		comm_send_message_base_t* next = comm_send_message_begin->next;
		comm_send_message_begin->next  = SEND_MESSAGE_NEXT_UNUSED;
		comm_send_message_begin        = next;
		comm_send_isr.state = COMM_SEND_PREAMBLE;
		return;
	}
}
