#include <gtest/gtest.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include <queue>

#include "../Macros.h"

#include "../CommandLib.h"
#include "../Comm.h"
#include "../Comm_private.h"

using namespace ::testing;

extern PRIVATE volatile comm_error_t comm_error;
extern PRIVATE comm_recv_isr_t comm_recv_isr;

static int      OnReceive_cmd_broadcast_called;
static uint16_t	OnReceive_cmd_broadcast_value;
static int      OnReceive_cmd_strip_called;
static uint16_t OnReceive_cmd_strip_value;

extern PRIVATE INLINE void CommIsrRaiseError(comm_error_t error);
extern PRIVATE INLINE void CommIsrReceivePreamble(const uint8_t data_byte);
extern PRIVATE INLINE void CommIsrReceiveCommandId(const uint8_t data_byte);
extern PRIVATE INLINE void CommIsrReceiveCommandIdBroadCast();
extern PRIVATE INLINE void CommIsrReceiveBlockNr(const uint8_t data_byte);
extern PRIVATE INLINE void CommIsrReceiveBlockCount(const uint8_t data_byte);
extern PRIVATE INLINE void CommIsrReceiveCrc(const uint8_t data_byte);
extern PRIVATE INLINE uint8_t CommIsrGetMyBlockNr(command_type_t command_type);

extern ISR(USART_RX_vect);

using test_command_type_strip_t = uint16_t;
command_t<COMMAND_TYPE_STRIP, test_command_type_strip_t> test_command_type_strip;
bool OnReceive_test_command_type_strip(uint8_t relative_block_nr, const test_command_type_strip_t& value) {
	return true;
}
command_info_t command_info_test_command_type_strip(test_command_type_strip, OnReceive_test_command_type_strip);

std::queue<on_received_single_block<uint16_t>::function_t> expected_OnReceive_cmd_broadcast;
bool OnReceive_cmd_broadcast(const uint16_t& value) {
	auto& funcs = expected_OnReceive_cmd_broadcast;
	if(!funcs.empty()) {
		auto func = funcs.front();
		funcs.pop();
		return func(value);
	}
	EXPECT_TRUE(false) << "Unexpected call to: OnReceive_cmd_broadcast(" << value << ")";
	return true;
}

std::queue<on_received_single_block<uint16_t>::function_t> expected_OnReceive_cmd_device;
bool OnReceive_cmd_device(const uint16_t& value) {
	auto& funcs = expected_OnReceive_cmd_device;
	if(!funcs.empty()) {
		auto func = funcs.front();
		funcs.pop();
		return func(value);
	}
	EXPECT_TRUE(false) << "Unexpected call to: OnReceive_cmd_device(" << value << ")";
	return true;
}

std::queue<on_received_multi_block<uint16_t>::function_t> expected_OnReceive_cmd_strip;
bool OnReceive_cmd_strip(uint8_t relative_block_nr, const uint16_t& value) {
	auto& funcs = expected_OnReceive_cmd_strip;
	if(!funcs.empty()) {
		auto func = funcs.front();
		funcs.pop();
		return func(relative_block_nr, value);
	}
	EXPECT_TRUE(false) << "Unexpected call to: OnReceive_cmd_strip(" << value << ")";
	return true;
}

std::queue<on_received_single_block<uint16_t>::function_t> expected_OnReceive_cmd_broadcast_isr_allowed;
bool OnReceive_cmd_broadcast_isr_allowed(const uint16_t& value) {
	auto& funcs = expected_OnReceive_cmd_broadcast_isr_allowed;
	if(!funcs.empty()) {
		auto func = funcs.front();
		funcs.pop();
		return func(value);
	}
	EXPECT_TRUE(false) << "Unexpected call to: OnReceive_cmd_broadcast_isr_allowed(" << value << ")";
	return true;
}

class CommTest : public Test {
protected:
	void TearDown() override {
		CommReset();
		CommandLibReset();
		memset(&test_command_type_strip, 0x00, sizeof(test_command_type_strip));
		
		expected_OnReceive_cmd_broadcast = std::queue<on_received_single_block<uint16_t>::function_t>();
	}
};

