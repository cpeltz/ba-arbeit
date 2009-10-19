#ifndef DEBUG_H
#define DEBUG_H

#include <inttypes.h>
#include <avr/pgmspace.h>

#ifdef DEBUG
	extern uint8_t DEBUG_ENABLE;
#endif

void debug_write_string(const char *data);
void debug_write_string_p(const char *progmem_string);
void debug_write_integer(const char *message, const int16_t integer);

#endif
