#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "irq.h"
#include "definitions.h"
#include "flags.h"
#include "timer.h"
#include "motor.h"

/**
 * @defgroup MOTOR_Module Motor Module
 * This Module provides low-level access and manipulation
 * of the drives.
 * @{
 */

/**
 * Position of wheel 0 (left)
 */
int16_t irq_Position_W0 = 0;
/**
 * Position of wheel 1 (right)
 */
int16_t irq_Position_W1 = 0;
/**
 * Difference between both wheels.
 */
static int16_t irq_WheelDifference = 0;
/**
 * Probably used for triggering on position.
 *
 * \todo Needs refactoring
 */
int16_t irq_p_trigger_position[2] = { 0, 0 };
/**
 * Probably used for triggering on position.
 *
 * \todo Needs refactoring
 */
//static uint8_t irq_p_trigger = 0;
/**
 * Holds the tacho value (whatever that is)
 *
 * @todo Marked for removal
 * @todo find out what a tacho is
 */
//static tacho_t irq_tacho;

/**
 * Temp register
 */
static uint8_t sreg = 0;

/**
 * Interrupt Service Routine for wheel 0 (left) sensor A.
 */
ISR(INT4_vect) {
	// Interrupt Service Routine für Drehgeber A-0

	switch (IRQ_PIN & 0x30) {
		case ((0 << IRQ_A0) | (0 << IRQ_B0)):
		case ((1 << IRQ_A0) | (1 << IRQ_B0)):
			// Linksdrehung, Rad 0
			irq_Position_W0++;
//			irq_tacho.left++;
			irq_WheelDifference++;
//			irq_tacho.difference++;
			wheel_AddSpeed_W0();
			break;

		case ((0 << IRQ_A0) | (1 << IRQ_B0)):
		case ((1 << IRQ_A0) | (0 << IRQ_B0)):
			// Rechtsdrehung, Rad 0
			irq_Position_W0--;
//			irq_tacho.left--;
			irq_WheelDifference--;
//			irq_tacho.difference--;
			wheel_DelSpeed_W0();
			break;
	}

	// IRQ_P_Trigger, links aktiv
	if (irq_p_trigger_position[0] != 0) {
		irq_p_trigger_position[0]--;
	}
}

/**
 * Interrupt Service Routine for wheel 0 (left) sensor B.
 */
ISR(INT5_vect) {
	// Interrupt Service Routine für Drehgeber B-0

	switch (IRQ_PIN & 0x30) {
		case ((0 << IRQ_A0) | (0 << IRQ_B0)):
		case ((1 << IRQ_A0) | (1 << IRQ_B0)):
			// Rechtsdrehung, Rad 0
			irq_Position_W0--;
//			irq_tacho.left--;
			irq_WheelDifference--;
//			irq_tacho.difference--;
			wheel_DelSpeed_W0();
			break;

		case ((0 << IRQ_A0) | (1 << IRQ_B0)):
		case ((1 << IRQ_A0) | (0 << IRQ_B0)):
			// Linksdrehung, Rad 0
			irq_Position_W0++;
//			irq_tacho.left++;
			irq_WheelDifference++;
//			irq_tacho.difference++;
			wheel_AddSpeed_W0();
			break;
	}

	if (irq_p_trigger_position[0] != 0) {
		irq_p_trigger_position[0]--;
	}
}

/**
 * Interrupt Service Routine for wheel 1 (right) sensor A.
 */
ISR(INT6_vect) {
	// Interrupt Service Routine für Drehgeber A-1
	
	switch (IRQ_PIN & 0xc0) {
		case ((0 << IRQ_A1) | (0 << IRQ_B1)):
		case ((1 << IRQ_A1) | (1 << IRQ_B1)):
			// Rechtsdrehung, Rad 1
			irq_Position_W1--;
//			irq_tacho.right--;
			irq_WheelDifference++;
//			irq_tacho.difference++;
			wheel_DelSpeed_W1();
			break;

		case ((0 << IRQ_A1) | (1 << IRQ_B1)):
		case ((1 << IRQ_A1) | (0 << IRQ_B1)):
			// Linksdrehung, Rad 1
			irq_Position_W1++;
//			irq_tacho.right++;
			irq_WheelDifference--;
//			irq_tacho.difference--;
			wheel_AddSpeed_W1();
			break;
	}

	if (irq_p_trigger_position[1] != 0) {
		irq_p_trigger_position[1]--;
	}
}

