#include <gtest/gtest.h>

#include <avr/io.h>

#include "../../../macros.hpp"
#include "../../command_types.hpp"
#include "../receiver.hpp"

using namespace ::testing;
using namespace ::communication::receiver;

namespace communication {
namespace receiver {
extern PRIVATE INLINE void processIncommingByte(const uint8_t data_byte);
}}

class CommunitionReceiverReceiver : public Test {
protected:
	void TearDown() override {
		tearDown();
	}
};

TEST_F(CommunitionReceiverReceiver, requestToSendCommand) {
	::communication::commandTypeSetBlockNr(::communication::COMMAND_TYPE_UNIQUE_ID, 0);
	
	processIncommingByte((uint8_t)0x55);
	processIncommingByte((uint8_t)0x55);
	processIncommingByte((uint8_t)0xff); // Sender unique id
	processIncommingByte((uint8_t)0x00); // Command: Request to send
	processIncommingByte((uint8_t)0x02); // Length
	processIncommingByte((uint8_t)0x00); // unique id
	processIncommingByte((uint8_t)0x00); // max length
	processIncommingByte((uint8_t)0xFF); // crc
}

TEST_F(CommunitionReceiverReceiver, orangeLedCommand) {
	::communication::commandTypeSetBlockNr(::communication::COMMAND_TYPE_UNIQUE_ID, 0);
	
	processIncommingByte((uint8_t)0x55);
	processIncommingByte((uint8_t)0x55);
	processIncommingByte((uint8_t)0xff); // Sender unique id
	processIncommingByte((uint8_t)0x02); // Command: Toggle orange LED
	processIncommingByte((uint8_t)0x02); // Length
	processIncommingByte((uint8_t)0x00); // unique id
	processIncommingByte((uint8_t)0x00); // payload
	processIncommingByte((uint8_t)0xFD); // crc
	
	loop();
}
