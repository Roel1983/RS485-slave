#ifndef FAKEAVR_AVR_IO_H_
#define FAKEAVR_AVR_IO_H_

#include <stdint.h>

#include "sfr_defs.h"

void FakeIoReset();

#define PB0 0
#define PB1 1
#define PB2 2
#define PB3 3
#define PB4 4
#define PB5 5
#define PB6 6
#define PB7 7
extern uint8_t DDRB;
extern uint8_t PORTB;
extern uint8_t DDRC;
extern uint8_t PORTC;
extern uint8_t DDRD;
extern uint8_t PORTD;

extern uint8_t UCSR0A;
#define MPCM0 0
#define U2X0 1
#define UPE0 2
#define DOR0 3
#define FE0 4
#define UDRE0 5
#define TXC0 6
#define RXC0 7

extern uint8_t UCSR0B;
#define TXB80 0
#define RXB80 1
#define UCSZ02 2
#define TXEN0 3
#define RXEN0 4
#define UDRIE0 5
#define TXCIE0 6
#define RXCIE0 7

extern uint8_t UCSR0C;
#define UCPOL0 0
#define UCSZ00 1
#define UCPHA0 1
#define UCSZ01 2
#define UDORD0 2
#define USBS0 3
#define UPM00 4
#define UPM01 5
#define UMSEL00 6
#define UMSEL01 7

extern uint8_t UBRR0H;

extern uint8_t UBRR0L;

extern uint8_t UDR0;

#endif