TEST_F(CommTest, CommBegin) {
	CommBegin();
	
#if F_CPU == 16000000
	EXPECT_EQ(UBRR0H, 0x00);
	EXPECT_EQ(UBRR0L, 0x10);
#elif F_CPU == 14745000
	EXPECT_EQ(UBRR0H, 0x00);
	EXPECT_EQ(UBRR0L, 0x0F);
#else
	FAIL("");
#endif
	
	EXPECT_TRUE(UCSR0A & (1<<U2X0));
	EXPECT_EQ(UCSR0C, (0<<UMSEL00) | (0<<UPM00) | (0<<USBS0)  | (3<<UCSZ00));
	EXPECT_EQ(UCSR0B, (1<<RXEN0)   | (1<<TXEN0) | (1<<TXCIE0) | (1<<RXCIE0));
}

//~ TEST_F(CommTest, CommLoop_receivedNothing) {
	//~ CommLoop();
	
	//~ EXPECT_EQ(OnReceive_cmd_broadcast_called, 0);
	//~ EXPECT_EQ(OnReceive_cmd_strip_called, 0);
//~ }

//~ TEST_F(CommTest, CommLoop_receivingNotFinished) {
	//~ cmd_broadcast.state = COMMAND_STATE_WRITE_LOCKED;
	
	//~ CommLoop();
	
	//~ EXPECT_EQ(cmd_broadcast.state, COMMAND_STATE_WRITE_LOCKED);
	//~ EXPECT_EQ(OnReceive_cmd_broadcast_called, 0);
//~ }

//~ TEST_F(CommTest, CommLoop_receivedSingleBlockCommandType) {
	//~ cmd_broadcast.state = COMMAND_STATE_READ_LOCKED;
	//~ cmd_strip.processed_block_bits = 0x0001;
	
	//~ CommLoop();
	
	//~ EXPECT_EQ(cmd_broadcast.state, COMMAND_STATE_UNLOCKED);
	//~ EXPECT_EQ(OnReceive_cmd_broadcast_called, 1);
	//~ // TODO check parameters
//~ }

//~ TEST_F(CommTest, CommLoop_receivedMultiBlockCommandType_complete) {
	//~ cmd_strip.state = COMMAND_STATE_READ_LOCKED;
	//~ cmd_strip.processed_block_bits = 0x0001;
	
	//~ CommLoop();
	
	//~ EXPECT_EQ(cmd_strip.state, COMMAND_STATE_UNLOCKED);
	//~ EXPECT_EQ(OnReceive_cmd_strip_called, 1);
	//~ // TODO check parameters
//~ }


TEST_F(CommTest, CommIsrRaiseError_noErrorYet) {
	comm_error = COMM_ERROR_NONE;
	
	CommIsrRaiseError(COMM_ERROR_SIGNAL);
	
	EXPECT_EQ(comm_error, COMM_ERROR_SIGNAL);
}

TEST_F(CommTest, CommIsrRaiseError_hasOldError) {
	comm_error = COMM_ERROR_BUSY;
	
	CommIsrRaiseError(COMM_ERROR_SIGNAL);
	
	EXPECT_EQ(comm_error, COMM_ERROR_BUSY);
}

TEST_F(CommTest, CommGetError) {
	comm_error = COMM_ERROR_BUSY;
	
	EXPECT_EQ(CommGetError(), COMM_ERROR_BUSY);
	
	EXPECT_EQ(comm_error, COMM_ERROR_NONE);
}

static void ISR_USART_RX_vect_signalError(uint8_t ucsr0a) {
	comm_error = COMM_ERROR_NONE;
	comm_recv_isr.state, COMM_STATE_CRC;
	
	UDR0 = 0x42;
	UCSR0A |= ucsr0a;
	
	isr_USART_RX_vect();
	
	EXPECT_EQ(comm_recv_isr.state, COMM_STATE_PREAMBLE);
	EXPECT_EQ(comm_error,     COMM_ERROR_SIGNAL);
}

TEST_F(CommTest, ISR_USART_RX_vect_signalError_frameError) {
	ISR_USART_RX_vect_signalError(1 << FE0);
}

TEST_F(CommTest, ISR_USART_RX_vect_signalError_dataOverRun) {
	ISR_USART_RX_vect_signalError(1 << DOR0);
}

