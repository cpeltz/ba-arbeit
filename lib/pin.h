#ifndef PIN_H
#define PIN_H

#include <avr/io.h>

#define pin_set_A(pin) (PORTA |= (1 << pin))
#define pin_set_C(pin) (PORTC |= (1 << pin))
#define pin_clear_A(pin) (PORTA &= ~(1 << pin))
#define pin_clear_C(pin) (PORTC &= ~(1 << pin))
#define pin_toggle_A(pin) (PORTA ^= (1 << pin))
#define pin_toggle_C(pin) (PORTC ^= (1 << pin))

#endif
