#ifndef UART_H
#define UART_H

#include "options.h"

#ifndef F_CPU
#define F_CPU 16000000L
#endif

#define UBRR_VAL ((F_CPU+UART_BAUD_RATE*8)/(UART_BAUD_RATE*16)-1)
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))
#define BAUD_ERROR ((BAUD_REAL*1000)/UART_BAUD_RATE-1000)

void uart_init(void);
void uart_start_transmission(void);
void uart_put_debug_char(const uint8_t data);

#endif
