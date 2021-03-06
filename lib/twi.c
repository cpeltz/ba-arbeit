#include <avr/interrupt.h>
#include <inttypes.h>
#include "options.h"
#include "twi.h"
#include "definitions.h"
#include "io.h"
#include "led.h"
#include "pin.h"

/**
 * @defgroup TWI_Module I²C-Bus Module
 * This module controls the low level access and policy to the
 * I²C-Bus.
 * @{
 */

/**
 * Local variable to remember whether or not a transmission is going on.
 */
uint8_t transmission_underway = 0;

/**
 * ISR for the TWI (I²C) Interrupt.
 */
ISR(TWI_vect) {
  	// temporarily save the last Byte on the bus
	uint8_t twi_data = TWDR;

  	// Mask the prescaler bit out of the status
	switch (TWSR & 0xf8) {
		// The following status codes refer to the Slave Reciever Mode (see Chip Documentation Page 260)
		case 0x88:
			// Previously addressed with own
			// SLA+W; data has been received;
			// NOT ACK has been returned

			// The missing break statement is correct and wanted

			// Buffer is full and with the TWCR statement in the 0x80 case
			// we will no longer hear on our own Address until buffer is freed
			// but the just recieved byte will still be taken
		case 0x80:
			// Previously addressed with own
			// SLA+W; data has been received;
			// ACK has been returned


			_io_push(twi_data);

			if (io_get_free_buffer_size() <= 1) {
				TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
			} else {
				TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
			}
			break;
		case 0x60:
			// Own SLA+W has been received;
			// ACK has been returned
			
			//led_switch(LED_GREEN, SINGLE);
			if (io_get_free_buffer_size() <= 1) {
				TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
			} else {
				TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
			}
			break;
		case 0xa0:
			TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
			break;

		// From here on the status codes refer to the Slave Transmitter Mode (See Chip Documentation Page 263)
		case 0xa8:
			// Own SLA+R has been received;
			// ACK has been returned

			// The missing break statement is desired and correct.

			// We have to reset counting variable and remove the last obj before beginning a new transmission
			if (transmission_underway) {
				transmission_underway = 0;
				io_obj_remove_current();
			}
			transmission_underway = 1;

		case 0xb8:
			// Data byte in TWDR has been
			// transmitted; ACK has been
			// received

			// Put the next byte in the data register
			TWDR = io_get_next_transmission_byte();
			// Set the correct control bits: if there is more then 1 byte remaining to send, then
			// we have to await an ACK, otherwise we await a NOT ACK (thats what io_get_remaining_obj_size()
			// is for). The TWSTA and TWSTO get always set to 0 and TWINT is always set to 1;
			TWCR = ((io_obj_get_remaining_size() > 1) << TWEA) | (1 << TWIE) | (1 << TWINT) | (1 << TWEN);
			break;

		case 0xc8:
			// Last data byte in TWDR has been
			// transmitted (TWEA = "0"); ACK
			// has been received

			// The missing break is desired and correct.

			// Sending was successful, but we got an unwanted ack, but still remove the obj from the
			// internal buffer.			
		case 0xc0:
			// Data byte in TWDR has been
			// transmitted; NOT ACK has been
			// received

			// Only reset the status bit to retry the send the next time, as we couldn't complete
			transmission_underway = 0;
			io_reset_transmission_status();
			io_obj_remove_current();
			TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWIE) | (1 << TWEN);
			break;

		case 0x00:
			// Bus error due to an illegal
			// START or STOP condition

			// To make sure we send the object correctly
			transmission_underway = 0;
			io_reset_transmission_status();
			// Recover from Bus Error with TWSTA = 0, TWSTO = 1 and TWINT = 1. See Chip Documentation Page 264.
			TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);
			break;
	}
}

/**
 * Initializes the I²c Bus Module.
 */
void twi_init(void) {
	extern uint8_t INTERFACE_TWI;
	if (INTERFACE_TWI) {
		// Slave Address = 84
		TWAR = TWI_ADDRESS;
		// TWI aktivieren
		TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
	}
}
/*@}*/
