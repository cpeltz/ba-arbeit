/*#define __AVR_ATmega2561__*/
#include <inttypes.h>
#include <avr/io.h>
#include "dip.h"
#include "definitions.h"

/**
 * @defgroup DIP_Module DIP-Switch Module
 * This module provides access to the onboard DIP-Switches.
 * @{
 */

/**
 *	Initializes the dip switches and the corresponding pull-up.
 */
void dip_init(void) {
	DIP_DDR &= ~((1 << DIP1) | (1 << DIP2) | (1 << DIP3) | (1 << DIP4));
	DIP_PORT |= (1 << DIP1) | (1 << DIP2) | (1 << DIP3) | (1 << DIP4);
}

/**
 *	Reads the state of one dip switch.
 *
 *	@param[in] bit The requested switch number. (0-3)
 *	@return <em>uint8_t</em> Returns 1 if dip is on, otherwise 0.
 */
uint8_t dip_read(const uint8_t bit) {
	return (((~DIP_PIN >> 4) & (1 << bit)) >> bit);
}
/*@}*/
