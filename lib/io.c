#include "io.h"
#include "twi.h"
#include "uart.h"
#include "lcd.h"
#include "led.h"
#define IO_INBUFFER_SIZE 256
#define IO_OUTBUFFER_SIZE 256


static uint8_t in_buffer[IO_INBUFFER_SIZE];
static uint8_t out_buffer[IO_OUTBUFFER_SIZE];
static uint8_t obj_memory[IO_OUTBUFFER_SIZE]
uint8_t inpos_begin = 0, inpos_end = 1;
uint8_t outpos_begin = 0, outpos_end = 1;
uint8_t objpos_begin = 0, objpos_end = 1;

static uint8_t transmission_offset = 0;

void io_init(void) {
	twi_init();
	uart_init();
	led_init();
}

uint8_t io_get_available(void) {
	if (inpos_begin > inpos_end)
		return IO_INBUFFER_SIZE - (inpos_begin - inpos_end);
	else
		return inpos_end - inpos_begin;
}

// Gets one byte from the buffer
uint8_t io_get(uint8_t* value) {
	if ((inpos_begin + 1) % IO_INBUFFER_SIZE == inpos_end)
		return 0;
	*value = in_buffer[inpos_begin];
	inpos_begin = (inpos_begin + 1) % IO_INBUFFER_SIZE;
	return 1;
}

// Puts one byte in the buffer
uint8_t io_put(uint8_t value) {
	if (outpos_end == outpos_begin)
		return 0;
	out_buffer[outpos_end - 1] = value;
	outpos_end = (outpos_end + 1) % IO_OUTBUFFER_SIZE;
	return 1;
}

void _io_push(uint8_t value) {
	if (inpos_end == inpos_begin)
		return;
	in_buffer[inpos_end - 1] = value;
	inpos_end = (inpos_end + 1) % IO_INBUFFER_SIZE;
}

uint8_t io_get_next_transmission_byte(void) {
	if ((outpos_start + 1) % IO_OUTBUFFER_SIZE == outpos_end)
		return 0xff;
	if (transmission_offset >= io_obj_get_current_size())
		return 0xff;
	transmission_offset++;
	return out_buffer[outpos_begin + (transmission_offset - 1)];
}

uint8_t io_obj_get_current_size() {
	uint8_t obj_end = obj_memory[objpos_begin];
	if(obj_end > outpos_begin)
		return obj_end - outpos_begin;
	return IO_OUTBUFFER_SIZE - (outpos_begin - obj_end);
}

uint8_t io_obj_get_remaining_size() {
	return io_obj_get_current_size() - transmission_offset;
}

void io_obj_remove_current(void) {
	if ((objpos_begin + 1) % IO_OUTBUFFER_SIZE == objpos_end)
		return;
	objpos_begin++;
}

void io_obj_start(void) {
	if (obj_memory[objpos_end - 1] != outpos_end - 1)
		io_obj_end();
}

void io_obj_end(void) {
	obj_memory[objpos_end - 1] = outpos_end - 1;
	objpos_end = (objpos + 1) % IO_OUTBUFFER_SIZE;
}
