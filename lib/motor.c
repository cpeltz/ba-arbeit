/*#define __AVR_ATmega2561__*/
#include <avr/io.h>
#include <inttypes.h>
#include <stdlib.h>
#include "definitions.h"
#include "motor.h"
#include "flags.h"

/**
 * @defgroup MOTOR_Module Motor Module
 * This Module provides low-level access and manipulation
 * of the drives.
 * @{
 */

/**
 * Stores the speed for both wheels.
 */
static int8_t motor_speed[2];

/**
 * Simple function to let motor 0 run forward.
 */
static void motor_RunForward_M0(void) {
	INPUT_PORT |= (1 << INPUT2);
	INPUT_PORT &= ~(1 << INPUT1);
}

/**
 * Simple function to let motor 0 run backward.
 */
static void motor_RunReverse_M0(void) {
	INPUT_PORT &= ~(1 << INPUT2);
	INPUT_PORT |= (1 << INPUT1);
}

/**
 * Simple function to let motor 1 run forward.
 */
static void motor_RunForward_M1(void) {
	INPUT_PORT &= ~(1 << INPUT4);
	INPUT_PORT |= (1 << INPUT3);
}

/**
 * Simple function to let motor 1 run backward.
 */
static void motor_RunReverse_M1(void) {
	INPUT_PORT |= (1 << INPUT4);
	INPUT_PORT &= ~(1 << INPUT3);
}

/**
 * Initialize the motor module.
 */
void motor_init(void) {
	// Ausgänge für L298 Motortreiber initialisieren
	INPUT_DDR |= (1 << INPUT1) | (1 << INPUT2) | (1 << INPUT3) | (1 << INPUT4);
	INPUT_PORT &= ~((1 << INPUT1) | (1 << INPUT2) | (1 << INPUT3) | (1 << INPUT4));

	ENABLE_A_DDR |= (1 << ENABLE_A);
	ENABLE_A_PORT &= ~(1 << ENABLE_A);

	ENABLE_B_DDR |= (1 << ENABLE_B);
	ENABLE_B_PORT &= ~(1 << ENABLE_B);

	// Timer für PWM an ENABLE-Ausgängen initialisieren
	TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM00);
	TCCR0B = (3 << CS00);

	// Timer 0:
	// PWM, Phase correct, Prescaler clkIO/64
	// Clear OC0A / OC0B on compare match when up-counting. Set OC0A / OC0B
	// on compare match when downcounting.
	ENABLE_A_PWM = 0;
	ENABLE_B_PWM = 0;
}

/**
 * Helper function. Used to calculate the PWM value.
 *
 * The PWM devices needs a good value. Otherwise the motor
 * won't run smoothly.
 * @param[in] speed The speed for which the PWM value should be calculated.
 * @return <em>uint8_t</em> The PWM value for the speed parameter.
 */
static uint8_t motor_CalculatePWM(int8_t speed) {
	// Wenn speed = 0 -> PWM = 0,
	// 1 < speed < 38 -> PWM = speed + 38,
	// 38 < speed -> PWM = (speed * 2) + 1.
	if ((speed < 1)) {
		return 0;
	} else if ((speed < 38)) {
		return (speed + 38);
	} else {
		return ((speed * 2) + 1);
	}
}

/**
 * Sets the speed for motor 0.
 *
 * @param[in] speed The desired speed of motor 0.
 */
static void motor_SetSpeed_M0(int8_t speed) {
	uint8_t positive;

	positive = abs(speed);
	if (!(speed)) {
		ENABLE_A_PWM = 0;
	} else {
		switch ((speed / positive)) {
			case 1:
				motor_RunForward_M0();
				break;
			case -1:
				motor_RunReverse_M0();
				break;
		}
		ENABLE_A_PWM = motor_CalculatePWM(positive);
	}
}

/**
 * Sets the speed for motor 1.
 *
 * @param[in] speed The desired speed of motor 1.
 */
static void motor_SetSpeed_M1(int8_t speed) {
	uint8_t positive;

	positive = abs(speed);
	if (!(speed)) {
		ENABLE_B_PWM = 0;
	} else {
		switch ((speed / positive)) {
			case 1:
				motor_RunForward_M1();
				break;
			case -1:
				motor_RunReverse_M1();
				break;
		}
		ENABLE_B_PWM = motor_CalculatePWM(positive);
	}
}

/**
 * Sets the speed for a motor.
 *
 * @param[in] motor The motor. Valid values are #WHEEL_LEFT,
 * #WHEEL_RIGHT and #WHEEL_BOTH.
 * @param[in] speed The desired speed of motor 0.
 */
void motor_SetSpeed(uint8_t motor, int8_t speed) {
	switch (motor) {
		case WHEEL_LEFT:
			motor_speed[WHEEL_LEFT] = speed;
			motor_SetSpeed_M0(speed);
			break;
		case WHEEL_RIGHT:
			motor_speed[WHEEL_RIGHT] = speed;
			motor_SetSpeed_M1(speed);
			break;
		case WHEEL_BOTH:
			motor_speed[WHEEL_LEFT] = speed;
			motor_speed[WHEEL_RIGHT] = speed;
			motor_SetSpeed_M0(speed);
			motor_SetSpeed_M1(speed);
			break;
	}
}

/**
 * Reads the set speed for a motor.
 *
 * @param[in] motor The motor, which speed one wants. Valid
 * values are #WHEEL_LEFT and #WHEEL_RIGHT.
 * @return <em>int8_t</em> The speed of the requested motor.
 */
int8_t motor_ReadSpeed(uint8_t motor) {
	return motor_speed[motor];
}

/**
 * Fills the given structure with avaiable data.
 * @todo Implement.
 */
void motor_status(global_state_t *state) {}
/*@}*/
