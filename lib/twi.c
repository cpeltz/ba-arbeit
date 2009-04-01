
#define __AVR_ATmega2561__
#include <avr/interrupt.h>
#include <inttypes.h>
#include "twi.h"
#include "flags.h"
#include "irq.h"
#include "fifo.h"
#include "definitions.h"
#include "led.h"

uint8_t twi_rx_state = 0;
uint8_t twi_tx_state = 0;

ISR(TWI_vect) {
	led_switch(LED_BLUE, SINGLE);
  
	uint8_t twi_status = TWSR & 0xf8;
	char twi_data = 0; // TODO Really a char? Check documentation!

	switch (twi_status) {
		case 0x60:
			// Own SLA+W has been received;
			// ACK has been returned
			twi_rx_state = 0;

			TWCR &= ~(1 << TWSTO);
			TWCR |= (1 << TWEA);
			break;

		case 0x80:
			// Previously addressed with own
			// SLA+W; data has been received;
			// ACK has been returned

			twi_data = TWDR;
			_io_push(twi_data);

			TWCR &= ~(1 << TWSTO);
			TWCR |= (1 << TWEA);


		case 0xa0:
			// A STOP condition or repeated
			// START condition has been
			// received while still addressed as
			// Slave

			TWCR &= ~((1 << TWSTA) | (1 << TWSTO));
			TWCR |= (1 << TWEA);
			break;

		case 0xa8:
			// Own SLA+R has been received;
			// ACK has been returned


		case 0xb8:
			// Data byte in TWDR has been
			// transmitted; ACK has been
			// received


		case 0xc0:
			// Data byte in TWDR has been
			// transmitted; NOT ACK has been
			// received

		case 0xc8:
			// Last data byte in TWDR has been
			// transmitted (TWEA = "0"); ACK
			// has been received
			twi_tx_state = 0;

			TWCR &= ~((1 << TWSTA) | (1 << TWSTO));
			TWCR |= (1 << TWEA);
			break;

		case 0x00:
			// Bus error due to an illegal
			// START or STOP condition

			flag_Set(RESET_TWI); // FIXME check whether or not this thing works
			break;
	}
	TWCR |= (1 << TWINT);
}

void twi_init(void) {
	if (flag_read(INTERFACE_TWI)) {
		// Slave Address = 84
		TWAR = TWI_ADDRESS;
		// TWI aktivieren
		TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
	}
}
