#include "order.h"
#include "flags.h"
#include "order_functions.h"

typedef void (*order_function)(order_t *);
order_function order_array[16];
void order_array_init() {
	order_array[0]  = &extended_instruction;
	order_array[1]  = &reset_instruction;
	order_array[2]  = &register_instruction;
	order_array[3]  = &queue_instruction;
	order_array[4]  = &current_order_instruction;
	order_array[5]  = &drive_instruction;
	order_array[6]  = &pid_drive_instruction;
	order_array[7]  = &set_pid_instruction;
	order_array[8]  = 0;
	order_array[9]  = 0;
	order_array[10] = 0;
	order_array[11] = 0;
	order_array[12] = 0;
	order_array[13] = 0;
	order_array[14] = 0;
	order_array[15] = 0;
}

void order_init(order_t *order) {
	uint8_t i = 0;
	for(; i < 32; i++) {
		order->data[i] = 0;
	}
	order->status = ORDER_STATUS_INITIALIZED;
}

void order_copy(order_t *from, order_t *to) {
	uint8_t i = 0;
	for(; i < 32; i++)
		to->data[i] = from->data[i];
	to->status = from->status;
}

void order_process(order_t * const order) {
	// Dispatch first stage of processing
	order_array[order->data[0] & 0x0f](order);
}

void order_check_triggers(order_t * const order) {
	// Check the triggers
}
