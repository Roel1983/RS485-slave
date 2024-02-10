#include <avr/interrupt.h>

#include "../../macros.hpp"

#include "hal.hpp"

namespace communication {
namespace sender {
namespace hal {

int i = 0;

PRIVATE void __onReadyToSendNextByte() __attribute__ ((unused));
PRIVATE void __onReadyToSendNextByte() {i = 1;};
void onReadyToSendNextByte() __attribute__ ((weak, alias ("_ZN13communication6sender3halL23__onReadyToSendNextByteEv")));

PRIVATE void __onByteSendDone() __attribute__ ((unused));
PRIVATE void __onByteSendDone() {i = 1;};
void onByteSendDone() __attribute__ ((weak, alias ("_ZN13communication6sender3halL16__onByteSendDoneEv")));

ISR(USART_UDRE_vect) {
	onReadyToSendNextByte();
}

ISR(USART_TX_vect) {
	onByteSendDone();
}

}}} // End of: namespace communication::sender::hal
