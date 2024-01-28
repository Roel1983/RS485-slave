#ifndef PINS_H_
#define PINS_H_

#include <avr/io.h>

typedef struct {
	volatile uint8_t& ddr;
	volatile uint8_t& port;
} pin_port_reg_t;

typedef struct {
	pin_port_reg_t port_reg;
	uint8_t        bit_mask;
} pin_reg_t;

struct active_high_pin_t {
	const pin_reg_t& pin;
	inline void Set()   const {pin.port_reg.port |=  pin.bit_mask;}
	inline void Reset() const {pin.port_reg.port &= ~pin.bit_mask;}
	explicit operator bool() const {return pin.port_reg.port & pin.bit_mask;}
};

struct active_low_pin_t{
	const pin_reg_t& pin;
	inline void Set() const   {pin.port_reg.port &= ~pin.bit_mask;}
	inline void Reset() const {pin.port_reg.port |=  pin.bit_mask;}
	explicit operator bool() const {return !(pin.port_reg.port & pin.bit_mask);}
};

constexpr pin_port_reg_t PIN_PORT_B = {DDRB, PORTB};
constexpr pin_port_reg_t PIN_PORT_C = {DDRC, PORTC};
constexpr pin_port_reg_t PIN_PORT_D = {DDRD, PORTD};

constexpr pin_reg_t PIN00 = {PIN_PORT_D, _BV(0)};
constexpr pin_reg_t PIN01 = {PIN_PORT_D, _BV(1)};
constexpr pin_reg_t PIN02 = {PIN_PORT_D, _BV(2)};
constexpr pin_reg_t PIN03 = {PIN_PORT_D, _BV(3)};
constexpr pin_reg_t PIN04 = {PIN_PORT_D, _BV(4)};
constexpr pin_reg_t PIN05 = {PIN_PORT_D, _BV(5)};
constexpr pin_reg_t PIN06 = {PIN_PORT_D, _BV(6)};
constexpr pin_reg_t PIN07 = {PIN_PORT_D, _BV(7)};
constexpr pin_reg_t PIN08 = {PIN_PORT_B, _BV(0)};
constexpr pin_reg_t PIN09 = {PIN_PORT_B, _BV(1)};
constexpr pin_reg_t PIN10 = {PIN_PORT_B, _BV(2)};
constexpr pin_reg_t PIN11 = {PIN_PORT_B, _BV(3)};
constexpr pin_reg_t PIN12 = {PIN_PORT_B, _BV(4)};
constexpr pin_reg_t PIN13 = {PIN_PORT_B, _BV(5)};
constexpr pin_reg_t PIN14 = {PIN_PORT_C, _BV(0)};
constexpr pin_reg_t PIN15 = {PIN_PORT_C, _BV(1)};
constexpr pin_reg_t PIN16 = {PIN_PORT_C, _BV(2)};
constexpr pin_reg_t PIN17 = {PIN_PORT_C, _BV(3)};

#endif
