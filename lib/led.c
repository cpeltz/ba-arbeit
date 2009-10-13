#include "led.h"

/**
 * @defgroup LED_Library LED Library.
 * The LED Library provides functions to deal with the
 * onboard LED's.
 * @{
 */

/**
 * Setup the LED's for useage.
 */
void led_init(void) {
	// Initialisiere LED Ausgänge und schalte LEDs aus
	LED_DDR |= (1 << LED_RED1) | (1 << LED_RED2) | (1 << LED_ORANGE) | (1 << LED_GREEN) | (1 << LED_BLUE);
	LED_PORT = 0;
}
/*@}*/
