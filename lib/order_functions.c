#include "order_functions.h"
#include "order.h"
#include "queue.h"
#include "io.h"
#include "drive.h"
#include "flags.h"
#include "definitions.h"
#include <avr/wdt.h>

/**
 * @addtogroup ORDER_Module
 * @{
 */

/**
 * @file
 *
 * Contains the protocol order functions.
 * @todo Write Helper functions to help with repetetive tasks and clarify them
 * @todo need more defines for constants so they are more readable
 * @todo Put references to the Protocol document in here.
 */
 
extern uint8_t local_time_flags;
extern uint16_t timer_t_trigger_counter[2];
extern int16_t irq_p_trigger_position[2];

/**
 * Extended Instruction Format Handler Function.
 *
 * This function currently does nothing besides setting the order
 * status to #ORDER_STATUS_DONE. It can be used later on if all the
 * other order choices have already been taken, but one needs a new
 * protocol command.
 * @param[in,out] order The specific order to be used.
 */
void extended_instruction(order_t *order) {
	order->status |= ORDER_STATUS_DONE;
}

/**
 * The control instruction handler function.
 *
 * The Control instruction is used for a variaty of different tasks
 * like resetting the board or controlling the order queue from the
 * outside.
 * @param[in,out] order The order data that specifies what exactly
 * should be done.
 */
void control_instruction(order_t *order) {
	// Extract the specific Control Instruction we should carry out
	int instruction = order->data[0] & 0xf0;
	switch(instruction) {
		case 0x10: // Reset Instruction
			wdt_reset();
			wdt_enable(4);
			while(1) {} // Wait for the watchdog to reset the board
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

/**
 * Register query instruction handler function.
 *
 * Used to ask the system from the outside about interesting
 * information, like the current order or the current speed.
 * @param{in,out] order The order that specifies which register
 * (not real register) should be returned.
 */
void register_instruction(order_t *order) {
	int instruction = (order->data[0] & 0xf0);
	order_t *current_order = 0;
	uint8_t current_order_size = 0;
	global_state_t state;
	drive_status(&state);
	switch(instruction) {
		case 0x10: // left wheel Speed
				io_obj_start();
				io_put(state.speed_left);
				io_obj_end();
			break;
		case 0x20: // right wheel Speed
				io_obj_start();
				io_put(state.speed_right);
				io_obj_end();
			break;
		case 0x30: // number of Orders in the Queue
			io_obj_start();
			io_put(queue_order_available());
			io_obj_end();
			break;
		case 0x40: // current Order
			if ((current_order = queue_get_current_normal_order())) {
				current_order_size = order_size(current_order) % (ORDER_TYPE_MAX_LENGTH + 1);
			}
			io_obj_start();
			io_put(current_order_size);
			io_obj_end();
			if (current_order) {
				uint8_t i = 0;
				io_obj_start();
				for (;i < current_order_size;i++) {
					io_put(current_order->data[current_order_size]);
				}
				io_obj_end();
			}
			break;
	}
	queue_clear_priority();
}

/**
 * Sets the trigger.
 *
 * To set for any wheel any trigger to a value, this function
 * should be used.
 * @param[in] trigger_type The type of the trigger, which should
 * be set. Valid values are 1 for time trigger and 2 for position
 * trigger.
 * @param[in] wheel For which wheel the trigger should be set.
 * Valid values are #WHEEL_LEFT and #WHEEL_RIGHT.
 * @param[in] trigger_value The raw value to which the trigger
 * will be set.
 * @todo Create defines for the trigger_type parameter
 * @todo Fix the setting of the triggers through global variables
 */
void setTrigger(uint8_t trigger_type, uint8_t wheel, int16_t trigger_value) {
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

/**
 * Checks the trigger on the given wheel.
 *
 * The trigger type of the wheel has to be known.
 * @param[in] trigger_type 1 for time trigger and 2 for position
 * trigger.
 * @param[in] wheel The wheel which trigger we request. Valid
 * values are #WHEEL_LEFT and #WHEEL_RIGHT.
 * @return <em>int</em> Zero if trigger is reached.
 * @todo Make sure this function returns the right values
 * @todo Really used to check if triggers reached? looks more like
 * checking the values of the triggers
 * @todo Make some defines for the trigger_type parameter.
 * @todo Compare with the trigger function in order.c
 */
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

/**
 * Drive instruction handler function.
 *
 * Very long but versitile function used to drive around the world
 * (maybe not in 80 days).
 * @param[in,out] order The order which specifies what exactly
 * should be done.
 * @todo Implement the setting of the differential correction
 * @todo The function is partially really ugly. Refactor it.
 */
void drive_instruction(order_t *order) {
	// Extract the type of triggers to be used
	uint8_t trigger_type_left = order->data[0] & 0x30;
	uint8_t trigger_type_right = order->data[0] & 0xc0;

	if(order->status & ORDER_STATUS_DONE)
		return;

	if(order->status & ORDER_STATUS_STARTED) { //Instruction is already running
		// Left trigger reached
		if(checkTrigger(trigger_type_left, WHEEL_LEFT) == 0) {
			order->status |= ORDER_STATUS_TRIGGER_LEFT_REACHED;
		}
		// Right trigger reached
		if(checkTrigger(trigger_type_right, WHEEL_RIGHT) == 0 ) {
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
			drive_UsePID(WHEEL_BOTH, 0);
		}
	
		// Use error-correction while driving every 100 ms
		if(flag_local_read( &local_time_flags, TIMER_100MS)) {
			if(trigger_type_left == 0x30) { // Use function for both wheels to use PID mode with D
				drive_UsePID(WHEEL_BOTH, order->data[1]);
			} else {
				drive_UsePID(WHEEL_LEFT, order->data[1]);
				drive_UsePID(WHEEL_RIGHT, order->data[2]);
			}
		}
	} else { //Instruction isn't running
		// Build the values for the specified triggers, even if there aren't any triggers present
		// FIXME This is ugly, fix it
		int16_t trigger_value_left = 0;
		int16_t trigger_value_right = 0;

		if (trigger_type_right == 0xc0) { // Set new differential correction
			/* IMPLEMENT */
			order->status |= ORDER_STATUS_DONE;
		}
		if (trigger_type_left == 0x30) { // PID with differential correction
			trigger_value_left = trigger_value_right = ((order->data[2] << 8) + order->data[3]);
		} else {
			trigger_value_left = ((order->data[3] << 8) + order->data[4]);
			trigger_value_right = ((order->data[5] << 8) + order->data[6]);
		}
		// Set the triggers for each wheel
		setTrigger(trigger_type_left, WHEEL_LEFT, trigger_value_left);
		setTrigger(trigger_type_right, WHEEL_RIGHT, trigger_value_right);
	}
}

/**
 * SetPID instruction handler function.
 *
 * Used to set the PID control parameters from the outside.
 * @param[in,out] order The order which contains the new parameters.
 * @todo maybe make it possible to only set one parameter at a time.
 */
void set_pid_instruction(order_t *order) {
	uint8_t wheel = order->data[0] >> 4;
	int16_t P, I, D, S;

	if(order->status & ORDER_STATUS_DONE)
		return;

	P = ((order->data[1] << 8) + order->data[2]);
	I = ((order->data[3] << 8) + order->data[4]);
	D = ((order->data[5] << 8) + order->data[6]);
	S = ((order->data[7] << 8) + order->data[8]);
	drive_SetPIDParameter(wheel, P, I, D, S);
	order->status |= ORDER_STATUS_DONE;
}
/*@}*/
