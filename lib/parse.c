#include "parse.h"
#include "order.h"

order_t parser_order_buffer[5];
int8_t current_buffer_position = 0;
int8_t first_buffer_position = -1;
uint8_t current_order_position = 0;

void parser_init(void) {
	uint8_t i = 0;
	for(; i < 5; i++) {
		order_init(&parser_order_buffer[0]);
	}
}

int parser_extended_order_complete(const order_t* order, uint8_t num_bytes) { //Doesn't really do anything, because there aren't any extended orders at the moment
	return 1;
}

uint8_t bytes_needed(uint8_t order) {
	uint8_t ret_value = 0;
	switch(order & 0x0f) {
		case 1: //Reset Instruction
		case 2: //Register Query Instruction
		case 3: //Queue Query Instruction
		case 4: //Current Order Instruction
			return 1; //These are all one byte Instructions
		case 5: //Drive Instruction is a variable byte order
			ret_value = 3; //min. three bytes are neccessary: one as order, two for speed (left and right)
			if(order & 0x10) { //left Wheel Time Trigger, needs one extra byte
				ret_value += 1;
			} else if(order & 0x20) { //left Wheel Position Trigger, needs two extra bytes
				ret_value += 2;
			}
			if(order & 0x40) { //right Wheel Time Trigger, needs one extra byte
				ret_value += 1;
			} else if(order & 0x80) { //right Wheel Position Trigger, needs two extra bytes
				ret_value += 2;
			}
			return ret_value;
		case 6: //PID Drive Instruction is a variable byte order
			ret_value = 3; //at least three bytes are neccessary
			if(order & 0x10) { //Time Trigger, needs an extra byte
				ret_value += 1;
			} else if(order & 0x20) { //Position Trigger, needs two extra bytes
				ret_value += 2;
			} else if(order & 0x40) { //set differential value, needs two bytes FIXME
				ret_value += 2;
			}
			return ret_value;
		case 7: //Set PID Parameters Instruction
			if(order & 0x10 || order & 0x20) { //Set PID Parameters for the right or/and left wheel
				return 5; //FIXME
			}
			return 3; //FIXME
	}
	return 1;
}

int parser_order_complete(const order_t* order, uint8_t num_bytes) { //TODO Recovery for Instructions which are too long
	if( order->data[0] & 0x0f == 0 )
		return parser_extended_order_complete(order, num_bytes);
	else if( order->data[0] & 0x0f <= 8 ) {
		if(num_bytes >= bytes_needed(order->data[0]))
			return 1;
	}
	return 0;
}

void parser_add_byte(uint8_t byte) { //TODO FIXME Maybe a mutex or a other kind of lock would be good
// TODO Syntax-check has to be in here or right after this
	if( current_buffer_position == first_buffer_position ) {
		return;
	}
	parser_order_buffer[current_buffer_position].data[current_order_position] = byte;
	current_order_position++;
	if(current_order_position >= 32 || parser_order_complete(&parser_order_buffer[current_buffer_position], current_order_position + 1)) {
		if( first_buffer_position == -1 ) {
			first_buffer_position = current_buffer_position;
		}
		current_buffer_position = (current_buffer_position + 1) % 5;
		current_order_position = 0;
	}
}

uint8_t parser_has_new_order() {
	if(first_buffer_position != -1) {
		return 1;
	}
	return 0;
}

// This is the Syntax-check function to determine whether or not the order is valid
// Could get really ugly :/
void parser_check_order(order_t* order) {
	// TODO Implement
	order->status |= ORDER_STATUS_VALID;
	/*
	 *	switch(order->data[0] & 0x0f) {
	 *		case 0: //Extended Instruction Format, no more tests ATM
	 *			break;
	 *		case 1: //Reset Instruction, everything else is ignored
	 *			break;
	 *		case 2: //Register Query Instruction, look for a valid register
	 *			break;
	 *		case 3: //Queue Query Instruction
	 *			break;
	 *
	 *		default:
	 *			order->status -= ORDER_STATUS_VALID;
	 *			break;
	 *	}
	 */
}

void parser_get_new_order(order_t* order) {
	parser_check_order(&parser_order_buffer[first_buffer_position]);
	order_copy(&parser_order_buffer[first_buffer_position], order);
	first_buffer_position = (first_buffer_position + 1) % 5;
	if( first_buffer_position == current_buffer_position ) {
		first_buffer_position = -1;
	}
}
