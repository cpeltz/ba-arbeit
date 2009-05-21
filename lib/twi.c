#define __AVR_ATmega2561__
#include <avr/interrupt.h>
#include <inttypes.h>
#include "options.h"
#include "twi.h"
#include "flags.h"
#include "irq.h"
#include "definitions.h"
#include "led.h"

// Local variable to remember whether or not a transmission is going on
uint8_t transmission_underway = 0;

ISR(TWI_vect) {
	// Led the blue LED blink if we are in here
	led_switch(LED_BLUE, SINGLE);
  
  	// Mask the prescaler bit out of the status
	uint8_t twi_status = TWSR & 0xf8;
	uint8_t twi_data = 0;

	// Standard bit combination for the Control Register
	TWCR = ~((1 << TWSTA) | (1 << TWSTO)) | (1 << TWEA) | (1 << TWEN);

	switch (twi_status) {
		// The following status codes refer to the Slave Reciever Mode (see Chip Documentation Page 260)
		case 0x88:
			// Previously addressed with own
			// SLA+W; data has been received;
			// NOT ACK has been returned

			// The missing break statement is correct and wanted

			// Buffer is full and with the TWCR statement in the 0x60 case
			// we will no longer hear on our own Address until buffer is freed
			// but the just recieved byte will still be taken
		case 0x80:
			// Previously addressed with own
			// SLA+W; data has been received;
			// ACK has been returned

			// The missing break statement is correct and wanted

			twi_data = TWDR;
			_io_push(twi_data);

		case 0x60:
			// Own SLA+W has been received;
			// ACK has been returned
			
			if (io_get_available() <= 1)
				TWCR = ~((1 << TWSTA) | (1 << TWSTO) | (1 << TWEA)) | (1 << TWEN);
			break;

			twi_data = TWDR;
			_io_push(twi_data);
			break;

		// From here on the status codes refer to the Slave Transmitter Mode (See Chip Documentation Page 263)
		case 0xa8:
			// Own SLA+R has been received;
			// ACK has been returned

			// The missing break statement is wanted and right.

			// We have to reset counting variable and remove the last obj before beginning a new transmission
			// TODO Would be better if this was done after last byte is send but what is the status code
			// TODO (cont.) for that?
			if (transmission_underway) {
				transmission_underway = 0;
				io_obj_remove_current();
			}

		case 0xb8:
			// Data byte in TWDR has been
			// transmitted; ACK has been
			// received

			// Put the next byte in the data register
			TWDR = io_get_next_transmission_byte();
			transmission_underway = 1;
			// Set the correct control bits: if there is more then 1 byte remaining to send, then
			// we have to await an ACK, otherwise we await a NOT ACK (thats what io_get_remaining_obj_size()
			// is for). The TWSTA and TWSTO get always set to 0 and TWINT is always set to 1;
			TWCR = ~((1 << TWSTA) | (1 << TWSTO)) | ((io_obj_get_remaining_size() > 1) << TWEA) | (1 << TWEN);
			break;

		case 0xc8:
			// Last data byte in TWDR has been
			// transmitted (TWEA = "0"); ACK
			// has been received

			// The missing break is wanted and right.

			// Sending was succesful, but we got an unwanted ack, but still remove the obj from the
			// internal buffer.
			io_obj_remove_current();
		case 0xc0:
			// Data byte in TWDR has been
			// transmitted; NOT ACK has been
			// received

			// Only reset the status bit to retry the send the next time, as we couldn't complete
			transmission_underway = 0;
			io_reset_transmission_status();
			break;

		case 0x00:
			// Bus error due to an illegal
			// START or STOP condition

			// To make sure we send the object correctly
			transmission_underway = 0;
			io_reset_transmission_status();
			// Recover from Bus Error with TWSTA = 0, TWSTO = 1 and TWINT = 1. See Chip Documentation Page 264.
			TWCR = ~(1 << TWSTA) | (1 << TWSTO) | (1 << TWEN);
			break;
	}
	// Have to set TWINT here because after setting TWINT TW operation will start again and
	// access to TWSR, TWDR and TWAR are not safe anymore
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
