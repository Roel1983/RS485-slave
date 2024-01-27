#include <gtest/gtest.h>

#include <avr/io.h>

#include "../Macros.h"

#include "../Comm.h"

PRIVATE void CommSetupUsart();

TEST(Comm_CommSetupUsart, test) {
	CommSetupUsart();
	
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

TEST(Comm_CommLoop, receivedSingleBlockCommandType) {
}

TEST(Comm_CommLoop, receivedMultiBlockCommandType_complete) {
}

TEST(Comm_CommLoop, receivedMultiBlockCommandType_firstHalf) {
}

TEST(Comm_CommLoop, receivedMultiBlockCommandType_lastHalf) {
}

TEST(Comm_CommIsrRaiseError, noErrorYet) {
}

TEST(Comm_CommIsrRaiseError, hasOldError) {
}

TEST(Comm_CommIsrGetMyBlockNr, test) {
}

TEST(Comm_ISR_USART_RX_vect, signalError) {
}

TEST(Comm_ISR_USART_RX_vect, skipReadSkip) {
}

TEST(Comm_ISR_USART_RX_vect, receiveBroadCastCommand) {
}

TEST(Comm_ISR_USART_RX_vect, receiveDeviceCommand) {
}

TEST(Comm_ISR_USART_RX_vect, receiveStripCommand) {
}

TEST(Comm_CommIsrReceivePreamble, noPreamble) {
}

TEST(Comm_CommIsrReceivePreamble, halfPreamble) {
}

TEST(Comm_CommIsrReceivePreamble, completePreamble) {
}

TEST(Comm_CommIsrReceiveCommandId, invalidCommandId) {
}

TEST(Comm_CommIsrReceiveCommandId, broadcastCommand) {
}

TEST(Comm_CommIsrReceiveCommandIdBroadCast, commandStateLocked) {
}

TEST(Comm_CommIsrReceiveCommandIdBroadCast, commandStateUnlocked) {
}

TEST(Comm_CommIsrReceiveBlockNr, test) {
}

TEST(Comm_CommIsrReceiveBlockCount, commandStateLocked) {
}

TEST(Comm_CommIsrReceiveBlockCount, receiveBlocksLowerThenMine) {
}

TEST(Comm_CommIsrReceiveBlockCount, receiveFirstFewOfMineBlocks) {
}

TEST(Comm_CommIsrReceiveBlockCount, receiveFirstFewOfMineBlocksAndLower) {
}

TEST(Comm_CommIsrReceiveBlockCount, receiveAllMineBlocks) {
}

TEST(Comm_CommIsrReceiveBlockCount, receiveAllMineBlocksAndLower) {
}

TEST(Comm_CommIsrReceiveBlockCount, receiveAllMineBlocksAndHigher) {
}

TEST(Comm_CommIsrReceiveBlockCount, receiveLastFewOfMineBlocks) {
}

TEST(Comm_CommIsrReceiveBlockCount, receiveLastFewOfMineBlocksAndHigher) {
}

TEST(Comm_CommIsrReceiveBlockCount, receiveBlocksHigherThenMine) {
}

TEST(Comm_CommIsrReceiveCrc, missmatch) {
}

TEST(Comm_CommIsrReceiveCrc, match_CommandStateWriteLocked) {
}

TEST(Comm_CommIsrReceiveCrc, match_CommandStateNotWriteLocked) {
}

TEST(Comm_CommSetDeviceNr, test) {
}

TEST(Comm_CommSetStripNr, test) {
}