TEST_F(CommTest, ISR_USART_RX_vect_skipReadSkip) {
	comm_recv_isr.crc = 0;
	comm_recv_isr.state= COMM_STATE_CRC;
	comm_recv_isr.command_info = &command_info_test_command_type_strip;
	
	uint8_t buffer[20];
	memset(buffer, 0xFF, sizeof(buffer));
	
	comm_recv_isr.skip_byte_count = 3;
	comm_recv_isr.read_byte_count = 4;
	comm_recv_isr.read_byte_pos   = buffer;
	comm_recv_isr.skip_byte_after_read_count = 5;
	
	uint8_t expected_crc = 0;
	for(int i = 0; i < 3 + 4 + 5 && !HasFailure(); i++) {
		SCOPED_TRACE(i);
		
		UDR0 = i;
		isr_USART_RX_vect();
		
		expected_crc += i;
		EXPECT_EQ(comm_recv_isr.crc, expected_crc);
		
		EXPECT_EQ(comm_error, COMM_ERROR_NONE);
		EXPECT_EQ(comm_recv_isr.state, COMM_STATE_CRC);
	}
	
	EXPECT_EQ(buffer[0], 3);
	EXPECT_EQ(buffer[1], 4);
	EXPECT_EQ(buffer[2], 5);
	EXPECT_EQ(buffer[3], 6);
	EXPECT_EQ(buffer[4], 0xFF);
	
	UDR0 = 0x100 - expected_crc;
	isr_USART_RX_vect();
	
	EXPECT_EQ(comm_error, COMM_ERROR_NONE);
	EXPECT_EQ(comm_recv_isr.state, COMM_STATE_PREAMBLE);
}

TEST_F(CommTest, ISR_USART_RX_vect_invalidCommState) {
	comm_recv_isr.state = (comm_state_t)0xff;
	
	isr_USART_RX_vect();
	
	EXPECT_EQ(comm_recv_isr.state, COMM_STATE_PREAMBLE);
}

TEST_F(CommTest, CommIsrReceivePreamble_noPreamble) {
	comm_error     = COMM_ERROR_NONE;
	comm_recv_isr.state = COMM_STATE_PREAMBLE;
	
	CommIsrReceivePreamble(0xAA);
	
	EXPECT_EQ(comm_error,     COMM_ERROR_DATA);
	EXPECT_EQ(comm_recv_isr.state, COMM_STATE_PREAMBLE);
}

TEST_F(CommTest, CommIsrReceivePreamble_halfPreamble) {
	comm_error     = COMM_ERROR_NONE;
	comm_recv_isr.state = COMM_STATE_PREAMBLE;
	
	CommIsrReceivePreamble(0x55);
	CommIsrReceivePreamble(0xAA);
	
	EXPECT_EQ(comm_error,     COMM_ERROR_DATA);
	EXPECT_EQ(comm_recv_isr.state, COMM_STATE_PREAMBLE);
}

TEST_F(CommTest, CommIsrReceivePreamble_completePreamble) {
	comm_error     = COMM_ERROR_NONE;
	comm_recv_isr.state = COMM_STATE_PREAMBLE;
	
	CommIsrReceivePreamble(0x55);
	CommIsrReceivePreamble(0x55);
	
	EXPECT_EQ(comm_error,     COMM_ERROR_NONE);
	EXPECT_EQ(comm_recv_isr.state, COMM_STATE_COMMAND_ID);
}

TEST_F(CommTest, CommIsrReceiveCommandId_invalidCommandId) {
	comm_error     = COMM_ERROR_NONE;
	comm_recv_isr.state = COMM_STATE_COMMAND_ID;
	
	CommIsrReceiveCommandId(0xff);
	
	EXPECT_EQ(comm_error,     COMM_ERROR_DATA);
	EXPECT_EQ(comm_recv_isr.state, COMM_STATE_PREAMBLE);
}

