#include "io.h"

uint8_t                  DDRB;
static constexpr uint8_t DDRB_initial_value = 0x00;

uint8_t                  PORTB;
static constexpr uint8_t PORTB_initial_value = 0x00;

uint8_t                  DDRC;
static constexpr uint8_t DDRC_initial_value = 0x00;

uint8_t                  PORTC;
static constexpr uint8_t PORTC_initial_value = 0x00;

uint8_t                  DDRD;
static constexpr uint8_t DDRD_initial_value = 0x00;

uint8_t                  PORTD;
static constexpr uint8_t PORTD_initial_value = 0x00;

                 uint8_t UCSR0A;
static constexpr uint8_t UCSR0A_initial_value = 0x20;

                 uint8_t UCSR0B;
static constexpr uint8_t UCSR0B_initial_value = 0x00;

                 uint8_t UCSR0C;
static constexpr uint8_t UCSR0C_initial_value = 0x06;

                 uint8_t UBRR0H;
static constexpr uint8_t UBRR0H_initial_value = 0x00;

                 uint8_t UBRR0L;
static constexpr uint8_t UBRR0L_initial_value = 0x00;

                 uint8_t UDR0;
static constexpr uint8_t UDR0_initial_value = 0x00;

                 uint8_t TCCR1A;
static constexpr uint8_t TCCR1A_initial_value = 0x00;

                 uint8_t TCCR1B;
static constexpr uint8_t TCCR1B_initial_value = 0x00;

                 uint8_t TCCR1C;
static constexpr uint8_t TCCR1C_initial_value = 0x00;

                 uint16_t OCR1A;
static constexpr uint16_t OCR1A_initial_value = 0x00;

                 uint8_t TIMSK1;
static constexpr uint8_t TIMSK1_initial_value = 0x00;

void FakeIoReset() {
	DDRB   = DDRB_initial_value;
	PORTB  = PORTB_initial_value;
	DDRC   = DDRC_initial_value;
	PORTC  = PORTC_initial_value;
	DDRD   = DDRD_initial_value;
	PORTD  = PORTD_initial_value;
	
	UCSR0A = UCSR0A_initial_value;
	UCSR0B = UCSR0B_initial_value;
	UCSR0C = UCSR0C_initial_value;
	
	UBRR0H = UBRR0H_initial_value;
	UBRR0L = UBRR0L_initial_value;
	
	UDR0   = UDR0_initial_value;
	
	TCCR1A = TCCR1A_initial_value;
	TCCR1B = TCCR1B_initial_value;
	TCCR1C = TCCR1C_initial_value;
	OCR1A  = OCR1A_initial_value;
	TIMSK1 = TIMSK1_initial_value;
}
