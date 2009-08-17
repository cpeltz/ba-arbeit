#include <stdlib.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "debug.h"
#include "definitions.h"

/**
 * @defgroup DEBUG_MODULE Debug Module
 * Module used to print debug information on the UART device.
 * @{
 */

/**
 * Buffer used by debug_WriteInteger and the itoa call.
 */
static char buffer[10];

/**
 * Prints a String for debugging purposes.
 *
 * @param[in] data The String to be printed
 */
void debug_PutString(const char *data) {
	uint8_t character;

	while ((character = *data++)) {
		uart_put_debug_char(character);
	}
}

/**
 * Prints a String saved in the Program-Memory.
 *
 * @param[in] progmem_string The Program-Memory string to be printed.
 */
void debug_WriteString_P(const char *progmem_string) {
	uint8_t character;

	while ((character = pgm_read_byte(progmem_string++))) {
		debug_put_debug_char(character);
	}
}

/**
 * Prints a program-memory string in conjunction with a integer value.
 *
 * @param[in] progmem_message The Message in program-memory.
 * @param[in] integer The integer which will printed alongside the message.
 */
void debug_WriteInteger(const char *progmem_message, const int16_t integer) {
	itoa(integer, buffer, 10);
	debug_WriteString_P(progmem_message);
	debug_PutString(buffer);
	debug_WriteString_P(PSTR("\n"));
}
/*@}*/
