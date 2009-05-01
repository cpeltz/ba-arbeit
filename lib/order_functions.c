#include "order_functions.h"
#include "order.h"
#include "definitions.h"

/**
* This file contains the handler functions for the protocol.
*/

extern uint8_t local_time_flags;
extern uint16_t timer_t_trigger_counter[2];
extern uint16_t irq_p_trigger_position[2];
#define LEFT_WHEEL 0
#define RIGHT_WHEEL 1
#define BOTH_WHEELS 2

// TODO Write Helper functions to help with repetetive tasks and clarify them
// TODO need more defines for constants so they are more readable

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
	queue_clear_priority();
}

void register_instruction(order_t *order) {
	int instruction = order[0] & 0xf0;
	order_t *current_order = 0;
	uint8_t current_order_size = 0;
	switch(instruction) {
		case 0x10: // left wheel Speed
				//TODO Implement (don't know where the Speed is)
			break;
		case 0x20: // right wheel Speed
				//TODO Implement (don't know where the Speed is)
			break;
		case 0x30: // number of Orders in the Queue
			io_obj_start();
			io_put(queue_order_available());
			io_obj_end();
			break;
		case 0x40: // current Order
			if (current_order = queue_get_current_normal_order()) {
				current_order_size = order_size(current_order) % (ORDER_TYPE_MAX_LENGTH + 1);
			}
			io_obj_start();
			io_put(current_order_size);
			io_obj_end();
			if (current_order) {
				uint8_t i = 0;
				io_obj_start();
				for (;i < current_order_size;i++) {
					io_obj_put(current_order->data[current_order_size]);
				}
				io_obj_end();
			}
			break;
	}
	queue_clear_priority();
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
	// Extract the type of triggers to be used
	uint8_t trigger_type_left = order->data[0] & 0x30;
	uint8_t trigger_type_right = order->data[0] & 0xc0;

	if(order->status & ORDER_STATUS_DONE)
		return;

	if(order->status & ORDER_STATUS_STARTED) { //Instruction is already running
		// Left trigger reached
		if(checkTrigger(trigger_type_left, LEFT_WHEEL) == 0) {
			order->status |= ORDER_STATUS_TRIGGER_LEFT_REACHED;
		}
		// Right trigger reached
		if(checkTrigger(trigger_type_right, RIGHT_WHEEL) == 0 ) {
			order->status |= ORDER_STATUS_TRIGGER_RIGHT_REACHED;
		}
		// If in PID with D mode one reached trigger equals all trigger reached
		if((order->status & ORDER_STATUS_TRIGGER_RIGHT_REACHED ||
			order->status & ORDER_STATUS_TRIGGER_LEFT_REACHED) &&
			trigger_type_left == 0x30) {
			order->status |= ORDER_STATUS_TRIGGER_LEFT_REACHED + ORDER_STATUS_TRIGGER_RIGHT_REACHED;
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
			if(trigger_type_left == 0x30) { // Use function for both wheels to use PID mode with D
				drive_UsePID(BOTH_WHEELS, order->data[1]);
			} else {
				drive_UsePID(LEFT_WHEEL, order->data[1]);
				drive_UsePID(RIGHT_WHEEL, order->data[2]);
			}
		}
	} else { //Instruction isn't running
		// Build the values for the specified triggers, even if there aren't any triggers present
		// FIXME This is ugly, fix it
		int16_t trigger_value_left = 0;
		int16_t trigger_value_right = 0;

		if (trigger_type_right == 0xc0) { // Set new differential correction
			// TODO Implement the setting of the differential correction
			order->status |= ORDER_STATUS_DONE;
		}
		if (trigger_type_left == 0x30) { // PID with differential correction
			trigger_value_left = trigger_value_right = order->data[2] << 8 + order->data[3];
		} else {
			trigger_value_left = order->data[3] << 8 + order->data[4];
			trigger_value_right = order->data[5] << 8 + order->data[6];
		}
		// Set the triggers for each wheel
		setTrigger(trigger_type_left, LEFT_WHEEL, trigger_value_left);
		setTrigger(trigger_type_right, RIGHT_WHEEL, trigger_value_right);
	}
}

void set_pid_instruction(order_t *order) {
	uint8_t wheel = order->data[0] >> 4;
	int16_t P, I, D, S;

	if(order->status & ORDER_STATUS_DONE)
		return;

	P = order->data[1] << 8 + order->data[2];
	I = order->data[3] << 8 + order->data[4];
	D = order->data[5] << 8 + order->data[6];
	S = order->data[7] << 8 + order->data[8];
	drive_SetPIDParameter(wheel, P, I, D, S);
	order->status |= ORDER_STATUS_DONE;
}
