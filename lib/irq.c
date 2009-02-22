#define __AVR_ATmega2561__
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "irq.h"
#include "definitions.h"
#include "flags.h"
#include "timer.h"
#include "motor.h"

static int16_t irq_Position_W0 = 0;
static int16_t irq_Position_W1 = 0;
static int16_t irq_WheelDifference = 0;
static int16_t irq_p_trigger_position[2] = { 0, 0 };
static uint8_t irq_p_trigger = 0;
static tacho_t irq_tacho;

static int16_t irq_t_i16 = 0;
static uint8_t sreg = 0;

#define IRQ_P_TRIGGER_L   0
#define IRQ_P_TRIGGER_R   1

ISR(INT4_vect) {
	// Interrupt Service Routine für Drehgeber A-0

	switch (IRQ_PIN & 0x30) {
		case ((0 << IRQ_A0) | (0 << IRQ_B0)):
		case ((1 << IRQ_A0) | (1 << IRQ_B0)):
			// Linksdrehung, Rad 0
			irq_Position_W0++;
			irq_tacho.left++;
			irq_WheelDifference++;
			irq_tacho.difference++;
			wheel_AddSpeed_W0();
			break;

		case ((0 << IRQ_A0) | (1 << IRQ_B0)):
		case ((1 << IRQ_A0) | (0 << IRQ_B0)):
			// Rechtsdrehung, Rad 0
			irq_Position_W0--;
			irq_tacho.left--;
			irq_WheelDifference--;
			irq_tacho.difference--;
			wheel_DelSpeed_W0();
			break;
	}

	if (flagLocal_Read(&irq_p_trigger, IRQ_P_TRIGGER_L)) {
		// IRQ_P_Trigger, links aktiv
		if (irq_Position_W0 == irq_p_trigger_position[0]) {
			flag_Set(P_TRIGGER_L);
			flagLocal_Clear(&irq_p_trigger, IRQ_P_TRIGGER_L);
		}
	}
}

ISR(INT5_vect) {
	// Interrupt Service Routine für Drehgeber B-0

	switch (IRQ_PIN & 0x30) {
		case ((0 << IRQ_A0) | (0 << IRQ_B0)):
		case ((1 << IRQ_A0) | (1 << IRQ_B0)):
			// Rechtsdrehung, Rad 0
			irq_Position_W0--;
			irq_tacho.left--;
			irq_WheelDifference--;
			irq_tacho.difference--;
			wheel_DelSpeed_W0();
			break;

		case ((0 << IRQ_A0) | (1 << IRQ_B0)):
		case ((1 << IRQ_A0) | (0 << IRQ_B0)):
			// Linksdrehung, Rad 0
			irq_Position_W0++;
			irq_tacho.left++;
			irq_WheelDifference++;
			irq_tacho.difference++;
			wheel_AddSpeed_W0();
			break;
	}

	if (flagLocal_Read(&irq_p_trigger, IRQ_P_TRIGGER_L)) {
		// IRQ_P_Trigger, links aktiv
		if (irq_Position_W0 == irq_p_trigger_position[0]) {
			flag_Set(P_TRIGGER_L);
			flagLocal_Clear(&irq_p_trigger, IRQ_P_TRIGGER_L);
		}
	}
}

ISR(INT6_vect) {
	// Interrupt Service Routine für Drehgeber A-1
	
	switch (IRQ_PIN & 0xc0) {
		case ((0 << IRQ_A1) | (0 << IRQ_B1)):
		case ((1 << IRQ_A1) | (1 << IRQ_B1)):
			// Rechtsdrehung, Rad 1
			irq_Position_W1--;
			irq_tacho.right--;
			irq_WheelDifference++;
			irq_tacho.difference++;
			wheel_DelSpeed_W1();
			break;

		case ((0 << IRQ_A1) | (1 << IRQ_B1)):
		case ((1 << IRQ_A1) | (0 << IRQ_B1)):
			// Linksdrehung, Rad 1
			irq_Position_W1++;
			irq_tacho.right++;
			irq_WheelDifference--;
			irq_tacho.difference--;
			wheel_AddSpeed_W1();
			break;
	}

	if (flagLocal_Read(&irq_p_trigger, IRQ_P_TRIGGER_R)) {
		// IRQ_P_Trigger, rechts aktiv
		if (irq_Position_W1 == irq_p_trigger_position[1]) {
			flag_Set(P_TRIGGER_R);
			flagLocal_Clear(&irq_p_trigger, IRQ_P_TRIGGER_R);
		}
	}
}