TEST_F(CommTest, CommIsrReceiveCommandId) {
	comm_error     = COMM_ERROR_NONE;
	comm_recv_isr.state = COMM_STATE_COMMAND_ID;
	
	CommIsrReceiveCommandId(COMMAND_ID_cmd_device);
	
	EXPECT_EQ(comm_error,     COMM_ERROR_NONE);
	EXPECT_EQ(comm_recv_isr.command_info, &command_infos[COMMAND_ID_cmd_device]);
	EXPECT_EQ(comm_recv_isr.state, COMM_STATE_BLOCK_NR);
}

TEST_F(CommTest, CommIsrReceiveCommandIdBroadCast_commandStateLocked) {
	comm_recv_isr.skip_byte_count = 0xFF;
	comm_recv_isr.read_byte_count = 0xFF;
	comm_recv_isr.skip_byte_after_read_count = 0xFF;
	
	comm_error     = COMM_ERROR_NONE;
	comm_recv_isr.state = COMM_STATE_COMMAND_ID;
	
	cmd_broadcast.state =COMMAND_STATE_READ_LOCKED;
	
	CommIsrReceiveCommandId(COMMAND_ID_cmd_broadcast);
	
	EXPECT_EQ(comm_error,     COMM_ERROR_BUSY);
	EXPECT_EQ(comm_recv_isr.command_info, &command_infos[COMMAND_ID_cmd_broadcast]);
	EXPECT_EQ(comm_recv_isr.skip_byte_count, sizeof(uint16_t));
	EXPECT_EQ(comm_recv_isr.read_byte_count, 0);
	EXPECT_EQ(comm_recv_isr.skip_byte_after_read_count, 0);
	EXPECT_EQ(comm_recv_isr.state, COMM_STATE_CRC);
}

TEST_F(CommTest, CommIsrReceiveCommandIdBroadCast_commandStateUnlocked) {
	comm_error     = COMM_ERROR_NONE;
	comm_recv_isr.state = COMM_STATE_COMMAND_ID;
	
	cmd_broadcast.state = COMMAND_STATE_UNLOCKED;
	
	CommIsrReceiveCommandId(COMMAND_ID_cmd_broadcast);
	
	EXPECT_EQ(comm_error,               COMM_ERROR_NONE);
	EXPECT_EQ(cmd_broadcast.state,      COMMAND_STATE_WRITE_LOCKED);
	EXPECT_EQ(comm_recv_isr.read_byte_pos,   (void*)cmd_broadcast.buffer);
	EXPECT_EQ(comm_recv_isr.read_byte_count, sizeof(uint16_t));	
	EXPECT_EQ(comm_recv_isr.state,           COMM_STATE_CRC);
}

TEST_F(CommTest, CommIsrReceiveBlockNr) {
	comm_recv_isr.block_nr = 0;
	comm_recv_isr.state = COMM_STATE_BLOCK_NR;
	
	CommIsrReceiveBlockNr(42);
	
	EXPECT_EQ(comm_recv_isr.block_nr, 42);
	EXPECT_EQ(comm_recv_isr.state, COMM_STATE_BLOCK_COUNT);
}

TEST_F(CommTest, CommIsrReceiveBlockCount_commandStateLocked) {
	
	CommSetStripNr(3);
	
	comm_error = COMM_ERROR_NONE;
	
	comm_recv_isr.state = COMM_STATE_BLOCK_COUNT;
	comm_recv_isr.command_info = &command_info_test_command_type_strip;
	command_info_test_command_type_strip.command.state = COMMAND_STATE_READ_LOCKED;
	
	comm_recv_isr.skip_byte_count            = 0xff;
	comm_recv_isr.read_byte_count            = 0xff;
	comm_recv_isr.skip_byte_after_read_count = 0xff;
		
	comm_recv_isr.block_nr = 0;
	CommIsrReceiveBlockCount(10);
	
	constexpr int block_size = sizeof(test_command_type_strip_t);
	EXPECT_EQ(comm_recv_isr.skip_byte_count,            10 * block_size);
	EXPECT_EQ(comm_recv_isr.read_byte_count,            0);
	EXPECT_EQ(comm_recv_isr.skip_byte_after_read_count, 0);
	
	EXPECT_EQ(command_info_test_command_type_strip.command.state, COMMAND_STATE_READ_LOCKED);
	EXPECT_EQ(comm_recv_isr.state,                      COMM_STATE_CRC);
	EXPECT_EQ(comm_error,                          COMM_ERROR_BUSY);
}

