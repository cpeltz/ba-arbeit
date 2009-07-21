#ifndef DEBUG_H
#define DEBUG_H

#include <inttypes.h>

/* Mostly unchanged. I took this part from the old Driver.
 * It's used to print out debugging Information.
 */

void debug_PutChar(const uint8_t data);
void debug_PutChar_P(const uint8_t * data);
void debug_PutString(const char *data);
void debug_WriteString_P(const char *progmem_string);
void debug_SendESCSequence(const char *sequence);
void debug_ResetTerminal(void);
void debug_ClearScreen(void);
void debug_GotoXY(const uint8_t x, const uint8_t y);
void debug_ClearEOL(void);
void debug_CursorUp(const uint8_t y);
void debug_NewLine(void);
void debug_WriteInteger(const char *message, const int16_t integer);

#endif
