#ifndef UART_H
#define UART_H

#include "options.h"

#ifndef F_CPU
#define F_CPU 16000000L
#endif

#define UBRR_VAL ((F_CPU+UART_BAUD_RATE*8)/(UART_BAUD_RATE*16)-1)
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))
#define BAUD_ERROR ((UART_BAUD_RATE_REAL*1000)/UART_BAUD_RATE-1000)

#define UART_TX_BUFFER_SIZE 128
#define UART_RX_BUFFER_SIZE 128

void uart_Init(void);
uint8_t uart_PutChar(const uint8_t character);
void uart_PutChar_Wait(const uint8_t character);
uint8_t uart_GetChar(void);
uint8_t uart_Write(const uint8_t * data);

#endif