#define testCommIsrReceiveBlockCount(args...) {SCOPED_TRACE("testCommIsrReceiveBlockCount"); _testCommIsrReceiveBlockCount(args);}
static void _testCommIsrReceiveBlockCount(
	int my_block_nr,
	int send_block_nr, int send_block_count,
	
	int expected_skip_block_count,
	int expected_read_block_count,
	int expected_skip_byte_after_read_block_count,
	
	int expected_write_pos,
	
	uint8_t expected_processed_block_bits,
	
	command_state_t expected_command_state
) {
	CommSetStripNr(my_block_nr);
	
	comm_error = COMM_ERROR_NONE;
	
	comm_recv_isr.state = COMM_STATE_BLOCK_COUNT;
	comm_recv_isr.command_info = &command_info_test_command_type_strip;
	command_info_test_command_type_strip.command.state = COMMAND_STATE_UNLOCKED;
	
	comm_recv_isr.skip_byte_count            = 0xff;
	comm_recv_isr.read_byte_count            = 0xff;
	comm_recv_isr.skip_byte_after_read_count = 0xff;
		
	comm_recv_isr.block_nr = send_block_nr;
	CommIsrReceiveBlockCount(send_block_count);
	
	constexpr int block_size = sizeof(test_command_type_strip_t);
	EXPECT_EQ(comm_recv_isr.skip_byte_count,            expected_skip_block_count * block_size);
	EXPECT_EQ(comm_recv_isr.read_byte_count,            expected_read_block_count * block_size);
	if(expected_read_block_count) {
		EXPECT_EQ(
				comm_recv_isr.read_byte_pos,
				&command_info_test_command_type_strip.command.buffer[expected_write_pos * block_size]
		);
		EXPECT_EQ(
				command_info_test_command_type_strip.command.processed_block_bits,
				expected_processed_block_bits
		);
	}
	EXPECT_EQ(comm_recv_isr.skip_byte_after_read_count, expected_skip_byte_after_read_block_count * block_size);
	
	EXPECT_EQ(command_info_test_command_type_strip.command.state, expected_command_state);
	EXPECT_EQ(comm_recv_isr.state,                      COMM_STATE_CRC);
	
	EXPECT_EQ(comm_error,                          COMM_ERROR_NONE);
}
/* expected_processed_block_bits ---------------------------+ */
/* expected_write_pos      -----------------------------+   | */
/* expected_skip_byte_after_read_block_count -------+   |   | */
/* expected_read_block_count --------------------+  |   |   | */
/* expected_skip_block_count -----------------+  |  |   |   | */
/* send_block_count        --------------+    |  |  |   |   | */
/* send_block_nr           -----------+  |    |  |  |   |   | */
/* my_block_nr             ------+    |  |    |  |  |   |   | */
/*                               v    v  v    v  v  v   v   v */

TEST_F(CommTest, CommIsrReceiveBlockCount_receiveBlocksLowerThenMine) {
	testCommIsrReceiveBlockCount(6,   1, 4,   4, 0, 0,  0,  0b0000, COMMAND_STATE_UNLOCKED);
	testCommIsrReceiveBlockCount(6,   1, 5,   5, 0, 0,  0,  0b0000, COMMAND_STATE_UNLOCKED);
	testCommIsrReceiveBlockCount(6,   5, 1,   1, 0, 0,  0,  0b0000, COMMAND_STATE_UNLOCKED);
}

TEST_F(CommTest, CommIsrReceiveBlockCount_receiveFirstFewOfMineBlocks) {
	testCommIsrReceiveBlockCount(6,   6, 1,   0, 1, 0,  0,  0b0001, COMMAND_STATE_WRITE_LOCKED);
	testCommIsrReceiveBlockCount(6,   6, 3,   0, 3, 0,  0,  0b0111, COMMAND_STATE_WRITE_LOCKED);
}

TEST_F(CommTest, CommIsrReceiveBlockCount_receiveFirstFewOfMineBlocksAndLower) {
	testCommIsrReceiveBlockCount(6,   5, 2,   1, 1, 0,  0,  0b0001, COMMAND_STATE_WRITE_LOCKED);
	testCommIsrReceiveBlockCount(6,   5, 4,   1, 3, 0,  0,  0b0111, COMMAND_STATE_WRITE_LOCKED);
}

