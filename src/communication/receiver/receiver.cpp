#include <avr/interrupt.h>
#include <avr/io.h>

#include <assert.h>
#ifdef UNITTEST
#include <cstring>
#endif

#include "../../macros.hpp"
#include "../commands.hpp"
#include "command.hpp"
#include "receiver_errors.hpp"

namespace communitation {
namespace receiver {

typedef enum {
	STATE_PREAMBLE,
	STATE_SENDER_UNIQUE_ID,
	STATE_PAYLOAD_LENGTH,
	STATE_COMMAND_ID,
	STATE_BLOCK_NR,
	STATE_CRC,
} State;

struct Isr {
	uint8_t           crc;
	State             state;
	uint8_t           read_byte_count;
	union {
		uint8_t      *read_byte_pos;
		uint8_t       preamble_count;
		struct {
			uint16_t  remaining_payload_length;
			uint8_t   block_nr;
		};
	};
	uint16_t          skip_byte_count;
	uint16_t          skip_byte_count_after_read;
	uint8_t           sender_unique_id;
	const CommandInfo* command_info;
};

PRIVATE constexpr uint32_t BAUDRATE       = 115200;
PRIVATE constexpr uint8_t  PREAMBLE_BYTE  = 0x55;
PRIVATE constexpr uint8_t  PREAMBLE_COUNT = 2;
PRIVATE constexpr uint8_t  EXTENDED_PAYLOAD_LENGHT_MASK = 0x80;

Isr    isr;

PRIVATE INLINE void processIncommingByte     (const uint8_t data_byte);
PRIVATE INLINE bool receiveBlockData         (const uint8_t data_byte);
PRIVATE INLINE void receivePreamble          (const uint8_t data_byte);
PRIVATE INLINE void receiveSenderUniqueId    (const uint8_t data_byte);
PRIVATE INLINE void receivePayloadLength     (const uint8_t data_byte);
PRIVATE INLINE void receiveCommandId         (const uint8_t data_byte);
PRIVATE INLINE void receiveCommandIdUnknownCommand();
PRIVATE INLINE void receiveCommandIdBroadcast();
PRIVATE INLINE void receiveCommandIdAddressable();
PRIVATE INLINE void receiveBlockNr           (const uint8_t data_byte);
PRIVATE INLINE void calculateReceiveBlockData(CommandBase& command, const uint8_t block_count);
PRIVATE INLINE void receiveCrc               (const uint8_t data_byte);

PRIVATE INLINE void notifyCommandReceived(const CommandInfo& command_info);
PRIVATE INLINE void notifySingleBlockCommandReceived(const CommandInfo& command_info);
PRIVATE INLINE void notifyMultiBlockCommandReceived(const CommandInfo& command_info);

PRIVATE void receiveSkipRemainingPayload();

#ifdef UNITTEST
void ReceiverReset() {
	memset(&isr, 0, sizeof(isr));
	ReceiverErrorsReset();
}
#endif

void ReceiverSetup() {
	constexpr uint16_t baudrate_prescaler = (F_CPU / (8UL * BAUDRATE)) - 1;
	
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

ISR(USART_RX_vect) {
	const bool has_signal_error = (UCSR0A & ((1 << FE0) | (1 << DOR0) | (1 << UPE0))) != 0;
	if(has_signal_error) {
		raiseError(ERROR_SIGNAL);
		isr.preamble_count = 0;
		isr.state          = STATE_PREAMBLE;
	} else {
		const uint8_t data_byte = UDR0;
		processIncommingByte(data_byte);
	}
}

PRIVATE INLINE void processIncommingByte(const uint8_t data_byte) {
	isr.crc += data_byte;
	
	if (receiveBlockData(data_byte)) {
		return;
	}
	
	switch (isr.state) {
	case STATE_PREAMBLE:
		receivePreamble(data_byte);
		return;
	case STATE_SENDER_UNIQUE_ID:
		receiveSenderUniqueId(data_byte);
		return;
	case STATE_PAYLOAD_LENGTH:
		receivePayloadLength(data_byte);
		return;
	case STATE_COMMAND_ID:
		receiveCommandId(data_byte);
		return;
	case STATE_BLOCK_NR:
		receiveBlockNr(data_byte);
		return;
	case STATE_CRC:
		receiveCrc(data_byte);
		return;
	}
}

PRIVATE INLINE bool receiveBlockData(const uint8_t data_byte) {
	if (isr.skip_byte_count) {
		--isr.skip_byte_count;
		return true;
	}
	if (isr.read_byte_count) {
		--isr.read_byte_count;
		*(isr.read_byte_pos++) = data_byte;
		return true;
	}
	if (isr.skip_byte_count_after_read) {
		isr.skip_byte_count = isr.skip_byte_count_after_read - 1;
		isr.skip_byte_count_after_read = 0;
		return true;
	}
	return false;
}

PRIVATE INLINE void receivePreamble(const uint8_t data_byte) {
	if (data_byte != PREAMBLE_BYTE) {
		raiseError(ERROR_PREAMBLE);
		isr.preamble_count = 0;
		isr.state          = STATE_PREAMBLE;
		return;
	}
	if (++isr.preamble_count >= PREAMBLE_COUNT) {
		isr.crc = 0;
		isr.state = STATE_SENDER_UNIQUE_ID;
	}
}

PRIVATE INLINE void receiveSenderUniqueId(const uint8_t data_byte) {
	isr.sender_unique_id = data_byte;
	isr.state = STATE_PAYLOAD_LENGTH;
}

PRIVATE INLINE void receivePayloadLength(const uint8_t data_byte) {
	if (data_byte & EXTENDED_PAYLOAD_LENGHT_MASK) {
		isr.remaining_payload_length = (data_byte & ~EXTENDED_PAYLOAD_LENGHT_MASK) << 8;
		return;
	} else {
		isr.remaining_payload_length |= data_byte;
	}
	isr.state = STATE_COMMAND_ID;
}

PRIVATE INLINE void receiveCommandId(const uint8_t data_byte) {
	const uint8_t command_id = data_byte;
	
	isr.command_info = CommandGetInfoGet(command_id);
	if (!isr.command_info) {
		receiveCommandIdUnknownCommand();
		return;
	}
	
	if (isr.command_info->type == COMMAND_TYPE_BROADCAST) {
		receiveCommandIdBroadcast();
	} else {
		receiveCommandIdAddressable();
	}
}

PRIVATE INLINE void receiveCommandIdUnknownCommand() {
	receiveSkipRemainingPayload();
	raiseError(ERROR_UNKNOW_COMMAND);
	isr.preamble_count = 0;
	isr.state          = STATE_PREAMBLE;
}

PRIVATE INLINE void receiveCommandIdBroadcast() {
	CommandBase& command(isr.command_info->command);
	
	if (isr.command_info->block_size != isr.remaining_payload_length) {
		receiveSkipRemainingPayload();
		raiseError(ERROR_INVALID_LENGTH);
		isr.preamble_count = 0;
		isr.state          = STATE_PREAMBLE;
		return;
	}
	if (command.lock != COMMAND_LOCK_NONE) {
		receiveSkipRemainingPayload();
		raiseError(ERROR_BUSY);
		isr.preamble_count = 0;
		isr.state          = STATE_PREAMBLE;
		return;
	}
	command.lock = COMMAND_LOCK_WRITE;
	
	isr.read_byte_pos   = command.buffer;
	isr.read_byte_count = isr.remaining_payload_length;
	isr.state           = STATE_CRC;
}

PRIVATE INLINE void receiveCommandIdAddressable() {
	isr.state = STATE_BLOCK_NR;
}

PRIVATE INLINE void receiveBlockNr(const uint8_t data_byte) {
	CommandBase& command(isr.command_info->command);
	
	isr.block_nr = data_byte;
	isr.remaining_payload_length -= 1; 
	
	const uint8_t block_count = isr.remaining_payload_length / isr.command_info->block_size;
	
	if (isr.remaining_payload_length == block_count * isr.command_info->block_size) {
		receiveSkipRemainingPayload();
		raiseError(ERROR_INVALID_LENGTH);
		isr.preamble_count = 0;
		isr.state          = STATE_PREAMBLE;
		return;
	}
	if (command.lock != COMMAND_LOCK_NONE) {
		receiveSkipRemainingPayload();
		raiseError(ERROR_BUSY);
		isr.preamble_count = 0;
		isr.state          = STATE_PREAMBLE;
		return;
	}
	
	calculateReceiveBlockData(command, block_count);
	
	if (isr.read_byte_count) {
		command.lock = COMMAND_LOCK_WRITE;
	}
	isr.state = STATE_CRC;
}

PRIVATE INLINE void calculateReceiveBlockData(CommandBase& command, const uint8_t block_count) {
	const CommandInfo& command_info(*isr.command_info);
	const CommandType command_type = command_info.type;
	
	const uint8_t my_block_nr    = CommandTypeGetBlockNr   (command_type);
	const uint8_t my_block_count = CommandTypeGetBlockCount(command_type);
	const uint8_t block_size     = command_info.block_size;
	
	uint8_t skip_block_count_before_read;
	uint8_t read_block_count;
	uint8_t skip_block_count_after_read;
	
	if (isr.block_nr <= my_block_nr) {
		skip_block_count_before_read = my_block_nr - isr.block_nr;
		if (skip_block_count_before_read >= block_count) {
			skip_block_count_before_read = block_count;
			isr.read_byte_count            = 0;
			isr.skip_byte_count_after_read = 0;
		} else {
			read_block_count = block_count - skip_block_count_before_read;
			if (read_block_count > my_block_count) {
				skip_block_count_after_read = read_block_count - my_block_count;
				isr.skip_byte_count_after_read = skip_block_count_after_read * block_size;
				read_block_count = my_block_count;
			} else {
				isr.skip_byte_count_after_read = 0;
			}
			isr.read_byte_count = read_block_count * block_size;
			isr.read_byte_pos   = command_info.command.buffer;
			command.processed_block_bits = (1 << read_block_count) - 1;
		}
		isr.skip_byte_count = skip_block_count_before_read * block_size;
	} else {
		const uint8_t my_end_block_nr = my_block_nr + my_block_count;
		read_block_count = (isr.block_nr < my_end_block_nr) ? my_end_block_nr - isr.block_nr : 0;
		
		if (read_block_count >= block_count) {
			read_block_count = block_count;
			isr.skip_byte_count_after_read = 0;
			isr.skip_byte_count = 0;
		} else {
			skip_block_count_after_read = block_count - read_block_count;
			isr.skip_byte_count_after_read = skip_block_count_after_read * block_size;
			isr.skip_byte_count = 0;
		}
		uint8_t read_block_offset = (isr.block_nr - my_block_nr);
		command.processed_block_bits = ((1 << read_block_count) - 1) << read_block_offset;
		
		isr.read_byte_count = read_block_count * block_size;
		isr.read_byte_pos   = isr.command_info->command.buffer + read_block_offset * block_size;
	}
}

PRIVATE INLINE void receiveCrc(const uint8_t data_byte) {
	const CommandInfo& command_info(*isr.command_info);
	CommandBase&       command(command_info.command);
	
	if (isr.crc != 0) {
		if (command.lock == COMMAND_LOCK_WRITE) {
			command.lock = COMMAND_LOCK_NONE;
		}
		raiseError(ERROR_CRC);
		isr.preamble_count = 0;
		isr.state          = STATE_PREAMBLE;
		return;
	}
	
	if (command.lock == COMMAND_LOCK_WRITE) {
		command.lock = COMMAND_LOCK_READ;
		
		if (command_info.is_allow_notify_from_isr) {
			notifyCommandReceived(command_info);
		}
	}
	isr.preamble_count = 0;
	isr.state          = STATE_PREAMBLE;
}

PRIVATE INLINE void notifyCommandReceived(const CommandInfo& command_info) {
	if (CommandTypeGetBlockCount(command_info.type) == 1) {
		notifySingleBlockCommandReceived(command_info);
	} else {
		notifyMultiBlockCommandReceived(command_info);
	}
}

PRIVATE INLINE void notifySingleBlockCommandReceived(const CommandInfo& command_info) {
	CommandBase& command(command_info.command);
	if (command_info.onReceived(0, command.buffer)) {
		command.lock = COMMAND_LOCK_NONE;
	}
}

PRIVATE INLINE void notifyMultiBlockCommandReceived(const CommandInfo& command_info) {
	CommandBase& command(command_info.command);
	
	uint8_t relative_block_nr = 0;
	uint8_t processed_block_bits = command.processed_block_bits;
	uint8_t processed_block_bit = 0b1;
	while(processed_block_bits) {
		if (processed_block_bits & processed_block_bit && 
				command_info.onReceived(
					relative_block_nr,
					&command.buffer[relative_block_nr * command_info.block_size]))
		{
			command.processed_block_bits &= ~processed_block_bit;
		}
		processed_block_bits &= ~processed_block_bit;
		relative_block_nr++;
		processed_block_bit <<= 1;
	}
	if (!command.processed_block_bits) {
		command.lock = COMMAND_LOCK_NONE;
	}
}

PRIVATE void receiveSkipRemainingPayload() {
	isr.skip_byte_count            = isr.remaining_payload_length;
	isr.read_byte_count            = 0;
	isr.skip_byte_count_after_read = 0;
	isr.state                      = STATE_CRC;
}

}} // End of namespace ::communitation::receiver
