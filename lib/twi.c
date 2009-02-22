
#define __AVR_ATmega2561__
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdlib.h>
#include "twi.h"
#include "debug.h"
#include "queue.h"
#include "flags.h"
#include "irq.h"
#include "parse.h"
#include "fifo.h"
#include "definitions.h"
#include "status.h"
#include "motor.h"
#include "led.h"

uint8_t twi_rx_state = 0;
uint8_t twi_tx_state = 0;
char twi_receive[32];
uint8_t twi_receive_length = 0;
drive_order_t twi_transfer;

uint8_t output = 0;
uint8_t output_offset = 0;
static uint8_t twi_comfifoBuffer[10];
static uint8_t twi_stateBuffer[10];
static fifo_t twi_comfifo;
static fifo_t twi_state;
static global_state_t temp_state;

static uint8_t twi_parameter_load(const uint8_t value);

static uint8_t twi_queue_output(uint8_t reset);

ISR(TWI_vect) {
	led_switch(LED_BLUE, SINGLE);
  
	uint8_t twi_status = TWSR & 0xf8;
	fifo_Write(&twi_state, twi_status);
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
			//debug_WriteInteger(PSTR("D:"), twi_data);

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
	parser_add_byte(twi_data);
}

void twi_init(void) {
	if (flag_Read(INTERFACE_TWI)) {
		// Slave Address = 84
		TWAR = 84;
		// TWI aktivieren
		TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
		fifo_Init(&twi_comfifo, twi_comfifoBuffer, 10);
		fifo_Init(&twi_state, twi_stateBuffer, 10); 
	}
}

static uint8_t twi_parameter_load(const uint8_t value) {
	uint8_t ret = 0;
	global_state_t status;
	tacho_t tacho;

	switch (value) {
		case 0:
			ret = 0;
		case 1:
			// High-Byte, Radposition, links
			// ret = (wheel_ReadPosition(0) >> 8);
			status_read(&status);
			ret = (status.position_left >> 8);
			break;
		case 2:
			// Low-Byte, Radposition, links
			// ret = (wheel_ReadPosition(0) & 0xff);
			status_read(&status);
			ret = (status.position_left & 0xff);
			break;
		case 3:
			// High-Byte, Radposition, rechts
			status_read(&status);
			ret = (status.position_right >> 8);
			break;
		case 4:
			// Low-Byte, Radposition, rechts
			status_read(&status);
			ret = (status.position_right & 0xff);
			break;
		case 5:
			// Tempo, links
			status_read(&status);
			ret = status.speed_left;
			break;
		case 6:
			// Tempo, rechts
			status_read(&status);
			ret = status.speed_right;
			break;
		case 7:
			// High-Byte, Differenz
			status_read(&status);
			ret = (status.difference >> 8);
			break;
		case 8:
			// Low-Byte, Differenz
			status_read(&status);
			ret = (status.difference & 0xff);
			break;
		case 9:
			// main_state[0]
			status_read(&status);
			ret = status.state_left;
			break;
		case 10:
			// main_state[1]
			status_read(&status);
			ret = status.state_right;
			break;
		case 11:
			// Einträge in der Queue
			ret = queue_Entries();
			break;
		case 12:
			// High-Byte, Tacho links
			wheel_GetTacho(&tacho);
			ret = (tacho.left >> 8);
			break;
		case 13:
			// Low_Byte, Tacho links
			wheel_GetTacho(&tacho);
			ret = (tacho.left & 0xf0);
			break;
		case 14:
			// High-Byte, Tacho rechts
			wheel_GetTacho(&tacho);
			ret = (tacho.right >> 8);
			break;
		case 15:
			// Low_Byte, Tacho rechts
			wheel_GetTacho(&tacho);
			ret = (tacho.right & 0xf0);
			break;
		case 16:
			// High-Byte, Tacho Differenz
			wheel_GetTacho(&tacho);
			ret = (tacho.difference >> 8);
			break;
		case 17:
			// Low_Byte, Tacho Difference
			wheel_GetTacho(&tacho);
			ret = (tacho.difference & 0xf0);
			break;

		default:
			ret = 0xff;
			break;
	}

	return ret;
}

static uint8_t twi_queue_output(uint8_t reset) {
	uint8_t ret = 0;

	static uint8_t output_state = 0;
	static uint8_t output_readposition = 0;
	static uint8_t output_entries = 0;
	static drive_order_t output_order;

	switch (reset) {
		case 1:
			// Queue-Ausgabe zurücksetzen
			output_state = 0;
			output_readposition = queue_ReadPosition();
			output_entries = queue_Entries();
			break;

		case 0:
			// Queue ab aktueller Leseposition ausgeben
			switch (output_state) {
				case 0:
					// Ausgabe initialisieren
					if (output_entries > 0) {
						output_entries--;
						twi_drive_order_output(&output_order, 0);
						queue_ReadEntry(output_readposition++, &output_order);
						ret = twi_drive_order_output(&output_order, 1);
						output_state = 1;
					} else {
						ret = 0x00;
					}
					break;

				case 1:
					// Ausgabe fortführen
					ret = twi_drive_order_output(&output_order, 1);
					if (ret == 0) {
						// Eintrag zu Ende
						if (output_entries)
							ret = '\r';
						output_state = 0;
					}
					break;
			}
			break;
	}
	return ret;
}

int16_t twi_fiforead(void) {
	return fifo_Read(&twi_comfifo);
}

void twi_fifoinit(void) {
	fifo_Init(&twi_comfifo, twi_comfifoBuffer, 10);
}

int16_t twi_statefifoRead(void) {
	return fifo_Read(&twi_state);
}
