#include "parse.h"
#include "io.h"
#include "order.h"

/**
 * @defgroup PARSER_Module Parser
 * This Modules is used to convert the raw byte stream from
 * the incoming interfaces to orders which which the system
 * can work.
 * @{
 */

/**
 * Ring Buffer of orders, holds #PARSER_ORDER_BUFFER_SIZE orders
 */
order_t parser_order_buffer[PARSER_ORDER_BUFFER_SIZE];
/**
 * Holds the current position in the Ring
 */
int8_t current_buffer_position = 0;
/**
 * Holds the position of the first order in the Ring
 */
int8_t first_buffer_position = -1;
/**
 * Holds the current position IN the order structure
 */
uint8_t current_order_position = 0;

void parser_add_byte(uint8_t);

/**
 * Initializes the Parser Module.
 *
 * Mainly needed to init the ring buffer of orders.
 */
void parser_init(void) {
	// Initialize the Ringbuffer
	uint8_t i = 0;
	for(; i < PARSER_ORDER_BUFFER_SIZE; i++) {
		order_init(&parser_order_buffer[0]);
	}
}

/**
 * Updates the parser.
 *
 * Takes all available bytes from the incoming buffer and
 * then puts them in the parser.
 * @todo Abort when parser_add_byte fails, or better get a
 * test function to check for more space.
 */
void parser_update(void) {
	uint8_t value = 0, times = io_get_available();
	for (; 0 < times; times--) {
		io_get(&value);
		parser_add_byte(value);
	}
}

/**
 * Checks if an extended order is complete.
 *
 * Not implemented because there are no extended orders at the
 * moment.
 */
int parser_extended_order_complete(const order_t* order, uint8_t num_bytes) { 
	//Doesn't really do anything, because there aren't any extended orders at the moment
	return 1;
}

/**
 * Returns the number of bytes needed for the particular order.
 *
 * @param[in] order The command byte of the order (first byte).
 * @return <em>uint8_t</em> The number of bytes needed for this order to
 * be complete.
 * @todo Maybe if the value was once calculated it should be cached somewhere
 */
uint8_t bytes_needed(uint8_t order) {
	uint8_t ret_value = 0;
	switch(order & 0x0f) {
		case 1: //Control Instruction
		case 2: //Register Query Instruction
			return 1; //These are all one byte Instructions
		case 3: //Drive Instruction is a variable byte order
			ret_value = 3; //min. three bytes are neccessary: one as order, two for speed (left and right)
			if((order & 0x30) == 0x30) { // Drive Instruction with differntial correction
				ret_value = 2;
				if((order & 0xc0) == 0xc0) { // Set differential correction value flag
					ret_value = 3;
				} else {
					if(order & 0x40 || order & 0x80) { // Time/Position Trigger, needs two extra bytes
						ret_value += 2;
					}
				}
			} else { // Normal drive command
				if(order & 0x10 || order & 0x20) { //left Wheel Time/Position Trigger, needs two extra bytes
					ret_value += 2;
				}
				if(order & 0x40 || order & 0x80) { //right Wheel Time/Position Trigger, needs two extra bytes
					ret_value += 2;
				}
			}
			return ret_value;
		case 4: //Set PID Parameters Instruction
			return 9;
	}
	return 1;
}

/**
 * Test whether a order is complete.
 *
 * @param[in] order The order, which gets tested.
 * @param[in] num_bytes The number of bytes already in the order.
 * @return <em>int</em> 1 if the order is complete, otherwise 0.
 */
int parser_order_complete(const order_t* order, uint8_t num_bytes) { 
	if( (order->data[0] & 0x0f) == 0 )
		return parser_extended_order_complete(order, num_bytes);
	else if( (order->data[0] & 0x0f) <= 8 ) {
		// Call "bytes_needed()", a simple helper function
		// to determine whether or not the order has the right
		// amount of parameters and therefor bytes.
		if(num_bytes >= bytes_needed(order->data[0]))
			return 1;
	}
	return 0;
}

/**
 * Adds a byte to the order.
 *
 * @param[in] byte The byte which should be added to the order.
 * @todo Look for optimizations, this function has to be as fast as possible
 */
void parser_add_byte(uint8_t byte) { 
	// This function simple adds another byte to the current order
	// or discards the byte if the buffer is full.
	if( current_buffer_position == first_buffer_position ) {
		return; // Discard, buffer full
	}
	// Put the byte at its position and increment
	parser_order_buffer[current_buffer_position].data[current_order_position] = byte;
	current_order_position++;
	if (current_order_position >= ORDER_TYPE_MAX_LENGTH || parser_order_complete(&parser_order_buffer[current_buffer_position], current_order_position + 1)) {
		// if the order is full (bad sign) or the order is complete
		// go on to the next order structure
		if (first_buffer_position == -1 ) {
			// This trick is needed to acknowledge a full buffer
			first_buffer_position = current_buffer_position;
		}
		current_buffer_position = (current_buffer_position + 1) % PARSER_ORDER_BUFFER_SIZE;
		current_order_position = 0;
	}
}

uint8_t parser_has_new_order() {
	// Simple Query funtion
	// return 1 if there is a new order waiting and 0 if there isn't
	if (first_buffer_position != -1) {
		return 1;
	}
	return 0;
}

/**
 * Checks whether the order is valid and/or a priority order.
 *
 * It will set the status accordingly.
 * @param[in,out] order The order to be checked.
 */
void parser_check_order(order_t* order) {
	int8_t cmd = -1;
	order->status |= ORDER_STATUS_VALID;
	cmd = order->data[0] & 0x0f;
	if (cmd - ORDER_TYPE_CONTROL  == 0 ||
		cmd - ORDER_TYPE_REGISTER == 0)
		order->status |= ORDER_STATUS_PRIORITY;
}

/**
 * Fills the given order with the data of the next buffered order.
 *
 * @param[out] order The order structure which should be filled.
 */
void parser_get_new_order(order_t* order) {
	// Function fills the parameter with the next order in the ringbuffer
	// but will do a check on it first. if the Order isn't flagged valid
	// the caller must discard it.
	parser_check_order(&parser_order_buffer[first_buffer_position]);
	order_copy(&parser_order_buffer[first_buffer_position], order);
	first_buffer_position = (first_buffer_position + 1) % PARSER_ORDER_BUFFER_SIZE;
	if( first_buffer_position == current_buffer_position ) {
		first_buffer_position = -1;
	}
}
/*@}*/