TEST_F(CommTest, CommIsrReceiveBlockCount_receiveAllMineBlocksAndLower) {
	testCommIsrReceiveBlockCount(3,   2, 5,   1, 4, 0,  0,  0b1111, COMMAND_STATE_WRITE_LOCKED);
}

TEST_F(CommTest, CommIsrReceiveBlockCount_receiveAllMineBlocks) {
	testCommIsrReceiveBlockCount(3,   3, 4,   0, 4, 0,  0,  0b1111, COMMAND_STATE_WRITE_LOCKED);
}

TEST_F(CommTest, CommIsrReceiveBlockCount_receiveAllMineBlocksAndHigher) {
	testCommIsrReceiveBlockCount(3,   3, 5,   0, 4, 1,  0,  0b1111, COMMAND_STATE_WRITE_LOCKED);
}

TEST_F(CommTest, CommIsrReceiveBlockCount_receiveAllMineBlocksAndLowerAndHigher) {
	testCommIsrReceiveBlockCount(3,   1, 7,   2, 4, 1,  0,  0b1111, COMMAND_STATE_WRITE_LOCKED);
}

TEST_F(CommTest, CommIsrReceiveBlockCount_receiveMiddleFewOfMineBlocks) {
	testCommIsrReceiveBlockCount(3,   4, 2,   0, 2, 0,  1,  0b0110, COMMAND_STATE_WRITE_LOCKED);
}

TEST_F(CommTest, CommIsrReceiveBlockCount_receiveLastFewOfMineBlocks) {
	testCommIsrReceiveBlockCount(4,   5, 3,   0, 3, 0,  1,  0b1110, COMMAND_STATE_WRITE_LOCKED);
	testCommIsrReceiveBlockCount(4,   7, 1,   0, 1, 0,  3,  0b1000, COMMAND_STATE_WRITE_LOCKED);
}

TEST_F(CommTest, CommIsrReceiveBlockCount_receiveLastFewOfMineBlocksAndHigher) {
	testCommIsrReceiveBlockCount(4,   5, 4,   0, 3, 1,  1,  0b1110, COMMAND_STATE_WRITE_LOCKED);
	testCommIsrReceiveBlockCount(4,   7, 3,   0, 1, 2,  3,  0b1000, COMMAND_STATE_WRITE_LOCKED);
}

TEST_F(CommTest, CommIsrReceiveBlockCount_receiveBlocksHigherThenMine) {
	testCommIsrReceiveBlockCount(4,   8, 4,   0, 0, 4,  0,  0b0000, COMMAND_STATE_UNLOCKED);
	testCommIsrReceiveBlockCount(4,   9, 8,   0, 0, 8,  0,  0b0000, COMMAND_STATE_UNLOCKED);
}

#define testCommIsrReceiveCrc(args...) {SCOPED_TRACE("testCommIsrReceiveCrc"); _testCommIsrReceiveCrc(args);}
static void _testCommIsrReceiveCrc(
	uint8_t         crc,
	command_state_t command_state,
	command_state_t expected_command_state,
	comm_error_t    expected_comm_error,
	comm_state_t    expected_comm_state
	
) {
	comm_error = COMM_ERROR_NONE;
	comm_recv_isr.state = COMM_STATE_CRC;
	
	comm_recv_isr.command_info = &command_info_test_command_type_strip;
	command_info_test_command_type_strip.command.state = command_state;
	
	comm_recv_isr.crc = crc;
	CommIsrReceiveCrc(0x42);
	
	EXPECT_EQ(command_info_test_command_type_strip.command.state, expected_command_state);
	EXPECT_EQ(comm_error,     expected_comm_error);
	EXPECT_EQ(comm_recv_isr.state, expected_comm_state);
}

TEST_F(CommTest, CommIsrReceiveCrc_missmatch_CommandStateWriteLocked) {
	testCommIsrReceiveCrc(~0, COMMAND_STATE_WRITE_LOCKED,
			COMMAND_STATE_UNLOCKED, COMM_ERROR_DATA, COMM_STATE_PREAMBLE);
}

