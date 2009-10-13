#include "order.h"
#include "parse.h"
#include "order_functions.h"
#include "debug.h"
#include <string.h>
#include <avr/pgmspace.h>

/**
 * @defgroup ORDER_Module Order Module
 * This module provides functions used with the
 * order_t Type and the general execution Framework
 * of orders.
 * @{
 */

/**
 * @file
 * Contains functions to deal with the order_t type.
 */

/**
 * Ordinary typedef to ease the writing.
 */
typedef void (*order_function)(order_t *);

/**
 * Call Table for the protocol functions.
 *
 * This array will be later filled with pointer to functions
 * that correspond with the protocol commands.
 */
order_function order_array[16];

/**
 * Setup the order function call table.
 */
void order_array_init(void) {
	// Initialize the Call Table with the appropriate functions
	// these are implemented in order_functions.c
	order_array[0]  = &extended_instruction;
	order_array[1]  = &control_instruction;
	order_array[2]  = &query_instruction;
	order_array[3]  = &drive_instruction;
	order_array[4]  = &advanced_drive_instruction;
	order_array[5]  = &set_pid_instruction;
	order_array[6]  = &option_instruction;
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

/**
 * Initialize the given order_t structure.
 *
 * All data bytes will be written to zero and the status
 * will be set to #ORDER_STATUS_INITIALIZED. Every order_t structure
 * has to be initialized before work is safe on it.
 * @param[out] order The order_t structure which should be initialized.
 */
void order_init(order_t *order) {
	// Function to Initialize a order_t structure
	// Overwrite every piece of memory with 0
	memset(order->data, 0, ORDER_TYPE_MAX_LENGTH);
	// Set the Status of the order to Initialized
	order->status = ORDER_STATUS_INITIALIZED;
}

/**
 * Copies the contents of from to to.
 *
 * @param[in] from The source order.
 * @param[out] to The destination order.
 * @todo Check whether or not memcpy(to, from, sizeof(order_t)) would work
 */
void order_copy(const order_t * const from, order_t *to) {
	// Used to copy the order data from one to another
	memcpy(to->data, from->data, ORDER_TYPE_MAX_LENGTH);
	to->status = from->status;
}

/**
 * Dispatch function for orders.
 *
 * To execute an order we need to call the apropriate order_function
 * located in the call table ::order_array. This function takes
 * the command part of the first byte and uses it to get the wanted
 * function. If there is no entry in the call table, then the order
 * will be set to #ORDER_STATUS_DONE. An order may be executed very
 * often. This behaviour is needed to control the drive on longer
 * movements, but also to periodically check whether or not the
 * triggers have been reached.
 * @param[in] order The order which should be executed.
 */
void order_process(order_t * const order) {
	// Dispatch first stage of processing
	// That means, call the right function
	if(order_array[order->data[0] & 0x0f] != 0) {
		order_array[order->data[0] & 0x0f](order);
	} else { // Set the Order status to done if there is no function for this order
		order->status |= ORDER_STATUS_DONE; 
	}
}

/**
 * Function to lookup the size of an order.
 *
 * @param[in] order The order, of which one needs the size.
 * @return <em>uint8_t</em> The number of Bytes the order is long.
 * @todo Maybe think about unifiing order_size and bytes_needed
 */
uint8_t order_size(const order_t * const order) {
	return parser_bytes_needed(order->data[0]);
}
/*@}*/
