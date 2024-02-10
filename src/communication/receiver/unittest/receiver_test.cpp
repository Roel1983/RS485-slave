#include <gtest/gtest.h>

#include <avr/io.h>

#include "../../../macros.hpp"
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
		teardown();
	}
};

TEST_F(CommunitionReceiverReceiver, test) {
	processIncommingByte((uint8_t)0x55);
	processIncommingByte((uint8_t)0x55);
	processIncommingByte((uint8_t)0xff);
	processIncommingByte((uint8_t)0x02);
	processIncommingByte((uint8_t)0x00);
	processIncommingByte((uint8_t)0x12);
	processIncommingByte((uint8_t)0x34);
	processIncommingByte((uint8_t)0xB9);
}