ISR(INT7_vect) {
	// Interrupt Service Routine für Drehgeber B-1

	switch (IRQ_PIN & 0xc0) {
		case ((0 << IRQ_A1) | (0 << IRQ_B1)):
		case ((1 << IRQ_A1) | (1 << IRQ_B1)):
			// Linksdrehung, Rad 1
			irq_Position_W1++;
			irq_tacho.right++;
			irq_WheelDifference--;
			irq_tacho.difference--;
			wheel_AddSpeed_W1();
			break;

		case ((0 << IRQ_A1) | (1 << IRQ_B1)):
		case ((1 << IRQ_A1) | (0 << IRQ_B1)):
			// Rechtsdrehung, Rad 1
			irq_Position_W1--;
			irq_tacho.right--;
			irq_WheelDifference++;
			irq_tacho.difference++;
			wheel_DelSpeed_W1();
			break;
	}

	if (flagLocal_Read(&irq_p_trigger, IRQ_P_TRIGGER_R)) {
		// IRQ_P_Trigger, rechts aktiv
		if (irq_Position_W1 == irq_p_trigger_position[1]) {
			flag_Set(P_TRIGGER_R);
			flagLocal_Clear(&irq_p_trigger, IRQ_P_TRIGGER_R);
		}
	}
}

void irq_init(void) {
	IRQ_DDR &= ~((1 << IRQ_A0) | (1 << IRQ_B0) | (1 << IRQ_A1) | (1 << IRQ_B1));
	IRQ_PORT |= (1 << IRQ_A0) | (1 << IRQ_B0) | (1 << IRQ_A1) | (1 << IRQ_B1);
	// Interrupt Ports als Eingänge mit PullUps

	EICRB = (1 << ISC70) | (1 << ISC60) | (1 << ISC50) | (1 << ISC40);
	// Any logical change on INT(7-4) generates an interrupt request

	EIMSK = (1 << INT7) | (1 << INT6) | (1 << INT5) | (1 << INT4);
	// External Interrupt Request 7 - 4 Enable 
}

int16_t	wheel_ReadPosition(uint8_t wheel) {
	switch (wheel) {
		case 0:
			irq_t_i16 = irq_Position_W0;
			break;
		case 1:
			irq_t_i16 = irq_Position_W1;
			break;
	}
	return irq_t_i16;
}

void wheel_ClearPosition(uint8_t wheel) {
	sreg = SREG;
	cli();
	switch (wheel) {
		case 0:
			irq_Position_W0 = 0;
			break;
		case 1:
			irq_Position_W1 = 0;
			break;
		case 2:
			irq_Position_W0 = 0;
			irq_Position_W1 = 0;
			break;
	}
	SREG = sreg;
}

int16_t wheel_ReadDifference(void) {
	return irq_WheelDifference;
}

void wheel_WriteDifference(int16_t difference) {
	sreg = SREG;
	cli();
	irq_WheelDifference = difference;
	SREG = sreg;
}

void wheel_ClearDifference(void) {
	sreg = SREG;
	cli();
	irq_WheelDifference = 0;
	SREG = sreg;
}

void trigger_Set_P(const uint8_t wheel, const int16_t trigger_position) {
	//Used to set the Position trigger for the wheel(s).
	sreg = SREG;
	cli();
	switch (wheel) {
		case 0:
		case 1:
			irq_p_trigger_position[wheel] = trigger_position;
			flagLocal_Set(&irq_p_trigger, IRQ_P_TRIGGER_L + wheel);
			flag_Clear(P_TRIGGER_L + wheel);
			break;
		case 2:
			trigger_Set_P(0, trigger_position);
			trigger_Set_P(1, trigger_position);
			break;
	}
	SREG = sreg;
}

void trigger_Clear_P(const uint8_t wheel) {
	// Clears to previously set Positions for the trigger.
	sreg = SREG;
	cli();
	switch (wheel) {
		case 0:
		case 1:
			flagLocal_Clear(&irq_p_trigger, IRQ_P_TRIGGER_L + wheel);
			flag_Clear(P_TRIGGER_L + wheel);
			break;
		case 2:
			trigger_Clear_P(0);
			trigger_Clear_P(1);
			break;
	}
	SREG = sreg;
}

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
