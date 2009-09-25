#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "irq.h"
#include "definitions.h"
#include "timer.h"
#include "motor.h"
#include "pin.h"

/**
 * @defgroup MOTOR_Module Motor Module
 * This Module provides low-level access and manipulation
 * of the drives.
 * @{
 */


/**
 * Position of the wheels
 */
int16_t irq_Position[NUMBER_OF_WHEELS] = { 0, 0 };
/**
 * Difference between both wheels.
 */
static int16_t irq_WheelDifference = 0;
/**
 * Probably used for triggering on position.
 */
int16_t irq_p_trigger_position[NUMBER_OF_WHEELS] = { 0, 0 };

/**
 * Used to store the speed during the 100ms. It gets incremented by irq.c.
 */
extern int8_t timer_SpeedSum[NUMBER_OF_WHEELS];

/**
 * Interrupt Service Routine for wheel 0 (left) sensor A.
 */
ISR(INT4_vect) {
	// Interrupt Service Routine für Drehgeber A-0
	pin_set_C(0);

	switch (IRQ_PIN & 0x30) {
		case ((0 << IRQ_A0) | (0 << IRQ_B0)):
		case ((1 << IRQ_A0) | (1 << IRQ_B0)):
			// Linksdrehung, Rad 0
			irq_Position[WHEEL_LEFT]++;
			irq_WheelDifference++;
			// Increment the speed for the wheel
			// needed for the PID-Controller
			if ((timer_SpeedSum[WHEEL_LEFT] < 127))
				timer_SpeedSum[WHEEL_LEFT]++;
			break;

		case ((0 << IRQ_A0) | (1 << IRQ_B0)):
		case ((1 << IRQ_A0) | (0 << IRQ_B0)):
			// Rechtsdrehung, Rad 0
			irq_Position[WHEEL_LEFT]--;
			irq_WheelDifference--;
			// Decrement the speed for the wheel
			// needed for the PID_Controller
			if ((timer_SpeedSum[WHEEL_LEFT] > -127))
				timer_SpeedSum[WHEEL_LEFT]--;
			break;
	}

	// IRQ_P_Trigger, links aktiv
	if (irq_p_trigger_position[WHEEL_LEFT] != 0) {
		irq_p_trigger_position[WHEEL_LEFT]--;
	}
	pin_clear_C(0);
}

/**
 * Interrupt Service Routine for wheel 0 (left) sensor B.
 */
ISR(INT5_vect) {
	// Interrupt Service Routine für Drehgeber B-0
	pin_set_C(1);

	switch (IRQ_PIN & 0x30) {
		case ((0 << IRQ_A0) | (0 << IRQ_B0)):
		case ((1 << IRQ_A0) | (1 << IRQ_B0)):
			// Rechtsdrehung, Rad 0
			irq_Position[WHEEL_LEFT]--;
			irq_WheelDifference--;
			// Decrement the speed for the wheel
			// needed for the PID_Controller
			if ((timer_SpeedSum[WHEEL_LEFT] > -127))
				timer_SpeedSum[WHEEL_LEFT]--;
			break;

		case ((0 << IRQ_A0) | (1 << IRQ_B0)):
		case ((1 << IRQ_A0) | (0 << IRQ_B0)):
			// Linksdrehung, Rad 0
			irq_Position[WHEEL_LEFT]++;
			irq_WheelDifference++;
			// Increment the speed for the wheel
			// needed for the PID-Controller
			if ((timer_SpeedSum[WHEEL_LEFT] < 127))
				timer_SpeedSum[WHEEL_LEFT]++;
			break;
	}

	if (irq_p_trigger_position[WHEEL_LEFT] != 0) {
		irq_p_trigger_position[WHEEL_LEFT]--;
	}
	pin_clear_C(1);
}

/**
 * Interrupt Service Routine for wheel 1 (right) sensor A.
 */
ISR(INT6_vect) {
	// Interrupt Service Routine für Drehgeber A-1
	pin_set_C(2);
	
	switch (IRQ_PIN & 0xc0) {
		case ((0 << IRQ_A1) | (0 << IRQ_B1)):
		case ((1 << IRQ_A1) | (1 << IRQ_B1)):
			// Rechtsdrehung, Rad 1
			irq_Position[WHEEL_RIGHT]--;
			irq_WheelDifference++;
			// Decrement the speed for the wheel
			// needed for the PID_Controller
			if ((timer_SpeedSum[WHEEL_RIGHT] > -127))
				timer_SpeedSum[WHEEL_RIGHT]--;
			break;

		case ((0 << IRQ_A1) | (1 << IRQ_B1)):
		case ((1 << IRQ_A1) | (0 << IRQ_B1)):
			// Linksdrehung, Rad 1
			irq_Position[WHEEL_RIGHT]++;
			irq_WheelDifference--;
			// Increment the speed for the wheel
			// needed for the PID-Controller
			if ((timer_SpeedSum[WHEEL_RIGHT] < 127))
				timer_SpeedSum[WHEEL_RIGHT]++;
			break;
	}

	if (irq_p_trigger_position[WHEEL_RIGHT] != 0) {
		irq_p_trigger_position[WHEEL_RIGHT]--;
	}
	pin_clear_C(2);
}

/**
 * Interrupt Service Routine for wheel 1 (right) sensor B.
 */
ISR(INT7_vect) {
	// Interrupt Service Routine für Drehgeber B-1
	pin_set_C(3);

	switch (IRQ_PIN & 0xc0) {
		case ((0 << IRQ_A1) | (0 << IRQ_B1)):
		case ((1 << IRQ_A1) | (1 << IRQ_B1)):
			// Linksdrehung, Rad 1
			irq_Position[WHEEL_RIGHT]++;
			irq_WheelDifference--;
			// Increment the speed for the wheel
			// needed for the PID-Controller
			if ((timer_SpeedSum[WHEEL_RIGHT] < 127))
				timer_SpeedSum[WHEEL_RIGHT]++;
			break;

		case ((0 << IRQ_A1) | (1 << IRQ_B1)):
		case ((1 << IRQ_A1) | (0 << IRQ_B1)):
			// Rechtsdrehung, Rad 1
			irq_Position[WHEEL_RIGHT]--;
			irq_WheelDifference++;
			// Decrement the speed for the wheel
			// needed for the PID_Controller
			if ((timer_SpeedSum[WHEEL_RIGHT] > -127))
				timer_SpeedSum[WHEEL_RIGHT]--;
			break;
	}

	if (irq_p_trigger_position[WHEEL_RIGHT] != 0) {
		irq_p_trigger_position[WHEEL_RIGHT]--;
	}
	pin_clear_C(3);
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
	return irq_Position[wheel];
}

/**
 * Resets the position counter for the given wheel.
 *
 * @param[in] wheel Selects the wheel for reset. Possible values are
 * #WHEEL_LEFT, #WHEEL_RIGHT and #WHEEL_BOTH.
 */
void wheel_ClearPosition(uint8_t wheel) {
	cli();
	switch (wheel) {
		case WHEEL_LEFT:
		case WHEEL_RIGHT:
			irq_Position[wheel] = 0;
			break;
		case WHEEL_BOTH:
			irq_Position[WHEEL_LEFT] = 0;
			irq_Position[WHEEL_RIGHT] = 0;
			break;
	}
	sei();
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
 */
void wheel_WriteDifference(int16_t difference) {
	cli();
	irq_WheelDifference = difference;
	sei();
}

/**
 * Resets the difference between the wheels.
 */
void wheel_ClearDifference(void) {
	cli();
	irq_WheelDifference = 0;
	sei();
}
/*@}*/
