#include <gtest/gtest.h>

#include "../Comm.h"

typedef enum {
	COMMAND_STATE_UNLOCKED,
	COMMAND_STATE_WRITE_LOCKED,
	COMMAND_STATE_READ_LOCKED,
} command_state_t;

extern PRIVATE uint8_t comm_receive_error_state;

extern PRIVATE uint8_t command1_state;
extern PRIVATE uint8_t command1_buffer[4];

extern PRIVATE uint8_t command2_state;
extern PRIVATE uint8_t command2_buffer[4];

uint8_t CalculateCrc(const uint8_t* buffer, size_t n) {
	uint8_t crc = 0;
	for (int i = 2; i < n; i++) {
		crc -= buffer[i];
	}
	return crc;
}

void send(const uint8_t* buffer, size_t len) {
	for(int i = 0; i < len; i++) {
		CommReceiveByte(buffer[i]);
	}
}

void sendCommand(const uint8_t* command, size_t len) {
	uint8_t crc = CalculateCrc(command, len);
	for(int i = 0; i < len; i++) {
		CommReceiveByte(command[i]);
	}
	CommReceiveByte(crc);
}

void expectEq(uint8_t* buffer, uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
	EXPECT_EQ(buffer[0], a);
	EXPECT_EQ(buffer[1], b);
	EXPECT_EQ(buffer[2], c);
	EXPECT_EQ(buffer[3], d);
}

TEST(CommReceive_CommReceiveByte, invalidCommand) {
	 CommReceiveUnittestReset();
	 
	 const uint8_t command[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		3, 
		'r', 'o', 'e', 'l'
	};
	sendCommand(command, sizeof(command));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_COMMAND_ID);
}

TEST(CommReceive_CommReceiveByte, broadcast) {
	 CommReceiveUnittestReset();
	 
	 const uint8_t command[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		0, // Command 0 (type BROADCAST)
		'r', 'o', 'e', 'l'
	};
	sendCommand(command, sizeof(command));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_NONE);
	EXPECT_EQ(command1_state, COMMAND_STATE_READ_LOCKED);
	expectEq(command1_buffer, 'r', 'o', 'e', 'l');
}

TEST(CommReceive_CommReceiveByte, broadcast_twice) {
	CommReceiveUnittestReset();
	 
	const uint8_t command[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		0, // Command 0 (type BROADCAST)
		'r', 'o', 'e', 'l'
	};
	sendCommand(command, sizeof(command));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_NONE);
	EXPECT_EQ(command1_state, COMMAND_STATE_READ_LOCKED);
	expectEq(command1_buffer, 'r', 'o', 'e', 'l');
	
	const uint8_t command2[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		0, // Command 0 (type BROADCAST)
		'f', 'o', 'o', 'l'
	};
	sendCommand(command2, sizeof(command2));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_NONE);
	EXPECT_EQ(command1_state, COMMAND_STATE_READ_LOCKED);
	expectEq(command1_buffer, 'r', 'o', 'e', 'l');
}

TEST(CommReceive_CommReceiveByte, broadcast_twice_loopInBetween) {
	CommReceiveUnittestReset();
	 
	const uint8_t command[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		0, // Command 0 (type BROADCAST)
		'r', 'o', 'e', 'l'
	};
	sendCommand(command, sizeof(command));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_NONE);
	EXPECT_EQ(command1_state, COMMAND_STATE_READ_LOCKED);
	expectEq(command1_buffer, 'r', 'o', 'e', 'l');
	
	CommReceiveLoop();
	
	const uint8_t command2[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		0, // Command 0 (type BROADCAST)
		'f', 'o', 'o', 'l'
	};
	sendCommand(command2, sizeof(command2));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_NONE);
	EXPECT_EQ(command1_state, COMMAND_STATE_READ_LOCKED);
	expectEq(command1_buffer, 'f', 'o', 'o', 'l');
}

TEST(CommReceive_CommReceiveByte, unsynced) {
	CommReceiveUnittestReset();
	 
	const uint8_t command[] = {
		 (uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		 0, // Command 0 (type BROADCAST)
		 'r', 'o', 'e', 'l'
	};
	send((uint8_t*)"foo", 3);
	sendCommand(command, sizeof(command));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_UNSYNC);
	EXPECT_EQ(command1_state, COMMAND_STATE_READ_LOCKED);
	expectEq(command1_buffer, 'r', 'o', 'e', 'l');
}

