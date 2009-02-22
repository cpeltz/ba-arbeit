#define __AVR_ATmega2561__
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "uart.h"
#include "motor.h"
#include "flags.h"
#include "definitions.h"
#include "led.h"
#include "fifo.h"

static uint8_t uart_EscapeCount = 0;
static uint8_t uart_TXBuffer[UART_TX_BUFFER_SIZE];
static uint8_t uart_RXBuffer[UART_RX_BUFFER_SIZE];
static fifo_t uart_TXfifo;
static fifo_t uart_RXfifo;

ISR(USART1_UDRE_vect) {
	if (uart_TXfifo.count > 0)
		// wenn Zeichen im Buffer, nächstes Zeichen senden
		UDR1 = fifo_Read(&uart_TXfifo);
	else
		// ansonsten UDRIE1 deaktivieren
		UCSR1B &= ~(1 << UDRIE1);
}

ISR(USART1_RX_vect) {
	uint8_t character = UDR1;

	led_switch(LED_BLUE, SINGLE);
	// Reset nach 3 ESCAPES
	if ((character == 27)) {
		if ((++uart_EscapeCount == 3)) {
			motor_Break(2);
			// Watchdog einschalten und endlosschleifen
			cli();
			wdt_reset();
			wdt_enable(4);
			while (1);
		}
	} else {
		uart_EscapeCount = 0;
	}

	if (!(fifo_Write(&uart_RXfifo, character))) {
		// setze RX_OVERFLOW Flag falls Buffer voll
		flag_set(UART_RX_OVERFLOW);
	}
}

void uart_init(void) {
	// UART Initialisieren
	UBRR1H = UBRR_VAL >> 8;
	UBRR1L = UBRR_VAL & 0xff;
	// Aktiviere RX, TX und RX Complete IRQ
	if (!flag_read(INTERFACE_TWI)) {
		UCSR1B = (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1);
	} else {
		// bei TWI Receiver ausschalten
		UCSR1B = (1 << TXEN1) | (1 << RXCIE1);
	}
	// Lösche IRQ Flags
	UCSR1A = (1 << RXC1) | (1 << TXC1);
	// Initialisiere RX und TX Buffer
	fifo_Init(&uart_TXfifo, uart_TXBuffer, UART_TX_BUFFER_SIZE);
	fifo_Init(&uart_RXfifo, uart_RXBuffer, UART_RX_BUFFER_SIZE);
}

uint8_t uart_PutChar(const uint8_t character) {
	// character in TXfifo schreiben
	uint8_t ret = fifo_Write(&uart_TXfifo, character);
	// und UDR Ready IRQ aktivieren
	UCSR1B |= (1 << UDRIE1);
	// fifo_Write Status zurückgeben
	return ret;
}

void uart_PutChar_Wait(const uint8_t character) {
	// Warte bis wieder Platz im TX Buffer
	while (uart_TXfifo.count == UART_TX_BUFFER_SIZE);
	uart_PutChar(character);
}

uint8_t uart_GetChar(void) {
	return fifo_Read(&uart_RXfifo);
}

uint8_t uart_Write(const uint8_t * data) {
	// gibt String auf UART aus
	while (*data) {
		if (!(uart_PutChar(*data++)))
			return 0;
	}
	return 1;
}
