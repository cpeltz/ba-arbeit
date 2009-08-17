#include <inttypes.h>
#include "definitions.h"
#include "options.h"
#include "pid.h"
#include "drive.h"
#include "motor.h"
#include "irq.h"
#include "timer.h" /**< @todo move the wheel function out of this file */
#include "lcd.h"

/**
 * @defgroup DRIVE_Module Drive Module
 * This module provides access to the drive managment
 * and control.
 * @{
 */

/**
 *	Used to remember the PID-Parameters for each wheel
 */
static pid_data_t drive_PID[2];

/**
 * External reference to the Position Variable of the left wheel.
 */
extern int16_t irq_Position_W0;
/**
 * External reference to the Position Variable of the right wheel.
 */
extern int16_t irq_Position_W1;

/**
 *	Stores the position the left wheel should hold during active braking.
 */
int16_t drive_brake_position_left;
/**
 *	Stores the position the right wheel should hold during active braking.
 */
int16_t drive_brake_position_right;

/**
 * Sets the PID-Parameter for either one wheel or both wheels.
 *
 * @param[in] wheel Either 0 for the left wheel, 1 for the right wheel or 2 for both wheels.
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
 *	@param[in] wheel Either 0 for the left wheel, 1 for the right wheel or 2 for both wheels.
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
 * @param[in] wheel Either 0 for the left wheel, 1 for the right wheel or 2 for both wheels.
 * @param[in] speed Describes the speed with which the wheel should be used.
 */
void drive_UsePID(const uint8_t wheel, const int8_t speed) {
	int16_t wheel_ModSpeed[2];
	int16_t wheel_Difference;

	if (DEBUG_ENABLE)
		debug_WriteString_P(PSTR("drive.c : drive_UsePID()\n"));
	switch (wheel) {
		case WHEEL_LEFT:
			// PID Fahrt für linkes Rad
			if (speed > 0)
				motor_SetSpeed(WHEEL_LEFT, pid_Controller(speed, wheel_ReadSpeed(WHEEL_LEFT), &drive_PID[WHEEL_LEFT]));
			else
				motor_SetSpeed(WHEEL_LEFT, -pid_Controller(-speed, -wheel_ReadSpeed(WHEEL_LEFT), &drive_PID[WHEEL_LEFT]));
			break;
		case WHEEL_RIGHT:
			// PID Fahrt für rechtes Rad
			if (speed > 0)
				motor_SetSpeed(WHEEL_RIGHT, pid_Controller(speed, wheel_ReadSpeed(WHEEL_RIGHT), &drive_PID[WHEEL_RIGHT]));
			else
				motor_SetSpeed(WHEEL_RIGHT, -pid_Controller(-speed, -wheel_ReadSpeed(WHEEL_RIGHT), &drive_PID[WHEEL_RIGHT]));
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
	drive_brake_active_left();
	drive_brake_active_right();
}

/**
 * Sets the current position as holding position used while doing active braking.
 *
 * Sets position for both wheels.
 * @todo This version is only for testing!
 */
void drive_brake_active_set(void) {
	// Set the Position of the wheels for use with active braking.
//	drive_brake_active_set_left();
//	drive_brake_active_set_right();
	extern uint8_t ACTIVE_BRAKE_ENABLE;
	extern uint8_t ACTIVE_BRAKE_AMOUNT;
	if(!ACTIVE_BRAKE_ENABLE)
		return;
	if (drive_brake_position_left == irq_Position_W0) {
		motor_SetSpeed(WHEEL_LEFT, 0);
	} else if (drive_brake_position_left < irq_Position_W0) {
		motor_SetSpeed(WHEEL_LEFT, -(ACTIVE_BRAKE_AMOUNT));
	} else {
		motor_SetSpeed(WHEEL_LEFT, ACTIVE_BRAKE_AMOUNT);
	}
	if (drive_brake_position_right == irq_Position_W1) {
		motor_SetSpeed(WHEEL_RIGHT, 0);
	} else if (drive_brake_position_right < irq_Position_W1) {
		motor_SetSpeed(WHEEL_RIGHT, -(ACTIVE_BRAKE_AMOUNT));
	} else {
		motor_SetSpeed(WHEEL_RIGHT, ACTIVE_BRAKE_AMOUNT);
	}
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
	if (drive_brake_position_left == irq_Position_W0) {
		motor_SetSpeed(WHEEL_LEFT, 0);
	} else if (drive_brake_position_left < irq_Position_W0) {
		motor_SetSpeed(WHEEL_LEFT, -(ACTIVE_BRAKE_AMOUNT));
	} else {
		motor_SetSpeed(WHEEL_LEFT, ACTIVE_BRAKE_AMOUNT);
	}
}

/**
 * Sets the position for active braking for the left wheel.
 */
void drive_brake_active_set_left(void) {
	drive_brake_position_left = irq_Position_W0;
}

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
	if (drive_brake_position_right == irq_Position_W1) {
		motor_SetSpeed(WHEEL_RIGHT, 0);
	} else if (drive_brake_position_right < irq_Position_W1) {
		motor_SetSpeed(WHEEL_RIGHT, -(ACTIVE_BRAKE_AMOUNT));
	} else {
		motor_SetSpeed(WHEEL_RIGHT, ACTIVE_BRAKE_AMOUNT);
	}
}

/**
 * Sets the position for active braking for the right wheel.
 */
void drive_brake_active_set_right(void) {
	drive_brake_position_right = irq_Position_W1;
}
/*@}*/
