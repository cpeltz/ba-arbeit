#include <inttypes.h>
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
static pid_data_t drive_PID[NUMBER_OF_WHEELS];

/**
 * External reference to the Position Variable for the wheels.
 */
extern int16_t irq_Position[NUMBER_OF_WHEELS];

/**
 *	Stores the position of the wheels they should hold during active braking.
 */
int16_t drive_brake_position[NUMBER_OF_WHEELS];

/**
 * Sets the PID-Parameter for either one wheel or both wheels.
 *
 * @param[in] wheel Possible values include #WHEEL_LEFT, #WHEEL_RIGHT and #WHEEL_BOTH.
 * @param[in] Pfactor the proportional Factor of the PID-System
 * @param[in] Ifactor the integral Part of the PID-System
 * @param[in] Dfactor the differential factor
 * @param[in] SErrorMAX the max value for the sum of all errors
 */
void drive_SetPIDParameter(	const uint8_t wheel, const int16_t Pfactor, const int16_t Ifactor,
							const int16_t Dfactor, const int16_t SErrorMAX) {
	// Sets the parameter for a PID enabled movement
	if (DEBUG_ENABLE)
		debug_WriteString_P(PSTR("drive.c : drive_SetPID_Parameter()\n"));
	switch (wheel) {
		case WHEEL_LEFT:
		case WHEEL_RIGHT:
			pid_Init(Pfactor, Ifactor, Dfactor, SErrorMAX, &drive_PID[wheel]);
			break;
		case WHEEL_BOTH:
			pid_Init(Pfactor, Ifactor, Dfactor, SErrorMAX, &drive_PID[WHEEL_LEFT]);
			pid_Init(Pfactor, Ifactor, Dfactor, SErrorMAX, &drive_PID[WHEEL_RIGHT]);
			break;
    }
}

/**
 *	Sets the max value for the sum off all errors.
 *
 *	Beware that this function may be obsolete.
 *	@param[in] wheel Possible values include #WHEEL_LEFT, #WHEEL_RIGHT and #WHEEL_BOTH.
 *	@param[in] sumError the max value for the sum of all errors
 */

