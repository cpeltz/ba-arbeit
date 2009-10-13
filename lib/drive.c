#include <inttypes.h>
#include <string.h>
#include "definitions.h"
#include "options.h"
#include "pid.h"
#include "drive.h"
#include "motor.h"
#include "irq.h"
#include "timer.h" /**< @todo move the wheel function out of this file */
#include "lcd.h"
#include "debug.h"

/**
 * @defgroup DRIVE_Module Drive Module
 * This module provides access to the drive managment
 * and control.
 * @{
 */

/**
 *	Used to remember the PID-Parameters for each wheel
 */
static pid_data_t drive_pid[NUMBER_OF_WHEELS];

/**
 * External reference to the Position Variable for the wheels.
 */
extern int16_t irq_position[NUMBER_OF_WHEELS];

/**
 *	Stores the position of the wheels they should hold during active braking.
 */
int16_t drive_brake_position[NUMBER_OF_WHEELS];

/**
 * Sets the PID-Parameter for either one wheel or both wheels.
 *
 * @param[in] wheel Possible values include #WHEEL_LEFT, #WHEEL_RIGHT and #WHEEL_ALL.
 * @param[in] pfactor the proportional Factor of the PID-System
 * @param[in] ifactor the integral Part of the PID-System
 * @param[in] dfactor the differential factor
 * @param[in] sum_error_max the max value for the sum of all errors
 */
void drive_set_pid_parameter( const uint8_t wheel, const int16_t pfactor, const int16_t ifactor,
                              const int16_t dfactor, const int16_t sum_error_max) {
	// Sets the parameter for a PID enabled movement
	if (DEBUG_ENABLE)
		debug_write_string_p(PSTR("drive.c : drive_SetPID_Parameter()\n"));
	switch (wheel) {
		case WHEEL_ALL:
			for(uint8_t i = 0; i < NUMBER_OF_WHEELS; i++)
				pid_init(pfactor, ifactor, dfactor, sum_error_max, &drive_pid[i]);
			break;
		default:
			pid_init(pfactor, ifactor, dfactor, sum_error_max, &drive_pid[wheel]);
			break;
    }
}

/**
 *	Sets the max value for the sum off all errors.
 *
 *	Beware that this function may be obsolete.
 *	@param[in] wheel Possible values include #WHEEL_LEFT, #WHEEL_RIGHT and #WHEEL_ALL.
 *	@param[in] sum_error the max value for the sum of all errors
 */

void drive_set_pid_sum_error(const uint8_t wheel, const int16_t sum_error) {
	// Used to set the SumError for a PID enabled movement
	if (DEBUG_ENABLE)
		debug_write_string_p(PSTR("drive.c : drive_SetPIDSumError()\n"));
	switch (wheel) {
		case WHEEL_ALL:
			for(uint8_t i = 0; i < NUMBER_OF_WHEELS; i++)
				drive_pid[i].sum_error = sum_error;
		break;
		default:
			drive_pid[wheel].sum_error = sum_error;
			break;
	}
}

/**
 * Low-level PID control function.
 *
 * This function uses the PID-System to control the use of
 * the wheels while driving.
 * @param[in] wheel Possible values include #WHEEL_LEFT, #WHEEL_RIGHT and #WHEEL_BOTH.
 * @param[in] speed Describes the speed with which the wheel should be used.
 * @todo Remove the Modulo-Operators
 */