TEST_F(CommTest, CommIsrReceiveCrc_missmatch_CommandStateUnlocked) {
	testCommIsrReceiveCrc(~0, COMMAND_STATE_UNLOCKED,
			COMMAND_STATE_UNLOCKED, COMM_ERROR_DATA, COMM_STATE_PREAMBLE);
}

TEST_F(CommTest, CommIsrReceiveCrc_match_CommandStateWriteLocked) {
	testCommIsrReceiveCrc(0, COMMAND_STATE_WRITE_LOCKED,
			COMMAND_STATE_READ_LOCKED, COMM_ERROR_NONE, COMM_STATE_PREAMBLE);
}

TEST_F(CommTest, CommIsrReceiveCrc_match_CommandStateUnlocked) {
	testCommIsrReceiveCrc(0, COMMAND_STATE_UNLOCKED,
			COMMAND_STATE_UNLOCKED, COMM_ERROR_NONE, COMM_STATE_PREAMBLE);
}

TEST_F(CommTest, CommSetDeviceNr) {
	EXPECT_EQ(CommIsrGetMyBlockNr(COMMAND_TYPE_DEVICE), 0);
	CommSetDeviceNr(42);
	EXPECT_EQ(CommIsrGetMyBlockNr(COMMAND_TYPE_DEVICE), 42);
}

TEST_F(CommTest, CommSetStripNr_test) {
	EXPECT_EQ(CommIsrGetMyBlockNr(COMMAND_TYPE_STRIP), 0);
	CommSetStripNr(42);
	EXPECT_EQ(CommIsrGetMyBlockNr(COMMAND_TYPE_STRIP), 42);
}

static uint8_t crc = 0x00;
void SendCrc() {
	UDR0 = 0x100 - crc;
	isr_USART_RX_vect();
}

void Send(uint8_t *buffer, size_t size, bool add_crc = false) {
	crc = 0x00;
	for (int i = 0; i < size; i++) {
		UDR0 = buffer[i];
		isr_USART_RX_vect();
		crc += buffer[i];
	}
	if(add_crc) {
		SendCrc();
	}
}

void SendPreamble() {
	uint8_t buffer[] = {0x55, 0x55};
	Send(buffer, sizeof(buffer));
}

TEST_F(CommTest, ISR_USART_RX_vect_receiveBroadCastCommand) {
	uint8_t buffer[] = {
		COMMAND_ID_cmd_broadcast,
		0x12,
		0x34
	};
	SendPreamble();
	Send(buffer, sizeof(buffer), /*add_crc=*/true);
	
	EXPECT_EQ(comm_error,     COMM_ERROR_NONE);
	EXPECT_EQ(comm_recv_isr.state, COMM_STATE_PREAMBLE);
	EXPECT_EQ(cmd_broadcast.state, COMMAND_STATE_READ_LOCKED);
	EXPECT_EQ(cmd_broadcast.buffer[0], 0x3412);
	
	expected_OnReceive_cmd_broadcast.push([](const uint16_t& value){
		EXPECT_EQ(value, 0x3412);
		return false;
	});
	CommLoop();
	ASSERT_TRUE(expected_OnReceive_cmd_broadcast.empty());
	
	expected_OnReceive_cmd_broadcast.push([](const uint16_t& value){
		EXPECT_EQ(value, 0x3412);
		return true;
	});
	CommLoop();
	ASSERT_TRUE(expected_OnReceive_cmd_broadcast.empty());
	
	CommLoop();
}

