#include "order.h"
#include "../i2c_master.h"
#include <string.h>
#include <stdarg.h>

void order_init(order_t *order) {
	// Function to Initialize a order_t structure
	unsigned char i = 0;
	// Overwrite every piece of memory with 0
	// Have to do it that way because no memset available
	for(; i < ORDER_TYPE_MAX_LENGTH; i++) {
		order->dat[i] = 0;
	}
	// Set the position to write to
	order->pos = 0;
}

void intern_order_shift(order_t *order) {
	unsigned char i = ORDER_TYPE_MAX_LENGTH;
	for(; i > 1; i--) {
		order[i] = order[i - 1];
	}
	order->dat[0] = 0;
}

void order_shift(order_t *order, unsigned char times) {
	unsigned char i = 0;
	for(; i < times; i++) {
		intern_order_shift(order);
	}
}

void intern_order_unshift(order_t *order) {
	unsigned char i = 0;
	for(; i < ORDER_TYPE_MAX_LENGTH - 1; i++) {
		order[i] = order[i + 1];
	}
	order->dat[ORDER_TYPE_MAX_LENGTH - 1] = 0;
}

void order_unshift(order_t *order, unsigned char times) {
	unsigned char i = 0;
	for(; i < times; i++) {
		intern_order_unshift(order);
	}
}

unsigned char order_send(order_t *order) {
	unsigned char bytes_send = 0;
	order_shift(order, 1);
	order->dat[0] = TWI_ADDRESS;
	bytes_send = i2c_send(order->dat, order->pos + 1);
	order_unshift(order, 1);
	return bytes_send;
	//return i2c_send(order->data, order->pos + 1);
}

unsigned char order_send_and_recv_current_order(order_t *order) {
	order_send(order);
	order_init(order);
	order->dat[0] = TWI_ADDRESS;
	i2c_receive(order->dat, 2);
	return i2c_receive(order->dat, order->dat[1]);
}

unsigned char bytes_to_recv(order_t *order) {
	switch(order->dat[0]) {
		case 0x12:
		case 0x22:
		case 0x32:
			return 1;
		default:
			return 0;
	}
}

void order_send_and_recv(order_t *order) {
	order_shift(order, 1);
	order->dat[0] = TWI_ADDRESS;
	i2c_send(order->dat, order->pos + 1);
	order_init(order);
	order->dat[0] = TWI_ADDRESS;
	i2c_receive(order->dat, bytes_to_recv(order));
}

void order_set_type(order_t *order, unsigned char type) {
	order->dat[0] = type;
	order->pos = 1;
}

void order_add_params(order_t *order, char *format, ...) {
	int num = strlen(format);
	va_list arglist;
	int i = 0;

	va_start(arglist, format);
	for(; i < num; i++) {
		if(format[i] == '1') {
			order->dat[order->pos] = va_arg(arglist, unsigned char);
			order->pos += 1;
		} else if(format[i] == '2') {
			long arg = va_arg(arglist, long);
			order->dat[order->pos] = arg & 0xff00;
			order->dat[order->pos + 1] = arg & 0x00ff;
			order->pos += 2;
		}
	}
	va_end(arglist);
}