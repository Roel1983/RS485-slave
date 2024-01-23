#include <gtest/gtest.h>

#include "../Comm.h"

/*void CommReceiveReset() {
	comm_receive_crc               = 0;
	comm_receive_skip_before_count = 0;
	comm_receive_write_count       = 0;
	comm_receive_write_pos         = 0;
	comm_receive_skip_after_count  = 0;
}*/

typedef enum {
	COMMAND_STATE_UNLOCKED,
	COMMAND_STATE_WRITE_LOCKED,
	COMMAND_STATE_READ_LOCKED,
} command_state_t;
extern uint8_t command1_state;
extern uint8_t command1_buffer[4];

extern uint8_t command2_state;
extern uint8_t command2_buffer[4];
extern uint8_t   comm_receive_crc;

TEST(CommReceive_CommReceiveByte, test) {
	command1_state = COMMAND_STATE_UNLOCKED;
	
	CommReceiveByte('f');           CommReceiveLoop();
	CommReceiveByte('o');           CommReceiveLoop();
	CommReceiveByte('o');           CommReceiveLoop();
	CommReceiveByte((uint8_t)0xcc); CommReceiveLoop();
	CommReceiveByte('o');           CommReceiveLoop();
	
	CommReceiveByte((uint8_t)0xcc); CommReceiveLoop();
	CommReceiveByte((uint8_t)0xcc); CommReceiveLoop();
	
	EXPECT_EQ(command1_state, COMMAND_STATE_UNLOCKED);
	CommReceiveByte(COMMMAND_TEST); CommReceiveLoop();
	EXPECT_EQ(command1_state, COMMAND_STATE_WRITE_LOCKED);
	
	CommReceiveByte('R'); CommReceiveLoop();
	CommReceiveByte('o'); CommReceiveLoop();
	CommReceiveByte('e'); CommReceiveLoop();
	CommReceiveByte('l'); CommReceiveLoop();
	CommReceiveByte(110); 
	
	
	EXPECT_EQ(command1_state, COMMAND_STATE_READ_LOCKED);
	EXPECT_EQ(command1_buffer[0], 'R');
	EXPECT_EQ(command1_buffer[1], 'o');
	EXPECT_EQ(command1_buffer[2], 'e');
	EXPECT_EQ(command1_buffer[3], 'l');
	
	CommReceiveLoop();
}

TEST(CommReceive_CommReceiveByte, test2) {
	command2_state = COMMAND_STATE_UNLOCKED;
	
	uint8_t command[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		1, // Command 1 (type DEVICE)
		3, // Device address
		4, // Device count
		'A', 'b', 'c', 'd',
		'e', 'f', 'g', 'h',
		'i', 'j', 'k', 'l',
		'm', 'n', 'o', 'p',
		144 // CRC
	};
	
	for(uint8_t b : command) {
		CommReceiveByte(b);
	}
	EXPECT_EQ(command2_state, COMMAND_STATE_READ_LOCKED);
	EXPECT_EQ(command2_buffer[0], 'e');
	EXPECT_EQ(command2_buffer[1], 'f');
	EXPECT_EQ(command2_buffer[2], 'g');
	EXPECT_EQ(command2_buffer[3], 'h');
	
	CommReceiveLoop();
}