TEST_F(CommTest, ISR_USART_RX_vect_receiveDeviceCommand) {
	CommSetDeviceNr(2);
	
	uint8_t buffer[] = {
		COMMAND_ID_cmd_device,
		1,
		3,
		0x11,
		0x12,
		0x21,
		0x22,
		0x31,
		0x31
	};
	SendPreamble();
	Send(buffer, sizeof(buffer), /*add_crc=*/true);
	
	EXPECT_EQ(comm_error,       COMM_ERROR_NONE);
	EXPECT_EQ(comm_recv_isr.state,   COMM_STATE_PREAMBLE);
	EXPECT_EQ(cmd_device.state, COMMAND_STATE_READ_LOCKED);
	EXPECT_EQ(cmd_device.buffer[0], 0x2221);
	
	expected_OnReceive_cmd_device.push([](const uint16_t& value){
		EXPECT_EQ(value, 0x2221);
		return false;
	});
	CommLoop();
	ASSERT_TRUE(expected_OnReceive_cmd_device.empty());
	
	expected_OnReceive_cmd_device.push([](const uint16_t& value){
		EXPECT_EQ(value, 0x2221);
		return true;
	});
	CommLoop();
	ASSERT_TRUE(expected_OnReceive_cmd_device.empty());
	
	CommLoop();
}

TEST_F(CommTest, ISR_USART_RX_vect_receiveStripCommand) {
	CommSetStripNr(2);
	
	uint8_t buffer[] = {
		COMMAND_ID_cmd_strip,
		1,
		3,
		0x11,
		0x11,
		0x22,
		0x22,
		0x33,
		0x33
	};
	SendPreamble();
	Send(buffer, sizeof(buffer), /*add_crc=*/true);
	
	EXPECT_EQ(comm_error,       COMM_ERROR_NONE);
	EXPECT_EQ(comm_recv_isr.state,   COMM_STATE_PREAMBLE);
	EXPECT_EQ(cmd_strip.state, COMMAND_STATE_READ_LOCKED);
	EXPECT_EQ(cmd_strip.buffer[0], 0x2222);
	EXPECT_EQ(cmd_strip.buffer[1], 0x3333);
	
	expected_OnReceive_cmd_strip.push([](uint8_t relative_block_nr, const uint16_t& value){
		EXPECT_EQ(relative_block_nr, 0);
		EXPECT_EQ(value, 0x2222);
		return true;
	});
	expected_OnReceive_cmd_strip.push([](uint8_t relative_block_nr, const uint16_t& value){
		EXPECT_EQ(relative_block_nr, 1);
		EXPECT_EQ(value, 0x3333);
		return false;
	});
	CommLoop();
	ASSERT_TRUE(expected_OnReceive_cmd_strip.empty());
	
	expected_OnReceive_cmd_strip.push([](uint8_t relative_block_nr, const uint16_t& value){
		EXPECT_EQ(relative_block_nr, 1);
		EXPECT_EQ(value, 0x3333);
		return true;
	});
	CommLoop();
	ASSERT_TRUE(expected_OnReceive_cmd_strip.empty());
	
	CommLoop();
}

TEST_F(CommTest, ISR_USART_RX_vect_receiveBroadcast_isrAllowed_Command) {
	CommSetStripNr(2);
	
	uint8_t buffer[] = {
		COMMAND_ID_cmd_broadcast_isr_allowed,
		0x12,
		0x34
	};
	SendPreamble();
	Send(buffer, sizeof(buffer), /*add_crc=*/false);
	
	expected_OnReceive_cmd_broadcast_isr_allowed.push([](const uint16_t& value){
		EXPECT_EQ(value, 0x3412);
		return false;
	});
	SendCrc();
	ASSERT_TRUE(expected_OnReceive_cmd_broadcast_isr_allowed.empty());
	
	EXPECT_EQ(comm_error,     COMM_ERROR_NONE);
	EXPECT_EQ(comm_recv_isr.state, COMM_STATE_PREAMBLE);
	EXPECT_EQ(cmd_broadcast_isr_allowed.state, COMMAND_STATE_READ_LOCKED);
	EXPECT_EQ(cmd_broadcast_isr_allowed.buffer[0], 0x3412);
	
	expected_OnReceive_cmd_broadcast_isr_allowed.push([](const uint16_t& value){
		EXPECT_EQ(value, 0x3412);
		return false;
	});
	CommLoop();
	ASSERT_TRUE(expected_OnReceive_cmd_broadcast_isr_allowed.empty());
	
	expected_OnReceive_cmd_broadcast_isr_allowed.push([](const uint16_t& value){
		EXPECT_EQ(value, 0x3412);
		return true;
	});
	CommLoop();
	ASSERT_TRUE(expected_OnReceive_cmd_broadcast_isr_allowed.empty());
	
	CommLoop();
}
