#ifndef UART_H
#define UART_H

#ifndef F_CPU
#define F_CPU 16000000L
#endif

#define BAUD 115200L

#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD-1000)

#define UART_TX_BUFFER_SIZE 128
#define UART_RX_BUFFER_SIZE 128

void uart_Init(void);
uint8_t uart_PutChar(const uint8_t character);
void uart_PutChar_Wait(const uint8_t character);
int16_t uart_GetChar(void);
uint8_t uart_Write(const uint8_t * data);

#endif
