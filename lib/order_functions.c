#include "order_functions.h"
#include "order.h"
#include "definitions.h"

/**
* This file contains the handler functions for the protokoll.
*/

extern uint8_t local_time_flags;
extern uint16_t timer_t_trigger_counter[2];
extern uint16_t irq_p_trigger_position[2];
#define LEFT_WHEEL 0
#define RIGHT_WHEEL 1
#define BOTH_WHEELS 2

/**
* Extended Instruction Format Handler Function
*/
void extended_instruction(order_t *order) {
	order->status |= ORDER_STATUS_DONE;
}

void control_instruction(order_t *order) {
	// Extract the specific Control Instruction we should carry out
	int instruction = order->data[0] & 0xf0;
	switch(instruction) {
		case 0x10: // Reset Instruction
			WDRF = 1; // Set reset flag in MCUSR TODO: Verify if it works that way
			break;
		case 0x20: // Stop Queue execution and abaddon current order
			queue_pause();
			queue_pop();
			break;
		case 0x30: // Continue Queue execution
			queue_unpause();
			break;
		case 0x40: // Clear the Queue
			queue_clear();
			break;
		case 0x50: // Stop current Order and go over to active braking
			queue_pause();
			break;
	}
	queue_clear_control();
}

void register_instruction(order_t *order) {
	int instruction = order[0] & 0xf0;
	switch(instruction) {
		case 0x10: // left wheel Speed
			break;
		case 0x20: // right wheel Speed
			break;
		case 0x30: // number of Orders in the Queue
			break;
		case 0x40: // current Order
			break;
	}
	queue_clear_register();
}

void setTrigger(uint8_t trigger_type, uint8_t wheel, int16_t trigger_value) {
	//FIXME Just simply wrong, don't set triggers through global variables
	switch(trigger_type) {
		case 1:
			// Time trigger
			timer_t_trigger_counter[wheel] = trigger_value;
			break;
		case 2:
			// Position trigger
			irq_p_trigger_position[wheel] = trigger_value;
			break;
		default:
			// No or invalid trigger
			break;
	}
}

int checkTrigger(uint8_t trigger_type, uint8_t wheel) {
	switch(trigger_type) {
		case 1:
			// Time trigger
			return timer_t_trigger_counter[wheel];
		case 2:
			// Position trigger
			return irq_p_trigger_position[wheel];
		default:
			return 0;
	}
}

void drive_instruction(order_t *order) {
	if(order->status & ORDER_STATUS_DONE)
		return;

	// Extract the type of triggers to be used
	uint8_t trigger_type_left = order->data[0] & 0x30;
	uint8_t trigger_type_right = order->data[0] & 0xa0;

	if(order->status & ORDER_STATUS_STARTED) { //Instruction is already running
		// Left trigger reached
		if(checkTrigger(trigger_type_left, LEFT_WHEEL) == 0) {
			order->status |= ORDER_STATUS_TRIGGER_LEFT_REACHED;
		}
		// Right trigger reached
		if(checkTrigger(trigger_type_right, RIGHT_WHEEL) == 0 ) {
			order->status |= ORDER_STATUS_TRIGGER_RIGHT_REACHED;
		}
		// both triggers reached, ergo order is done 
		if(order->status & ORDER_STATUS_TRIGGER_RIGHT_REACHED && 
			order->status & ORDER_STATUS_TRIGGER_LEFT_REACHED) {
			order->status |= ORDER_STATUS_DONE;	
			// stop the wheels
			drive_UsePID(BOTH_WHEELS, 0);
		}
	
		// Use error-correction while driving every 100 ms
		if(flag_local_read( &local_time_flags, TIMER_100MS)) {
			drive_UsePID(LEFT_WHEEL, order->data[1]);
			drive_UsePID(RIGHT_WHEEL, order->data[2]);
		}
	} else { //Instruction isn't running
		// Build the values for the specified triggers, even if there aren't any triggers present
		// FIXME This is ugly, fix it
		int16_t trigger_value_left = 0;
		int16_t trigger_value_right = 0;
		if(trigger_type_left & 0x10) //Time trigger
			trigger_value_left = order->data[3];
		else if(trigger_type_left & 0x20) //Position trigger
			trigger_value_left = order->data[3] << 8 + order->data[4];
		if(trigger_type_right & 0x40) //Time trigger
			trigger_value_right = order->data[4];
		else if(trigger_type_right & 0x80) //Position trigger
			trigger_value_right = order->data[5] << 8 + order->data[6];
		// Set the triggers for each wheel
		setTrigger(trigger_type_left, LEFT_WHEEL, trigger_value_left);
		setTrigger(trigger_type_right, RIGHT_WHEEL, trigger_value_right);
	}
}

void pid_drive_instruction(order_t *order) {}

void set_pid_instruction(order_t *order) {}