TEST(CommReceive_CommReceiveByte, deviceCommand_twice) {
	CommReceiveUnittestReset();
	 
	const uint8_t command[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		1, // Command 1 (type DEVICE)
		4, 2,
		'r', 'o', 'e', 'l',
		'r', 'o', 'e', 'l'
	};
	sendCommand(command, sizeof(command));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_NONE);
	EXPECT_EQ(command2_state, COMMAND_STATE_READ_LOCKED);
	
	const uint8_t command2[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		1, // Command 1 (type DEVICE)
		4, 2,
		'f', 'o', 'o', 'l',
		'f', 'o', 'o', 'l'
	};
	sendCommand(command2, sizeof(command2));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_NONE);
	EXPECT_EQ(command2_state, COMMAND_STATE_READ_LOCKED);
}

TEST(CommReceive_CommReceiveByte, deviceCommand_tooLow) {
	CommReceiveUnittestReset();
	 
	const uint8_t command[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		1, // Command 1 (type DEVICE)
		2, 2,
		'r', 'o', 'e', 'l',
		'r', 'o', 'e', 'l'
	};
	sendCommand(command, sizeof(command));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_NONE);
	EXPECT_EQ(command2_state, COMMAND_STATE_UNLOCKED);
}

TEST(CommReceive_CommReceiveByte, deviceCommand_overlap1) {
	CommReceiveUnittestReset();
	 
	const uint8_t command[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		1, // Command 1 (type DEVICE)
		3, 2,
		'r', 'o', 'e', 'l',
		'r', 'o', 'e', 'l'
	};
	sendCommand(command, sizeof(command));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_NONE);
	EXPECT_EQ(command2_state, COMMAND_STATE_READ_LOCKED);
}

TEST(CommReceive_CommReceiveByte, deviceCommand_overlap2) {
	CommReceiveUnittestReset();
	 
	const uint8_t command[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		1, // Command 1 (type DEVICE)
		4, 2,
		'r', 'o', 'e', 'l',
		'r', 'o', 'e', 'l'
	};
	sendCommand(command, sizeof(command));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_NONE);
	EXPECT_EQ(command2_state, COMMAND_STATE_READ_LOCKED);
}

TEST(CommReceive_CommReceiveByte, deviceCommand_match) {
	CommReceiveUnittestReset();
	 
	const uint8_t command[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		1, // Command 1 (type DEVICE)
		4, 1,
		'r', 'o', 'e', 'l'
	};
	sendCommand(command, sizeof(command));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_NONE);
	EXPECT_EQ(command2_state, COMMAND_STATE_READ_LOCKED);
}

TEST(CommReceive_CommReceiveByte, deviceCommand_tooHigh) {
	CommReceiveUnittestReset();
	 
	const uint8_t command[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		1, // Command 1 (type DEVICE)
		5, 2,
		'r', 'o', 'e', 'l',
		'r', 'o', 'e', 'l'
	};
	sendCommand(command, sizeof(command));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_NONE);
	EXPECT_EQ(command2_state, COMMAND_STATE_UNLOCKED);
}

