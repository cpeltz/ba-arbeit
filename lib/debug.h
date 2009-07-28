#ifndef DEBUG_H
#define DEBUG_H

#include <inttypes.h>

void debug_PutChar(const uint8_t data);
void debug_PutChar_P(const uint8_t * data);
void debug_PutString(const char *data);
void debug_WriteString_P(const char *progmem_string);
void debug_WriteInteger(const char *message, const int16_t integer);

#endif
