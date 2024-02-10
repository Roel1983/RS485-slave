#include <gtest/gtest.h>

#include <avr/io.h>

#include "../receiver.hpp"

using namespace ::testing;
using namespace ::communication;

class CommunitionReceiverReceiver : public Test {
protected:
	void TearDown() override {
		receiver::teardown();
	}
};

TEST_F(CommunitionReceiverReceiver, begin) {
	receiver::setup();
	
#if F_CPU == 16000000
	EXPECT_EQ(UBRR0H, 0x00);
	EXPECT_EQ(UBRR0L, 0x10);
#elif F_CPU == 14745600
	EXPECT_EQ(UBRR0H, 0x00);
	EXPECT_EQ(UBRR0L, 0x0F);
#else
	FAIL() << "No expected value for F_CPU = " << F_CPU;
#endif
	
	EXPECT_TRUE(UCSR0A & (1<<U2X0));
	EXPECT_EQ(UCSR0C, (0<<UMSEL00) | (0<<UPM00) | (0<<USBS0)  | (3<<UCSZ00));
	EXPECT_EQ(UCSR0B, (1<<RXEN0)   | (1<<TXEN0) | (1<<TXCIE0) | (1<<RXCIE0));
}