void drive_SetPIDSumError(const uint8_t wheel, const int16_t sumError) {
	// Used to set the SumError for a PID enabled movement
	if (DEBUG_ENABLE)
		debug_WriteString_P(PSTR("drive.c : drive_SetPIDSumError()\n"));
	switch (wheel) {
		case WHEEL_LEFT:
		case WHEEL_RIGHT:
			drive_PID[wheel].sumError = sumError;
			break;
		case WHEEL_BOTH:
			drive_PID[WHEEL_LEFT ].sumError = sumError;
			drive_PID[WHEEL_RIGHT].sumError = sumError;
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
void drive_UsePID(const uint8_t wheel, const int8_t speed) {
	int16_t wheel_ModSpeed[2];
	int16_t wheel_Difference;

	if (DEBUG_ENABLE)
		debug_WriteString_P(PSTR("drive.c : drive_UsePID()\n"));
	switch (wheel) {
		case WHEEL_LEFT:
		case WHEEL_RIGHT:
			// PID Fahrt für linkes Rad
			// PID Fahrt für rechtes Rad
			if (speed > 0)
				motor_SetSpeed(wheel, pid_Controller(speed, wheel_ReadSpeed(wheel), &drive_PID[wheel]));
			else
				motor_SetSpeed(wheel, -pid_Controller(-speed, -wheel_ReadSpeed(wheel), &drive_PID[wheel]));
			break;
		case WHEEL_BOTH:
			// PID Fahrt für beide Räder mit Differenzausgleich
			wheel_ModSpeed[WHEEL_LEFT ] = wheel_ReadSpeed(WHEEL_LEFT);
			wheel_ModSpeed[WHEEL_RIGHT] = wheel_ReadSpeed(WHEEL_RIGHT);
			wheel_Difference = wheel_ReadDifference();

			// bei ungerader Differenz immer abwechselnd
			if (wheel_Difference % 2) {
				wheel_ModSpeed[WHEEL_LEFT] += wheel_Difference;
			} else {
				wheel_ModSpeed[WHEEL_RIGHT] -= wheel_Difference;
			}

			// geraden Differenzanteil gerecht verteilen
			wheel_ModSpeed[WHEEL_LEFT] += (wheel_Difference / 2);
			wheel_ModSpeed[WHEEL_RIGHT] -= (wheel_Difference / 2);

			// Richtungsunterschied da PID Regler nur positiv arbeitet
			if (speed > 0) {
				// Veränderte Werte begrenzen
				if (wheel_ModSpeed[WHEEL_LEFT] < 0)
					wheel_ModSpeed[WHEEL_LEFT] = 0;
				else if (wheel_ModSpeed[WHEEL_LEFT] > 127)
					wheel_ModSpeed[WHEEL_LEFT] = 127;
				if (wheel_ModSpeed[WHEEL_RIGHT] < 0)
					wheel_ModSpeed[WHEEL_RIGHT] = 0;
				else if (wheel_ModSpeed[WHEEL_RIGHT] > 127)
					wheel_ModSpeed[WHEEL_RIGHT] = 127;

				motor_SetSpeed(WHEEL_LEFT, pid_Controller(speed, wheel_ModSpeed[WHEEL_LEFT], &drive_PID[WHEEL_LEFT]));
				motor_SetSpeed(WHEEL_RIGHT, pid_Controller(speed, wheel_ModSpeed[WHEEL_RIGHT], &drive_PID[WHEEL_RIGHT]));
			} else {
				// Veränderte Werte begrenzen
				if (wheel_ModSpeed[WHEEL_LEFT] < -127)
					wheel_ModSpeed[WHEEL_LEFT] = -127;
				else if (wheel_ModSpeed[WHEEL_LEFT] > 0)
					wheel_ModSpeed[WHEEL_LEFT] = 0;
				if (wheel_ModSpeed[WHEEL_RIGHT] < -127)
					wheel_ModSpeed[WHEEL_RIGHT] = -127;
				else if (wheel_ModSpeed[WHEEL_RIGHT] > 0)
					wheel_ModSpeed[WHEEL_RIGHT] = 0;

				motor_SetSpeed(WHEEL_LEFT, -pid_Controller(-speed, -wheel_ModSpeed[WHEEL_LEFT], &drive_PID[WHEEL_LEFT]));
				motor_SetSpeed(WHEEL_RIGHT, -pid_Controller(-speed, -wheel_ModSpeed[WHEEL_RIGHT], &drive_PID[WHEEL_RIGHT]));
				}
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
	if (drive_brake_position[WHEEL_LEFT] == irq_Position[WHEEL_LEFT]) {
		motor_SetSpeed(WHEEL_LEFT, 0);
	} else if (drive_brake_position[WHEEL_LEFT] < irq_Position[WHEEL_LEFT]) {
		motor_SetSpeed(WHEEL_LEFT, -(ACTIVE_BRAKE_AMOUNT));
	} else {
		motor_SetSpeed(WHEEL_LEFT, ACTIVE_BRAKE_AMOUNT);
	}
	if (drive_brake_position[WHEEL_RIGHT] == irq_Position[WHEEL_RIGHT]) {
		motor_SetSpeed(WHEEL_RIGHT, 0);
	} else if (drive_brake_position[WHEEL_RIGHT] < irq_Position[WHEEL_RIGHT]) {
		motor_SetSpeed(WHEEL_RIGHT, -(ACTIVE_BRAKE_AMOUNT));
	} else {
		motor_SetSpeed(WHEEL_RIGHT, ACTIVE_BRAKE_AMOUNT);
	}
}

/**
 * Sets the current position as holding position used while doing active braking.
 *
 * Sets position for both wheels.
 */
void drive_brake_active_set(void) {
	// Set the Position of the wheels for use with active braking.
	drive_brake_active_set_left();
	drive_brake_active_set_right();
}

/**
 * Handles active braking for the left wheel.
 *
 * Will only work if ACTIVE_BRAKE_ENABLE not 0.
 */
void drive_brake_active_left(void) {
	extern uint8_t ACTIVE_BRAKE_ENABLE;
	extern uint8_t ACTIVE_BRAKE_AMOUNT;
	if(!ACTIVE_BRAKE_ENABLE)
		return;
	if (drive_brake_position[WHEEL_LEFT] == irq_Position[WHEEL_LEFT]) {
		motor_SetSpeed(WHEEL_LEFT, 0);
	} else if (drive_brake_position[WHEEL_LEFT] < irq_Position[WHEEL_LEFT]) {
		motor_SetSpeed(WHEEL_LEFT, -(ACTIVE_BRAKE_AMOUNT));
	} else {
		motor_SetSpeed(WHEEL_LEFT, ACTIVE_BRAKE_AMOUNT);
	}
}

/**
 * Sets the position for active braking for the left wheel.
 */
void drive_brake_active_set_left(void) {
	drive_brake_position[WHEEL_LEFT] = irq_Position[WHEEL_LEFT];
}

#include <string.h>

/**
 * Testfunction to set the position for a specified wheel.
 * @todo test this out against the other three functions.
 */
/*void drive_brake_active_set(uint8_t wheel) {
	switch(wheel) {
		case WHEEL_BOTH:
			memcpy(drive_brake_position, irq_Position, WHEEL_BOTH);
		default:
			drive_brake_position[wheel] = irq_Position[wheel];
			break;
	}
}*/

/**
 * Handles active braking for the right wheel.
 *
 * Will only work if ACTIVE_BRAKE_ENABLE not 0.
 */
void drive_brake_active_right(void) {
	extern uint8_t ACTIVE_BRAKE_ENABLE;
	extern uint8_t ACTIVE_BRAKE_AMOUNT;
	if(!ACTIVE_BRAKE_ENABLE)
		return;
	if (drive_brake_position[WHEEL_RIGHT] == irq_Position[WHEEL_RIGHT]) {
		motor_SetSpeed(WHEEL_RIGHT, 0);
	} else if (drive_brake_position[WHEEL_RIGHT] < irq_Position[WHEEL_RIGHT]) {
		motor_SetSpeed(WHEEL_RIGHT, -(ACTIVE_BRAKE_AMOUNT));
	} else {
		motor_SetSpeed(WHEEL_RIGHT, ACTIVE_BRAKE_AMOUNT);
	}
}

/*
void drive_brake_active(uint8_t wheel) {
	extern uint8_t ACTIVE_BRAKE_ENABLE;
	extern uint8_t ACTIVE_BRAKE_AMOUNT;
	if(!ACTIVE_BRAKE_ENABLE)
		return;
	
	motor_SetSpeed(wheel,
		(drive_brake_position[wheel] - irq_Position[wheel])
		* ACTIVE_BRAKE_AMOUNT);
}
*/

/**
 * Sets the position for active braking for the right wheel.
 */
void drive_brake_active_set_right(void) {
	drive_brake_position[WHEEL_RIGHT] = irq_Position[WHEEL_RIGHT];
}
/*@}*/