void drive_use_pid(const uint8_t wheel, const int8_t speed) {
	int16_t wheel_mod_speed[2];
	int16_t wheel_difference;

	if (DEBUG_ENABLE)
		debug_write_string_p(PSTR("drive.c : drive_UsePID()\n"));
	switch (wheel) {
		case WHEEL_BOTH:
			// Drive using PID with difference calibration
			wheel_difference = wheel_read_difference();
			for (uint8_t i = 0; i < NUMBER_OF_WHEELS; i++) {
				// calculate the sign for the mod_speed modification
				uint8_t sign = i - ((i / 2) * 2);
				// get the speed of the wheel
				wheel_mod_speed[i] = wheel_read_speed(i);
				// add or substract a share of the difference
				wheel_mod_speed[i] += (-1 * sign) * (wheel_Difference / NUMBER_OF_WHEELS);
				// limit the speed modifications
				if (speed > 0) {
					if (wheel_mod_speed[i] > 127)
						wheel_mod_speed[i] = 127;
					else if (wheel_mod_speed[i] < 0)
						wheel_mod_speed[i] = 0;
				} else {
					if (wheel_mod_speed[i] < -128)
						wheel_mod_speed[i] = -128;
					else if (wheel_mod_speed[i] > 0)
						wheel_mod_speed[i] = 0;
				}
				// calculate sign for motor_set_speed
				sign = (-1 * (speed =< 0));
				// set the modified speeds, after they were adjusted by the PID-Controller
				motor_set_speed(i, sign * pid_controller(sign * speed, sign * wheel_mod_speed[i], &drive_pid[i]));
			}
			break;
		default:
			uint8_t sign = (-1 * (speed =< 0));
			motor_set_speed(wheel, sign * pid_controller(sign * speed, sign * wheel_read_speed(wheel), &drive_pid[wheel]));
			break;
	}
}

/**
 * Function used to handle active braking of both wheels at once.
 */
void drive_brake_active(void) {
	// Do active braking. Wheels shouldn't move from their position.
	extern uint8_t ACTIVE_BRAKE_ENABLE;
	extern uint8_t ACTIVE_BRAKE_AMOUNT;
	if(!ACTIVE_BRAKE_ENABLE)
		return;
	if (drive_brake_position[WHEEL_LEFT] == irq_position[WHEEL_LEFT]) {
		motor_set_speed(WHEEL_LEFT, 0);
	} else if (drive_brake_position[WHEEL_LEFT] < irq_Position[WHEEL_LEFT]) {
		motor_set_speed(WHEEL_LEFT, -(ACTIVE_BRAKE_AMOUNT));
	} else {
		motor_set_speed(WHEEL_LEFT, ACTIVE_BRAKE_AMOUNT);
	}
	if (drive_brake_position[WHEEL_RIGHT] == irq_Position[WHEEL_RIGHT]) {
		motor_set_speed(WHEEL_RIGHT, 0);
	} else if (drive_brake_position[WHEEL_RIGHT] < irq_Position[WHEEL_RIGHT]) {
		motor_set_speed(WHEEL_RIGHT, -(ACTIVE_BRAKE_AMOUNT));
	} else {
		motor_set_speed(WHEEL_RIGHT, ACTIVE_BRAKE_AMOUNT);
	}
}

/**
 * Testfunction to set the position for a specified wheel.
 * @todo test this out against the other three functions.
 */
void drive_brake_active_set(uint8_t wheel) {
	switch(wheel) {
		case WHEEL_ALL:
			memcpy(drive_brake_position, irq_Position, NUMBER_OF_WHEELS);
		default:
			drive_brake_position[wheel] = irq_Position[wheel];
			break;
	}
}

/**
 * Handles braking for one specified wheel,
 *
 * @param wheel The wheel for which active braking should be done.
 */
void drive_brake_active(uint8_t wheel) {
	extern uint8_t ACTIVE_BRAKE_ENABLE;
	extern uint8_t ACTIVE_BRAKE_AMOUNT;
	if(!ACTIVE_BRAKE_ENABLE)
		return;
	if (drive_brake_position[wheel] == irq_position[wheel]) {
		motor_set_speed(wheel, 0);
	} else if (drive_brake_position[wheel] < irq_position[wheel]) {
		motor_set_speed(wheel, -(ACTIVE_BRAKE_AMOUNT));
	} else {
		motor_set_speed(wheel, ACTIVE_BRAKE_AMOUNT);
	}
}
/*@}*/
