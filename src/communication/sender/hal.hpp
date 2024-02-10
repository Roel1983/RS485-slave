#ifndef COMMUNICATION_SENDER_HAL_H_
#define COMMUNICATION_SENDER_HAL_H_

#include <avr/interrupt.h>
#include <avr/io.h>

namespace communication {
namespace sender {
namespace hal {
	
#define ON_READY_TO_SEND_NEXT_BYTE() ISR(USART_UDRE_vect)
#define ON_LAST_BYTE_SEND_COMPLETE() ISR(USART_TX_vect)

PRIVATE INLINE void setup() {
	DDRC   |= _BV(3);
	UCSR0B |= _BV(TXEN0)
	       |  _BV(TXCIE0);
}

PRIVATE INLINE void enableReadyToSendInterrupt() {
	UCSR0B |= _BV(UDRIE0);
}

PRIVATE INLINE void disableReadyToSendInterrupt() {
	UCSR0B &= ~_BV(UDRIE0);
}

PRIVATE INLINE void sendByte(const uint8_t data_byte) {
	UDR0 = data_byte;
}

PRIVATE INLINE void txEnable() {
	PORTC |= _BV(3); // Possible race condition
}

PRIVATE INLINE void txDisable() {
	PORTC &= ~_BV(3); // Possible race condition
}

}}} // End of: namespace communication::sender::hal

#endif // COMMUNICATION_SENDER_HAL_H_
