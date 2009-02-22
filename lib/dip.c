#define __AVR_ATmega2561__
#include <inttypes.h>
#include <avr/io.h>
#include "dip.h"
#include "definitions.h"

// DIP-Schalter Eingänge initialisieren + PullUp
void dip_init(void) {
	DIP_DDR &= ~((1 << DIP1) | (1 << DIP2) | (1 << DIP3) | (1 << DIP4));
	DIP_PORT |= (1 << DIP1) | (1 << DIP2) | (1 << DIP3) | (1 << DIP4);
}

// Eingang lesen und 0 oder 1 zurückgeben
uint8_t dip_read(const uint8_t bit) {
	return (((~DIP_PIN >> 4) & (1 << bit)) >> bit);
}