TEST(CommReceive_CommReceiveByte, crc_error) {
	CommReceiveUnittestReset();
	 
	const uint8_t command[] = {
		(uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		1, // Command 1 (type DEVICE)
		5, 2,
		'r', 'o', 'e', 'l',
		'r', 'o', 'e', 'l',
		0x42
	};
	send(command, sizeof(command));
	
	EXPECT_EQ(comm_receive_error_state, COMM_RECEIVE_ERROR_STATE_CRC);
	EXPECT_EQ(command2_state, COMMAND_STATE_UNLOCKED);
}


//~ /*void CommReceiveReset() {
	//~ comm_receive_crc               = 0;
	//~ comm_receive_skip_before_count = 0;
	//~ comm_receive_write_count       = 0;
	//~ comm_receive_write_pos         = 0;
	//~ comm_receive_skip_after_count  = 0;
//~ }*/

//~ typedef enum {
	//~ COMMAND_STATE_UNLOCKED,
	//~ COMMAND_STATE_WRITE_LOCKED,
	//~ COMMAND_STATE_READ_LOCKED,
//~ } command_state_t;
//~ extern uint8_t command1_state;
//~ extern uint8_t command1_buffer[4];

//~ extern uint8_t command2_state;
//~ extern uint8_t command2_buffer[4];

//~ extern uint8_t command3_state;
//~ extern uint8_t command3_buffer[2*4];

//~ extern uint8_t   comm_receive_crc;


//~ TEST(CommReceive_CommReceiveByte, test) {
	//~ command1_state = COMMAND_STATE_UNLOCKED;
	
	//~ CommReceiveByte('f');           CommReceiveLoop();
	//~ CommReceiveByte('o');           CommReceiveLoop();
	//~ CommReceiveByte('o');           CommReceiveLoop();
	//~ CommReceiveByte((uint8_t)0xcc); CommReceiveLoop();
	//~ CommReceiveByte('o');           CommReceiveLoop();
	
	//~ CommReceiveByte((uint8_t)0xcc); CommReceiveLoop();
	//~ CommReceiveByte((uint8_t)0xcc); CommReceiveLoop();
	
	//~ EXPECT_EQ(command1_state, COMMAND_STATE_UNLOCKED);
	//~ CommReceiveByte(COMMMAND_TEST); CommReceiveLoop();
	//~ EXPECT_EQ(command1_state, COMMAND_STATE_WRITE_LOCKED);
	
	//~ CommReceiveByte('R'); CommReceiveLoop();
	//~ CommReceiveByte('o'); CommReceiveLoop();
	//~ CommReceiveByte('e'); CommReceiveLoop();
	//~ CommReceiveByte('l'); CommReceiveLoop();
	//~ CommReceiveByte(110); 
	
	
	//~ EXPECT_EQ(command1_state, COMMAND_STATE_READ_LOCKED);
	//~ EXPECT_EQ(command1_buffer[0], 'R');
	//~ EXPECT_EQ(command1_buffer[1], 'o');
	//~ EXPECT_EQ(command1_buffer[2], 'e');
	//~ EXPECT_EQ(command1_buffer[3], 'l');
	
	//~ CommReceiveLoop();
//~ }

//~ uint8_t CalculateCrc(const uint8_t* buffer, size_t n) {
	//~ uint8_t crc = 0;
	//~ for (int i = 2; i < n; i++) {
		//~ crc -= buffer[i];
	//~ }
	//~ return crc;
//~ }

//~ TEST(CommReceive_CommReceiveByte, test2) {
	//~ command2_state = COMMAND_STATE_UNLOCKED;
	
	//~ uint8_t command[] = {
		//~ (uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		//~ 1, // Command 1 (type DEVICE)
		//~ 3, // Device address
		//~ 4, // Device count
		//~ 'A', 'b', 'c', 'd',
		//~ 'e', 'f', 'g', 'h',
		//~ 'i', 'j', 'k', 'l',
		//~ 'm', 'n', 'o', 'p'
	//~ };
	
	//~ for(uint8_t b : command) {
		//~ CommReceiveByte(b);
	//~ }
	//~ CommReceiveByte(CalculateCrc(command, sizeof(command)));
	
	//~ EXPECT_EQ(command2_state, COMMAND_STATE_READ_LOCKED);
	//~ EXPECT_EQ(command2_buffer[0], 'e');
	//~ EXPECT_EQ(command2_buffer[1], 'f');
	//~ EXPECT_EQ(command2_buffer[2], 'g');
	//~ EXPECT_EQ(command2_buffer[3], 'h');
	
	//~ CommReceiveLoop();
//~ }

//~ TEST(CommReceive_CommReceiveByte, test3) {
	//~ command2_state = COMMAND_STATE_UNLOCKED;
	
	//~ uint8_t command[] = {
		//~ (uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		//~ 2,  // Command 2 (type STRIP)
		//~ 14, // Strip address
		//~ 4,  // Device count
		//~ 'a', 'b', // 14
		//~ 'c', 'd', // 15
		//~ 'e', 'f', // 16 <-
		//~ 'g', 'h', // 17 <-
	//~ };
	
	//~ for(uint8_t b : command) {
		//~ CommReceiveByte(b);
	//~ }
	//~ CommReceiveByte(CalculateCrc(command, sizeof(command)));
	
	//~ EXPECT_EQ(command3_state, COMMAND_STATE_READ_LOCKED);
	//~ EXPECT_EQ(command3_buffer[0], 'e');
	//~ EXPECT_EQ(command3_buffer[1], 'f');
	//~ EXPECT_EQ(command3_buffer[2], 'g');
	//~ EXPECT_EQ(command3_buffer[3], 'h');
	
	//~ CommReceiveLoop();
//~ }
//~ TEST(CommReceive_CommReceiveByte, test) {
	//~ command1_state = COMMAND_STATE_UNLOCKED;
	
	//~ CommReceiveByte('f');           CommReceiveLoop();
	//~ CommReceiveByte('o');           CommReceiveLoop();
	//~ CommReceiveByte('o');           CommReceiveLoop();
	//~ CommReceiveByte((uint8_t)0xcc); CommReceiveLoop();
	//~ CommReceiveByte('o');           CommReceiveLoop();
	
	//~ CommReceiveByte((uint8_t)0xcc); CommReceiveLoop();
	//~ CommReceiveByte((uint8_t)0xcc); CommReceiveLoop();
	
	//~ EXPECT_EQ(command1_state, COMMAND_STATE_UNLOCKED);
	//~ CommReceiveByte(COMMMAND_TEST); CommReceiveLoop();
	//~ EXPECT_EQ(command1_state, COMMAND_STATE_WRITE_LOCKED);
	
	//~ CommReceiveByte('R'); CommReceiveLoop();
	//~ CommReceiveByte('o'); CommReceiveLoop();
	//~ CommReceiveByte('e'); CommReceiveLoop();
	//~ CommReceiveByte('l'); CommReceiveLoop();
	//~ CommReceiveByte(110); 
	
	
	//~ EXPECT_EQ(command1_state, COMMAND_STATE_READ_LOCKED);
	//~ EXPECT_EQ(command1_buffer[0], 'R');
	//~ EXPECT_EQ(command1_buffer[1], 'o');
	//~ EXPECT_EQ(command1_buffer[2], 'e');
	//~ EXPECT_EQ(command1_buffer[3], 'l');
	
	//~ CommReceiveLoop();
//~ }

//~ uint8_t CalculateCrc(const uint8_t* buffer, size_t n) {
	//~ uint8_t crc = 0;
	//~ for (int i = 2; i < n; i++) {
		//~ crc -= buffer[i];
	//~ }
	//~ return crc;
//~ }

//~ TEST(CommReceive_CommReceiveByte, test2) {
	//~ command2_state = COMMAND_STATE_UNLOCKED;
	
	//~ uint8_t command[] = {
		//~ (uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		//~ 1, // Command 1 (type DEVICE)
		//~ 3, // Device address
		//~ 4, // Device count
		//~ 'A', 'b', 'c', 'd',
		//~ 'e', 'f', 'g', 'h',
		//~ 'i', 'j', 'k', 'l',
		//~ 'm', 'n', 'o', 'p'
	//~ };
	
	//~ for(uint8_t b : command) {
		//~ CommReceiveByte(b);
	//~ }
	//~ CommReceiveByte(CalculateCrc(command, sizeof(command)));
	
	//~ EXPECT_EQ(command2_state, COMMAND_STATE_READ_LOCKED);
	//~ EXPECT_EQ(command2_buffer[0], 'e');
	//~ EXPECT_EQ(command2_buffer[1], 'f');
	//~ EXPECT_EQ(command2_buffer[2], 'g');
	//~ EXPECT_EQ(command2_buffer[3], 'h');
	
	//~ CommReceiveLoop();
//~ }

//~ TEST(CommReceive_CommReceiveByte, test3) {
	//~ command2_state = COMMAND_STATE_UNLOCKED;
	
	//~ uint8_t command[] = {
		//~ (uint8_t)0xcc, (uint8_t)0xcc, // Preamble
		//~ 2,  // Command 2 (type STRIP)
		//~ 14, // Strip address
		//~ 4,  // Device count
		//~ 'a', 'b', // 14
		//~ 'c', 'd', // 15
		//~ 'e', 'f', // 16 <-
		//~ 'g', 'h', // 17 <-
	//~ };
	
	//~ for(uint8_t b : command) {
		//~ CommReceiveByte(b);
	//~ }
	//~ CommReceiveByte(CalculateCrc(command, sizeof(command)));
	
	//~ EXPECT_EQ(command3_state, COMMAND_STATE_READ_LOCKED);
	//~ EXPECT_EQ(command3_buffer[0], 'e');
	//~ EXPECT_EQ(command3_buffer[1], 'f');
	//~ EXPECT_EQ(command3_buffer[2], 'g');
	//~ EXPECT_EQ(command3_buffer[3], 'h');
	
	//~ CommReceiveLoop();
//~ }


