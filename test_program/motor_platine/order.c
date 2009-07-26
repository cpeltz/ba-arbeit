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
	order->dat[0] = TWI_ADDRESS;
	order->pos = 1;
}

unsigned char order_send(order_t *order) {
	return i2c_send(order->dat, order->pos);
}

unsigned char order_send_and_recv_co(order_t *order) {
	order_send(order);
	order_init(order);
	i2c_receive(order->dat, 2);
	return i2c_receive(order->dat, order->dat[1]);
}

unsigned char bytes_to_recv(order_t *order) {
	switch(order->dat[0]) {
		case ORDER_REGISTER_LEFT_SPEED:
		case ORDER_REGISTER_RIGHT_SPEED:
		case ORDER_REGISTER_QUEUE_SIZE:
			return 1;
		default:
			return 0;
	}
}

unsigned char order_send_and_recv(order_t *order) {
	if (order->dat[1] == 0x42) // for recv a current order one needs the other function.
		return;
	i2c_send(order->dat, order->pos + 1);
	order_init(order);
	return i2c_receive(order->dat, bytes_to_recv(order) + 1);
}

void order_set_type(order_t *order, unsigned char type) {
	order->dat[1] = type;
	order->pos = 2;
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
