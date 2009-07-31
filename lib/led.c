#include <avr/io.h>
#include "led.h"
#include "definitions.h"
#include "debug.h"

/**
 * @defgroup LED_Library LED Library.
 * The LED Library provides functions to deal with the
 * onboard LED's.
 * @{
 */

/**
 * Storage for the state of the led's
 */
static uint16_t led_State = 0;

/**
 * Switches all LED's off.
 */
void led_switchoff(void) {
	// Alle LEDs ausschalten
	led_State &= (1<<15);
	LED_PORT = led_State;
}

/**
 * Setup the LED's for useage.
 */
void led_init(void) {
	// Initialisiere LED Ausgänge und schalte LEDs aus
	LED_DDR |= (1 << LED_RED1) | (1 << LED_RED2) | (1 << LED_ORANGE) | (1 << LED_GREEN) | (1 << LED_BLUE);
	led_switchoff();
}

/**
 * Set a specific LED to a specific state.
 *
 * @param[in] led The LED to be used. Valid values are #LED_RED1,
 * #LED_RED2, #LED_ORANGE, #LED_GREEN and #LED_BLUE.
 * @param[in] state The state the LED should enter.
 * @todo Find out more about the valid values for state.
 * @todo Broken, needs fixing.
 */
void led_switch(uint8_t led, uint8_t state) {
	switch(led) {
		case LED_RED1:
			led_State &= ~(7<<LED_RED1_BIT);
			led_State |= (state<<LED_RED1_BIT);
			break;
		case LED_RED2:
			led_State &= ~(7<<LED_RED2_BIT);
			led_State |= (state<<LED_RED2_BIT);
			break;
		case LED_ORANGE:
			led_State &= ~(7<<LED_ORANGE_BIT);
			led_State |= (state<<LED_ORANGE_BIT);
			break;
		case LED_GREEN:
			led_State &= ~(7<<LED_GREEN_BIT);
			led_State |= (state<<LED_GREEN_BIT);
			break;
		case LED_BLUE:
			led_State &= ~(7<<LED_BLUE_BIT);
			led_State |= (state<<LED_BLUE_BIT);
			break;
	}
	LED_PORT |= (1 << led);
	LED_PORT &= ~(state << led);
	//LED_DDR = led_State;
}

void wait_262ms(void) {
	extern uint8_t timer_global_flags;

	while(!(timer_global_flags & TIMER_262MS));
	timer_global_flags = 0;
}

/**
 * Runs a simple test for the LED's.
 */
void led_test(void) {
	led_switch(LED_RED1, ON);
	wait_262ms();

	led_switch(LED_RED2, ON);
	wait_262ms();

	led_switch(LED_ORANGE, ON);
	wait_262ms();

	led_switch(LED_GREEN, ON);
	wait_262ms();

	led_switch(LED_BLUE, ON);
	wait_262ms();

	led_switchoff();
}

/**
 * Reads the state of all LED's.
 *
 * @return <em>uint16_t</em> The state of the LED's.
 */
uint16_t led_GetState(void) {
	return led_State;
}

/**
 * Sets the "Toggle"-Bit (whatever that is).
 * @todo Find out what the toggle bit is.
 */
void led_ToggleBit(void) {
	led_State ^= (1<<15);
	LED_DDR = led_State;
}

/**
 * Reads the state of the toggle Bit.
 *
 * @return <em>uint8_t</em> 1 if the toggle bit is on, 0 otherwise.
 */
uint8_t led_ReadToggleBit(void) {
	return (led_State>>15);
}
/*@}*/
