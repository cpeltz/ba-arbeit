#include <stdlib.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "debug.h"
#include "flags.h"
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
 * Prints one character of a debug string.
 *
 * Prints only if DEBUG_ENABLE flag is asserted.
 * @param[in] data The character to be printed.
 */
void debug_PutChar(const uint8_t data) {
	extern uint8_t DEBUG_ENABLE;
	if (DEBUG_ENABLE)
		uart_put_debug(data);
}

/**
 * Prints a String for debugging purposes.
 *
 * @param[in] data The String to be printed
 */
void debug_PutString(const char *data) {
	uint8_t character;

	while ((character = *data++)) {
		debug_PutChar(character);
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
		debug_PutChar(character);
	}
}

/**
 * Helper function used by debug_ClearEOL() to send escape sequences.
 *
 * @param[in] sequence The Sequence which should be transmitted.
 */
void debug_SendESCSequence(const char *sequence) {
	debug_PutChar(27);
	debug_PutChar('[');
	debug_PutString(sequence);
}

/**
 * Helper function for debug_WriteInteger(). Sends an end-of-line.
 */
void debug_ClearEOL(void) {
	debug_SendESCSequence("K");
}

/**
 * Helper function for debug_WriteInteger(). Sends an new-line.
 */
void debug_NewLine(void) {
	debug_WriteString_P(PSTR("\n"));
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
	debug_ClearEOL();
	debug_NewLine();
}
/*@}*/
