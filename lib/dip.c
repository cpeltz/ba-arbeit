#include <inttypes.h>
#include <avr/io.h>
#include "dip.h"
#include "definitions.h"

void
dip_init(void)
// DIP-Schalter Eingänge initialisieren + PullUp
{
  DIP_DDR &= ~((1 << DIP1) | (1 << DIP2) | (1 << DIP3) | (1 << DIP4));
  DIP_PORT |= (1 << DIP1) | (1 << DIP2) | (1 << DIP3) | (1 << DIP4);
}

uint8_t
dip_read(const uint8_t bit)
// Eingang lesen und 0 oder 1 zurückgeben
{
  return (((~DIP_PIN >> 4) & (1 << bit)) >> bit);
}
