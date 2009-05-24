#define __AVR_ATmega2561__
#include <inttypes.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "debug.h"
#include "flags.h"
#include "definitions.h"

static char buffer[10];

void debug_PutChar(const uint8_t data) {
	if (flag_Read(DEBUG_ENABLE))
		uart_PutChar_Wait(data);
}

void debug_PutChar_P(const uint8_t * data) {
	if (flag_Read(DEBUG_ENABLE))
		uart_PutChar_Wait(*data);
}

static void debug_SendESC(void) {
	debug_PutChar(27);
}

void debug_PutString(const char *data) {
	uint8_t character;

	while ((character = *data++)) {
		debug_PutChar(character);
	}
}

void debug_WriteString_P(const char *progmem_string) {
	uint8_t character;

	while ((character = pgm_read_byte(progmem_string++))) {
		debug_PutChar_P(&character);
	}
}

void debug_SendESCSequence(const char *sequence) {
	debug_SendESC();
	debug_PutChar('[');
	debug_PutString(sequence);
}

void debug_ResetTerminal(void) {
	debug_SendESC();
	debug_PutChar('c');
}

void debug_ClearScreen(void) {
	debug_SendESCSequence("2J");
}

void debug_GotoXY(const uint8_t x, const uint8_t y) {
	itoa(y, buffer, 10);
	debug_SendESCSequence(buffer);
	debug_PutChar(';');
	itoa(x, buffer, 10);
	debug_PutString(buffer);
	debug_PutChar('H');
}

void debug_CursorUp(const uint8_t y) {
	itoa(y, buffer, 10);
	debug_SendESCSequence(buffer);
	debug_PutChar('A');
}

void debug_ClearEOL(void) {
	debug_SendESCSequence("K");
}

void debug_NewLine(void) {
	debug_WriteString_P(PSTR("\r\n"));
}

void debug_WriteInteger(const char *progmem_message, const int16_t integer) {
	itoa(integer, buffer, 10);
	debug_WriteString_P(progmem_message);
	debug_PutString(buffer);
	debug_ClearEOL();
	debug_NewLine();
}
