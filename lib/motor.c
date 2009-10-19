#include <avr/io.h>
#include <inttypes.h>
#include <stdlib.h>
#include "debug.h"
#include "definitions.h"
#include "motor.h"

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
 * Simple function to set the direction for motor 0 to forward.
 */
static void motor_set_direction_forward_0(void) {
	INPUT_PORT |= (1 << INPUT2);
	INPUT_PORT &= ~(1 << INPUT1);
}

/**
 * Simple function to set the direction for motor 0 to backward.
 */
static void motor_set_direction_backward_0(void) {
	INPUT_PORT &= ~(1 << INPUT2);
	INPUT_PORT |= (1 << INPUT1);
}

/**
 * Simple function to set the direction for motor 1 to forward.
 */
static void motor_set_direction_forward_1(void) {
	INPUT_PORT &= ~(1 << INPUT4);
	INPUT_PORT |= (1 << INPUT3);
}

/**
 * Simple function to set the direction for motor 1 to backward.
 */
static void motor_set_direction_backward_1(void) {
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
static uint8_t motor_calculate_pwm(int8_t speed) {
	// If speed = 0 -> PWM = 0,
	// 1 <= speed < 38 -> PWM = speed + 38,
	// 38 <= speed -> PWM = (speed * 2) + 1.
	if ((speed < 1)) {
		return 0;
	} else if ((speed < 38)) {
		return (speed + 38);
	} else {
		return ((speed * 2) + 1);
	}
}

/**
 * Simple typedef to ease writing.
 */
typedef void (*direction_function)(void);

/**
 * A function table for the direction settings.
 */
direction_function motor_direction_functions[NUMBER_OF_WHEELS * 2] = {
	&motor_set_direction_forward_0,
	&motor_set_direction_backward_0,
	&motor_set_direction_forward_1,
	&motor_set_direction_backward_1
};

/**
 * Array of the PWM SFRs.
 * @todo Check whether or not this works.
 */
volatile uint8_t* motor_pwm_registers[NUMBER_OF_WHEELS] = {
	(volatile uint8_t *)((0x27) + __SFR_OFFSET),
	(volatile uint8_t *)((0x28) + __SFR_OFFSET)
};


/**
 * Sets the speed for a motor.
 *
 * @param[in] wheel The motor for the given wheel. Valid values are #WHEEL_LEFT,
 * #WHEEL_RIGHT and #WHEEL_ALL.
 * @param[in] speed The desired speed..
 * @todo Test this
 */
void motor_set_speed(uint8_t wheel, int8_t speed) {
	uint8_t absolute = abs(speed);
	if (DEBUG_ENABLE) {
		debug_write_integer(PSTR("motor.c : motor_set_speed() : speed = "), speed);
		debug_write_integer(PSTR("motor.c : motor_set_speed() : wheel = "), wheel);
	}
	switch (wheel) {
		case WHEEL_ALL:
			for (uint8_t i = 0; i < NUMBER_OF_WHEELS; i++) {
				motor_speed[i] = speed;
				motor_direction_functions[i * 2 + (1 * (speed < 0))]();
				(*motor_pwm_registers[i]) = motor_calculate_pwm(absolute);
			}
			break;
		default:
			motor_speed[wheel] = speed;
			motor_direction_functions[wheel * 2 + (1 * (speed < 0))]();
			(*motor_pwm_registers[wheel]) = motor_calculate_pwm(absolute);
			if (DEBUG_ENABLE) {
				debug_write_integer(PSTR("motor.c : motor_set_speed() : new pwm = "), motor_calculate_pwm(absolute));
			}
			break;
	}
}

/**
 * Reads the set speed for a motor.
 *
 * @param[in] wheel The motor of the wheel, which speed one wants. Valid
 * values are #WHEEL_LEFT and #WHEEL_RIGHT.
 * @return <em>int8_t</em> The speed of the requested motor.
 */
int8_t motor_read_speed(uint8_t wheel) {
	return motor_speed[wheel];
}

/*@}*/
