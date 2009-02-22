#define __AVR_ATmega2561__
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "fifo.h"
#include "definitions.h"

// Routinen teilweise übernommen von http://www.roboternetz.de/wissen/index.php/FIFO_mit_avr-gcc
// Routines unchanged - till now

void fifo_Init(fifo_t * fifo, uint8_t * fifo_Buffer, const uint8_t fifo_Size) {
	fifo->count = 0;
	fifo->p_read = fifo->p_write = fifo_Buffer;
	fifo->read2end = fifo->write2end = fifo->size = fifo_Size;
}

uint8_t fifo_Write(fifo_t * fifo, const uint8_t fifo_Data) {
	if ((fifo->count >= fifo->size))
		return 0;

	uint8_t *p_write = fifo->p_write;

	*(p_write++) = fifo_Data;

	uint8_t write2end = fifo->write2end;

	if ((--write2end == 0)) {
		write2end = fifo->size;
		p_write -= write2end;
	}

	fifo->write2end = write2end;
	fifo->p_write = p_write;

	uint8_t sreg = SREG;
	cli();
	fifo->count++;
	SREG = sreg;

	return 1;
}

uint8_t fifo_Read(fifo_t * fifo) {
	if (!(fifo->count))
		return -1;

	uint8_t *p_read = fifo->p_read;
	uint8_t data = *(p_read++);
	uint8_t read2end = fifo->read2end;

	if ((--read2end == 0)) {
		read2end = fifo->size;
		// FIXME Was soll das p_read -= 0 ist p_read (read2end == 0 => true)
		p_read -= read2end;
	}

	fifo->p_read = p_read;
	fifo->read2end = read2end;

	uint8_t sreg = SREG;
	cli();
	fifo->count--;
	SREG = sreg;

	return data;
}
