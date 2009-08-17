#include "order_functions.h"
#include "order.h"
#include "queue.h"
#include "io.h"
#include "irq.h"
#include "drive.h"
#include "debug.h"
#include "definitions.h"
#include "motor.h"
#include "lcd_addition.h"
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

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

/**
 * External reference to the local time flags.
 */
extern uint8_t local_time_flags;
/**
 * External reference to the time trigger variables.
 */
extern uint16_t timer_t_trigger_counter[2];
/**
 * External reference to the position trigger variables.
 */
extern int16_t irq_p_trigger_position[2];
/**
 * External reference to the running time in seconds.
 */
extern uint16_t timer_1s_counter;

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
	if (DEBUG_ENABLE)
		debug_WriteInteger(PSTR("order_functions.c : control_instruction()\n"), instruction);
	switch (instruction) {
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
		case 0x60: // Reset running time timer
			timer_1s_counter = 0;
			break;
	}
	queue_clear_priority();
	drive_brake_active_set();
}

/**
 * Register query instruction handler function.
 *
 * Used to ask the system from the outside about interesting
 * information, like the current order or the current speed.
 * @param[in,out] order The order that specifies which register
 * (not real register) should be returned.
 */
void query_instruction(order_t *order) {
	int instruction = (order->data[0] & 0xf0);
	order_t *current_order = 0;
	uint8_t current_order_size = 0;
	if (DEBUG_ENABLE)
		debug_WriteString_P(PSTR("order_functions.c : query_instruction()\n"));
	switch (instruction) {
		case 0x10: // left wheel Speed
			io_obj_start();
			io_put(motor_ReadSpeed(WHEEL_LEFT));
			io_obj_end();
			break;
		case 0x20: // right wheel Speed
			io_obj_start();
			io_put(motor_ReadSpeed(WHEEL_RIGHT));
			io_obj_end();
			break;
		case 0x30: // number of Orders in the Queue
			io_obj_start();
			io_put(queue_order_available() - 1); // -1 because Query will be counted
			io_obj_end();
			break;
		case 0x40: // current Order
			if ((current_order = queue_get_current_normal_order())) {
				current_order_size = order_size(current_order);
			}
			io_obj_start();
			io_put(current_order_size);
			io_obj_end();
			if (DEBUG_ENABLE)
				debug_WriteInteger(PSTR("order_functions.c : query_instruction() : current_order_size = "), current_order_size);
			if (current_order) {
				uint8_t i = 0;
				io_obj_start();
				for (;i < current_order_size;i++) {
					io_put(current_order->data[i]);
					if (DEBUG_ENABLE)
						debug_WriteInteger(PSTR("order_functions.c : query_instruction() : put = "), current_order->data[i]);
				}
				io_obj_end();
			}
			break;
		case 0x50: // left wheel time trigger value
			io_obj_start();
			io_put(timer_t_trigger_counter[WHEEL_LEFT] >> 8);
			io_put(timer_t_trigger_counter[WHEEL_LEFT] & 0x00ff);
			io_obj_end();
			break;
		case 0x60: // left wheel position trigger value
			io_obj_start();
			io_put(irq_p_trigger_position[WHEEL_LEFT] >> 8);
			io_put(irq_p_trigger_position[WHEEL_LEFT] & 0x00ff);
			io_obj_end();
			break;
		case 0x70: // right wheel time trigger value
			io_obj_start();
			io_put(timer_t_trigger_counter[WHEEL_RIGHT] >> 8);
			io_put(timer_t_trigger_counter[WHEEL_RIGHT] & 0x00ff);
			io_obj_end();
			break;
		case 0x80: // right wheel position trigger value
			io_obj_start();
			io_put(irq_p_trigger_position[WHEEL_RIGHT] >> 8);
			io_put(irq_p_trigger_position[WHEEL_RIGHT] & 0x00ff);
			io_obj_end();
			break;
		case 0x90: // running time
			io_obj_start();
			io_put(timer_1s_counter >> 8);
			io_put(timer_1s_counter & 0x00ff);
			io_obj_end();
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
 * be set. Valid values are 0x10 or 0x40 for time trigger and 
 * 0x20 or 0x80 for position trigger.
 * @param[in] wheel For which wheel the trigger should be set.
 * Valid values are #WHEEL_LEFT and #WHEEL_RIGHT.
 * @param[in] trigger_value The raw value to which the trigger
 * will be set.
 */
void setTrigger(uint8_t trigger_type, uint8_t wheel, int16_t trigger_value) {
	switch(trigger_type) {
		case 0x10:
		case 0x40:
			// Time trigger
			timer_t_trigger_counter[wheel] = trigger_value;
			break;
		case 0x20:
		case 0x80:
			// Position trigger
			irq_p_trigger_position[wheel] = trigger_value;
			break;
		default:
			// No or invalid trigger
			break;
	}
}

/**
 * Sets the trigger for the advanced_drive_instruction.
 *
 * @param[in] wheel For which wheel the trigger should be set.
 * Valid values are #WHEEL_LEFT and #WHEEL_RIGHT.
 * @param[in] trigger_value_time The value for the time trigger.
 * @param[in] trigger_value_position The value for the position trigger.
 */
void setAdvancedTrigger(uint8_t wheel, int16_t trigger_value_time, int16_t trigger_value_position) {
	timer_t_trigger_counter[wheel] = trigger_value_time;
	irq_p_trigger_position[wheel] = trigger_value_position;
}

/**
 * Checks the trigger on the given wheel.
 *
 * The trigger type of the wheel has to be known.
 * @param[in] trigger_type 0x10 or 0x40 for time trigger and 
 * 0x20 or 0x80 for position trigger.
 * @param[in] wheel The wheel which trigger we request. Valid
 * values are #WHEEL_LEFT and #WHEEL_RIGHT.
 * @return <em>uint16_t</em> Zero if trigger is reached.
 */
uint16_t checkTrigger(uint8_t trigger_type, uint8_t wheel) {
	switch(trigger_type) {
		case 0x10:
		case 0x40:
			// Time trigger
			return timer_t_trigger_counter[wheel];
		case 0x20:
		case 0x80:
			// Position trigger
			return irq_p_trigger_position[wheel];
		default:
			return 1;
	}
}

/**
 * Checks the advanced trigger conditions for a given wheel.
 *
 * The trigger type of the wheel has to be known.
 * @param[in] trigger_type 0x10 or 0x40 for time OR position
 * trigger and 0x20 or 0x80 for time AND position trigger.
 * @param[in] wheel The wheel which trigger we request. Valid
 * values are #WHEEL_LEFT and #WHEEL_RIGHT.
 * @return <em>uint16_t</em> Zero if trigger is reached.
 */
uint16_t checkAdvancedTrigger(uint8_t trigger_type, uint8_t wheel) {
	switch(trigger_type) {
		case 0x10:// (L: ZvP R: N  )
		case 0x40:// (L: N   R: ZVP)
			// Time trigger
			return (!timer_t_trigger_counter[wheel] || !irq_p_trigger_position[wheel]) ? 0 : 1;
		case 0x20:// (L: Z^P R: N  )
		case 0x80:// (L: N   R: Z^P)
			// Position trigger
			return (!timer_t_trigger_counter[wheel] && !irq_p_trigger_position[wheel]) ? 0 : 1;
		default:
			return 1;
	}
}

/**
 * Drive instruction handler function.
 *
 * Very long but versitile function used to drive around the world
 * (maybe not in 80 days).
 * @param[in,out] order The order which specifies what exactly
 * should be done.
 * @todo The function is partially really ugly. Refactor it.
 */
void drive_instruction(order_t *order) {
	// Extract the type of triggers to be used
	uint8_t trigger_type_left = order->data[0] & 0x30;
	uint8_t trigger_type_right = order->data[0] & 0xc0;
	extern uint8_t ACTIVE_BRAKE_WHEN_TRIGGER_REACHED;
	if (DEBUG_ENABLE) {
		debug_WriteInteger(PSTR("order_functions.c : drive_instruction() :  trigger_type_left = "), trigger_type_left);
		debug_WriteInteger(PSTR("order_functions.c : drive_instruction() :  trigger_type_right = "), trigger_type_right);
	}

	if (order->status & ORDER_STATUS_DONE) {
		if (DEBUG_ENABLE)
			debug_WriteString_P(PSTR("order_functions.c : drive_instruction() :  status = ORDER_STATUS_DONE\n"));
		return;
	}

	if (order->status & ORDER_STATUS_STARTED) { //Instruction is already running
		uint16_t result = 0;
		if (DEBUG_ENABLE)
			debug_WriteString_P(PSTR("order_functions.c : drive_instruction() :  status = ORDER_STATUS_STARTED\n"));
		// Left trigger reached
		result = checkTrigger(trigger_type_left, WHEEL_LEFT);
		if (DEBUG_ENABLE)
			debug_WriteInteger(PSTR("order_functions.c : drive_instruction() :  checkTrigger(LEFT) = "), result);
		if (!(order->status & ORDER_STATUS_TRIGGER_LEFT_REACHED) &&
				checkTrigger(trigger_type_left, WHEEL_LEFT) == 0) {
			order->status |= ORDER_STATUS_TRIGGER_LEFT_REACHED;
			drive_brake_active_set_left();
			if (DEBUG_ENABLE)
				debug_WriteString_P(PSTR("order_functions.c : drive_instruction() :  status = ORDER_STATUS_TRIGGER_LEFT_REACHED\n"));
		}
		// Right trigger reached
		result = checkTrigger(trigger_type_right, WHEEL_RIGHT);
		if (DEBUG_ENABLE)
			debug_WriteInteger(PSTR("order_functions.c : drive_instruction() :  checkTrigger(RIGHT) = "), result);
		if (!(order->status & ORDER_STATUS_TRIGGER_RIGHT_REACHED) &&
				checkTrigger(trigger_type_right, WHEEL_RIGHT) == 0 ) {
			order->status |= ORDER_STATUS_TRIGGER_RIGHT_REACHED;
			drive_brake_active_set_right();
			if (DEBUG_ENABLE)
				debug_WriteString_P(PSTR("order_functions.c : drive_instruction() :  status = ORDER_STATUS_TRIGGER_RIGHT_REACHED\n"));
		}
		// If in PID with D mode one reached trigger equals all trigger reached
		if ((order->status & ORDER_STATUS_TRIGGER_RIGHT_REACHED ||
			order->status & ORDER_STATUS_TRIGGER_LEFT_REACHED) &&
			trigger_type_left == 0x30) {
			order->status |= ORDER_STATUS_TRIGGER_LEFT_REACHED + ORDER_STATUS_TRIGGER_RIGHT_REACHED;
		}
		// both triggers reached, ergo order is done 
		if (order->status & ORDER_STATUS_TRIGGER_RIGHT_REACHED && 
			order->status & ORDER_STATUS_TRIGGER_LEFT_REACHED) {
			if (DEBUG_ENABLE)
				debug_WriteString_P(PSTR("order_functions.c : drive_instruction() :  All trigger reached\n"));
			order->status |= ORDER_STATUS_DONE;	
			// stop the wheels
			drive_UsePID(WHEEL_BOTH, 0);
			wheel_ClearDifference();
		}
	
		// When it is wanted, we will use active breaking on the wheel where the trigger has been reached
		if (ACTIVE_BRAKE_WHEN_TRIGGER_REACHED) {
			if (order->status & ORDER_STATUS_TRIGGER_LEFT_REACHED)
				drive_brake_active_left();
			else if (order->status & ORDER_STATUS_TRIGGER_RIGHT_REACHED)
				drive_brake_active_right();
		}
		// Use error-correction while driving every 100 ms
		if (local_time_flags & TIMER_100MS) {
			if (trigger_type_left == 0x30) { // Use function for both wheels to use PID mode with D
				drive_UsePID(WHEEL_BOTH, order->data[1]);
			} else {
				if (!(order->status & ORDER_STATUS_TRIGGER_LEFT_REACHED)) {
					drive_UsePID(WHEEL_LEFT, order->data[1]);
				}
				if (!(order->status & ORDER_STATUS_TRIGGER_RIGHT_REACHED)) {
					drive_UsePID(WHEEL_RIGHT, order->data[2]);
				}
			}
		}
	} else { //Instruction isn't running
		// Build the values for the specified triggers, even if there aren't any triggers present
		int16_t trigger_value_left = 0;
		int16_t trigger_value_right = 0;
		if (DEBUG_ENABLE) {
			debug_WriteString_P(PSTR("order_functions.c : drive_instruction() :  Start execution\n"));
			debug_WriteInteger(PSTR("order_functions.c : drive_instruction() : speed_left = "), order->data[1]);
			debug_WriteInteger(PSTR("order_functions.c : drive_instruction() : speed_right = "), order->data[2]);
		}

		trigger_value_left = ((order->data[3] << 8) + order->data[4]);
		trigger_value_right = ((order->data[5] << 8) + order->data[6]);

		if (trigger_type_right == 0xc0) { // Set new differential correction
			trigger_value_left = (order->data[1] << 8) + order->data[2];
			wheel_WriteDifference(trigger_value_left);
			order->status |= ORDER_STATUS_DONE;
			return;
		}
		if (trigger_type_left == 0x30) { // PID with differential correction
			trigger_value_left = trigger_value_right = ((order->data[2] << 8) + order->data[3]);
		}
		// Set the triggers for each wheel
		if (DEBUG_ENABLE) {
			debug_WriteInteger(PSTR("order_functions.c : drive_instruction() :  trigger_value_left = "), trigger_value_left);
			debug_WriteInteger(PSTR("order_functions.c : drive_instruction() :  trigger_value_right = "), trigger_value_right);
		}
		setTrigger(trigger_type_left, WHEEL_LEFT, trigger_value_left);
		setTrigger(trigger_type_right, WHEEL_RIGHT, trigger_value_right);
		order->status |= ORDER_STATUS_STARTED;
	}
}

/**
 * The Handler function for the advanced drive order.
 *
 * @param[in,out] order The order which specifies what exactly
 * should be done.
 * @todo The function is partially really ugly. Refactor it.
 * @todo The function shared many things with the drive_instruction,
 * maybe a common function base for both is appropriate.
 */
void advanced_drive_instruction(order_t *order) {
	uint8_t trigger_type_left = order->data[0] & 0x30;
	uint8_t trigger_type_right = order->data[0] & 0xc0;
	extern uint8_t ACTIVE_BRAKE_WHEN_TRIGGER_REACHED;

	if (order->status & ORDER_STATUS_DONE)
		return;

	if (order->status & ORDER_STATUS_STARTED) { //Instruction is already running
		// Left trigger reached
		if (!(order->status & ORDER_STATUS_TRIGGER_LEFT_REACHED) && 
				checkAdvancedTrigger(trigger_type_left, WHEEL_LEFT) == 0) {
			order->status |= ORDER_STATUS_TRIGGER_LEFT_REACHED;
			drive_brake_active_set_left();
		}
		// Right trigger reached
		if (!(order->status & ORDER_STATUS_TRIGGER_RIGHT_REACHED) &&
				checkAdvancedTrigger(trigger_type_right, WHEEL_RIGHT) == 0 ) {
			order->status |= ORDER_STATUS_TRIGGER_RIGHT_REACHED;
			drive_brake_active_set_right();
		}
		// both triggers reached, ergo order is done 
		if (order->status & ORDER_STATUS_TRIGGER_RIGHT_REACHED && 
			order->status & ORDER_STATUS_TRIGGER_LEFT_REACHED) {
			order->status |= ORDER_STATUS_DONE;	
			// stop the wheels
			drive_UsePID(WHEEL_BOTH, 0);
			wheel_ClearDifference();
		}

		if (ACTIVE_BRAKE_WHEN_TRIGGER_REACHED) {
			if (order->status & ORDER_STATUS_TRIGGER_LEFT_REACHED)
				drive_brake_active_left();
			else if (order->status & ORDER_STATUS_TRIGGER_RIGHT_REACHED)
				drive_brake_active_right();
		}
		// Use error-correction while driving every 100 ms
		if (local_time_flags & TIMER_100MS) {
			if (!(order->status & ORDER_STATUS_TRIGGER_LEFT_REACHED)) {
				drive_UsePID(WHEEL_LEFT, order->data[1]);
			}
			if (!(order->status & ORDER_STATUS_TRIGGER_RIGHT_REACHED)) {
				drive_UsePID(WHEEL_RIGHT, order->data[2]);
			}
		}
	} else {
		int16_t trigger_value_left_time = 0;
		int16_t trigger_value_left_position = 0;
		int16_t trigger_value_right_time = 0;
		int16_t trigger_value_right_position = 0;
		if (DEBUG_ENABLE)
			debug_WriteString_P(PSTR("order_functions.c : advanced_drive_instruction() :  Start execution\n"));

		trigger_value_left_time = ((order->data[3] << 8) + order->data[4]);
		trigger_value_left_position = ((order->data[5] << 8) + order->data[6]);
		trigger_value_right_time = ((order->data[7] << 8) + order->data[8]);
		trigger_value_right_position = ((order->data[9] << 8) + order->data[10]);

		// Set the triggers for each wheel
		setAdvancedTrigger(WHEEL_LEFT, trigger_value_left_time, trigger_value_left_position);
		setAdvancedTrigger(WHEEL_RIGHT, trigger_value_right_time, trigger_value_right_position);
		order->status |= ORDER_STATUS_STARTED;
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
	if (DEBUG_ENABLE)
		debug_WriteString_P(PSTR("order_functions.c : set_pid_instruction() :  Start execution\n"));

	if(order->status & ORDER_STATUS_DONE)
		return;

	P = ((order->data[1] << 8) + order->data[2]);
	I = ((order->data[3] << 8) + order->data[4]);
	D = ((order->data[5] << 8) + order->data[6]);
	S = ((order->data[7] << 8) + order->data[8]);
	if (DEBUG_ENABLE) {
		debug_WriteInteger(PSTR("order_functions.c : drive_instruction() :  wheel = "), wheel);
		debug_WriteInteger(PSTR("order_functions.c : drive_instruction() :  P = "), P);
		debug_WriteInteger(PSTR("order_functions.c : drive_instruction() :  I = "), I);
		debug_WriteInteger(PSTR("order_functions.c : drive_instruction() :  D = "), D);
		debug_WriteInteger(PSTR("order_functions.c : drive_instruction() :  S = "), S);
	}
	drive_SetPIDParameter(wheel, P, I, D, S);
	order->status |= ORDER_STATUS_DONE;
}

/**
 * Option instruction handler function.
 *
 * Used to set behaviour variables, like how active braking should be done.
 * @param[in,out] order The order which contains the new parameters.
 * @todo maybe make it possible to only set one parameter at a time.
 */
void option_instruction(order_t *order) {
	extern uint8_t ACTIVE_BRAKE_AMOUNT;
	extern uint8_t ACTIVE_BRAKE_ENABLE;
	extern uint8_t ACTIVE_BRAKE_WHEN_IDLE;
	extern uint8_t ACTIVE_BRAKE_WHEN_TRIGGER_REACHED;
	extern uint8_t LCD_PRESENT;
	if (DEBUG_ENABLE) {
		debug_WriteString_P(PSTR("order_functions.c : option_instruction() :  Start execution\n"));
		debug_WriteInteger(PSTR("order_functions.c : option_instruction() : action = "), order->data[0] & 0xf0);
	}
	switch(order->data[0] & 0xf0) {
		case 0x00:
			//reserved
			break;
		case 0x10:
			if(order->data[1] > 127)
				ACTIVE_BRAKE_AMOUNT = ACTIVE_BRAKE_AMOUNT_DEFAULT;
			else
				ACTIVE_BRAKE_AMOUNT = order->data[1];
			break;
		case 0x20:
				ACTIVE_BRAKE_ENABLE = order->data[1];
			break;
		case 0x30:
				ACTIVE_BRAKE_WHEN_TRIGGER_REACHED = order->data[1];
			break;
		case 0x40:
				ACTIVE_BRAKE_WHEN_IDLE = order->data[1];
			break;
	}
	if (LCD_PRESENT)
		lcd_update_info(NULL);
	order->status |= ORDER_STATUS_DONE;
}
/*@}*/