/**
 * Interrupt Service Routine for wheel 1 (right) sensor B.
 */
ISR(INT7_vect) {
	// Interrupt Service Routine für Drehgeber B-1

	switch (IRQ_PIN & 0xc0) {
		case ((0 << IRQ_A1) | (0 << IRQ_B1)):
		case ((1 << IRQ_A1) | (1 << IRQ_B1)):
			// Linksdrehung, Rad 1
			irq_Position_W1++;
//			irq_tacho.right++;
			irq_WheelDifference--;
//			irq_tacho.difference--;
			wheel_AddSpeed_W1();
			break;

		case ((0 << IRQ_A1) | (1 << IRQ_B1)):
		case ((1 << IRQ_A1) | (0 << IRQ_B1)):
			// Rechtsdrehung, Rad 1
			irq_Position_W1--;
//			irq_tacho.right--;
			irq_WheelDifference++;
//			irq_tacho.difference++;
			wheel_DelSpeed_W1();
			break;
	}

	if (irq_p_trigger_position[1] != 0) {
		irq_p_trigger_position[1]--;
	}
}

/**
 * Setup for the Hall-sensors.
 */
void irq_init(void) {
	IRQ_DDR &= ~((1 << IRQ_A0) | (1 << IRQ_B0) | (1 << IRQ_A1) | (1 << IRQ_B1));
	IRQ_PORT |= (1 << IRQ_A0) | (1 << IRQ_B0) | (1 << IRQ_A1) | (1 << IRQ_B1);
	// Interrupt Ports als Eingänge mit PullUps

	EICRB = (1 << ISC70) | (1 << ISC60) | (1 << ISC50) | (1 << ISC40);
	// Any logical change on INT(7-4) generates an interrupt request

	EIMSK = (1 << INT7) | (1 << INT6) | (1 << INT5) | (1 << INT4);
	// External Interrupt Request 7 - 4 Enable 
}

/**
 * Reads the current position for the requested wheel.
 *
 * @param[in] wheel Selects for which wheel the position will be returned.
 * Possible values are #WHEEL_LEFT and #WHEEL_RIGHT
 * @return <em>int16_t</em> The Position of the given wheel.
 */
int16_t	wheel_ReadPosition(uint8_t wheel) {
	switch (wheel) {
		case WHEEL_LEFT:
			return irq_Position_W0;
			break;
		case WHEEL_RIGHT:
			return irq_Position_W1;
			break;
	}
	return 0;
}

/**
 * Resets the position counter for the given wheel.
 *
 * @param[in] wheel Selects the wheel for reset. Possible values are
 * #WHEEL_LEFT, #WHEEL_RIGHT and #WHEEL_BOTH.
 */
void wheel_ClearPosition(uint8_t wheel) {
	sreg = SREG;
	cli();
	switch (wheel) {
		case WHEEL_LEFT:
			irq_Position_W0 = 0;
			break;
		case WHEEL_RIGHT:
			irq_Position_W1 = 0;
			break;
		case WHEEL_BOTH:
			irq_Position_W0 = 0;
			irq_Position_W1 = 0;
			break;
	}
	SREG = sreg;
}

/**
 * Reads the difference between both wheels.
 *
 * @return <em>int16_t</em> The difference between the left and the right wheel.
 * @todo Find out what negative and positive values mean.
 */
int16_t wheel_ReadDifference(void) {
	return irq_WheelDifference;
}

/**
 * Sets the difference of the wheels to a defined value.
 *
 * @param[in] difference The difference of the left and right wheel.
 * @todo Find out what negative and positive values mean.
 * @todo Why is cli() here?
 */
void wheel_WriteDifference(int16_t difference) {
	sreg = SREG;
	cli();
	irq_WheelDifference = difference;
	SREG = sreg;
}

/**
 * Resets the difference between the wheels.
 *
 * @todo Why is cli() here?
 */
void wheel_ClearDifference(void) {
	sreg = SREG;
	cli();
	irq_WheelDifference = 0;
	SREG = sreg;
}

/*
void wheel_GetTacho(tacho_t * tacho) {
	sreg = SREG;
	cli();
	*tacho = irq_tacho;
	SREG = sreg;
}

void wheel_ClearTacho(void) {
	sreg = SREG;
	cli();
	irq_tacho.left = 0;
	irq_tacho.right = 0;
	irq_tacho.difference = 0;
	SREG = sreg;
}
*/
/*@}*/
