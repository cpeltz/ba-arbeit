#ifndef DEBUG_H
#define DEBUG_H

#include <inttypes.h>

#ifdef DEBUG
	extern uint8_t DEBUG_ENABLE;
#endif

void debug_PutString(const char *data);
void debug_WriteString_P(const char *progmem_string);
void debug_WriteInteger(const char *message, const int16_t integer);

#endif
