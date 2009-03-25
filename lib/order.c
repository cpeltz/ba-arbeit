#include "order.h"
#include "flags.h"
#include "order_functions.h"

// Ordinary typedef to ease the writing
typedef void (*order_function)(order_t *);

// Call Table for the protocol functions
order_function order_array[16];

void order_array_init() {
	// Initialize the Call Table with the appropriate functions
	// these are implemented in order_functions.c
	order_array[0]  = &extended_instruction;
	order_array[1]  = &control_instruction;
	order_array[2]  = &register_instruction;
	order_array[3]  = &drive_instruction;
	order_array[4]  = &set_pid_instruction;
	order_array[5]  = &pid_drive_instruction;
	order_array[6]  = 0;
	order_array[7]  = 0;
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
	// Function to Initialize a order_t structure
	uint8_t i = 0;
	// Overwrite every piece of memory with 0
	// Have to do it that way because no memset available
	for(; i < 32; i++) {
		order->data[i] = 0;
	}
	// Set the Status of the order to Initialized
	order->status = ORDER_STATUS_INITIALIZED;
}

void order_copy(order_t *from, order_t *to) {
	// Used to copy the order data from one to another
	uint8_t i = 0;
	for(; i < 32; i++)
		to->data[i] = from->data[i];
	to->status = from->status;
}

void order_process(order_t * const order) {
	// Dispatch first stage of processing
	// That means, call the right function
	if(order_array[order->data[0] & 0x0f] != 0)
		order_array[order->data[0] & 0x0f](order);
	else // Set the Order status to done if there is no function for this order
		order->status |= ORDER_STATUS_DONE; 
}

void order_check_triggers(order_t * const order) {
	// Check the triggers of the order
	// TODO Implement
}
