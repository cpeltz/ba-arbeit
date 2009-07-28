#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "io.h"
#include "definitions.h"
#include "led.h"

/**
 * @defgroup UART_Module USART Module
 * Controls the low-level access to the USART interface.
 * @{
 */

extern uint8_t INTERFACE_TWI;
extern uint8_t DEBUG_ENABLE;
/**
 * The USART transmitter ISR.
 */
ISR(USART1_UDRE_vect) {
	if (INTERFACE_TWI) {
		UCSR1B &= ~(1 << UDRIE1);
	} else {
		if (io_obj_get_remaining_size() > 0) {
			UDR1 = io_get_next_transmission_byte();
		} else {
			io_obj_remove_current();
			if (io_obj_remaining() > 0) {
				UDR1 = io_get_next_transmission_byte();
			} else {
				UCSR1B &= ~(1 << UDRIE1);
			}
		}
	}
}

/**
 * The USART reciever ISR.
 */
ISR(USART1_RX_vect) {
	uint8_t value = UDR1;

	led_switch(LED_BLUE, SINGLE);
	_io_push(value);
}

/**
 * Initializes the USART Module.
 */
void uart_init(void) {
	// UART Initialisieren
	UBRR1H = UBRR_VAL >> 8;
	UBRR1L = UBRR_VAL & 0xff;
	// Aktiviere RX, TX und RX Complete IRQ
	if (!INTERFACE_TWI) {
		UCSR1B = (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1);
	} else if(DEBUG_ENABLE) {
		UCSR1B = (1 << TXEN1);
	}
	// Lösche IRQ Flags
	UCSR1A = (1 << RXC1) | (1 << TXC1);
}

/**
 * Starts the USART transmission.
 *
 * This is needed because of the nature of USART transmission in
 * contrast to I²C Bus transmissions (where the master tells us
 * when to transmit).
 */
void uart_start_transmission() {
	UCSR1B |= (1 << UDRIE1);
}
 
/**
 * Prints one character for debugging and waits till it was transmitted.
 *
 * @param[in] data The Character to be send per UART.
 */
void uart_put_debug(const uint8_t data) {
	if (!INTERFACE_TWI && !DEBUG_ENABLE)
		return;
	UDR1 = data;
	uart_start_transmission();
	while(UCSR1B & (1 << UDRIE1));
}
/*@}*/
