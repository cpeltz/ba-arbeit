#include <avr/io.h>
#include "pin.h"

void pin_init(void) {
	DDRA = 0xff;
	DDRC = 0xff;
	PORTA = 0x00;
	PORTC = 0x00;
}

void pin_set_A(uint8_t pin) {
	PORTA |= (1 << pin);
}

void pin_set_C(uint8_t pin) {
	PORTC |= (1 << pin);
}

void pin_clear_A(uint8_t pin) {
	PORTA &= ~(1 << pin);
}

void pin_clear_C(uint8_t pin) {
	PORTC &= ~(1 << pin);
}

void pin_toggle_A(uint8_t pin) {
	PORTA ^= (1 << pin);
}

void pin_toggle_C(uint8_t pin) {
	PORTC ^= (1 << pin);
}

