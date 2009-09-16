#include <inttypes.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "definitions.h"
#include "led.h"
#include "pin.h"

/**
 * @defgroup TIMER_Module Timer Module
 * Provides functions and variables for timing purposes.
 * @{
 */

/**
 * Speed of the left wheel in the last 100ms.
 */
static int8_t timer_Speed_W0 = 0;
/**
 * Speed of the right wheel in the last 100ms.
 */
static int8_t timer_Speed_W1 = 0;
/**
 * Used to store the speed during the 100ms. It gets incremented by irq.c.
 */
int8_t timer_SpeedSum_W0 = 0;
/**
 * Used to store the speed during the 100ms. It gets incremented by irq.c.
 */
int8_t timer_SpeedSum_W1 = 0;
/**
 * Stores the current value for the time triggers.
 * 
 * This Values will be decremented over time until they have reached 0.
 */
uint16_t timer_t_trigger_counter[2] = { 0, 0 };
/**
 * Stores the system uptime in seconds (at least until overflow.
 */
uint16_t timer_1s_counter = 0;
/**
 * Helper counter used to count timer_1s_counter up.
 */
uint8_t timer_100ms_counter = 0;
uint8_t timer_global_flags = 0;

/**
 * The interrupt which gets called every 100ms.
 */
ISR(TIMER1_COMPC_vect) {
	// ISR wird alle 100 Millisekunden ausgeführt
	OCR1C = TCNT1 + 25000;
	// Neuen Wert für Compare Match C setzen
	timer_global_flags |= TIMER_100MS;
	// und TIMER_100MS Flag setzen

	// linker T_Trigger aktiv
	if (timer_t_trigger_counter[0] != 0) {
		timer_t_trigger_counter[0]--;
	}
	if (timer_t_trigger_counter[1] != 0) {
		timer_t_trigger_counter[1]--;
	}

	// Geschwindigkeiten in timer_Speed_W0/1 übertragen
	timer_Speed_W0 = timer_SpeedSum_W0;
	timer_Speed_W1 = timer_SpeedSum_W1;
	timer_SpeedSum_W0 = 0;
	timer_SpeedSum_W1 = 0;

	// Increase counter
	timer_100ms_counter++;
}

/**
 * Timer interrupt being called every 262ms.
 */
ISR(TIMER1_OVF_vect) {
	// ISR wird alle 262 Millisekunden aufgerufen
	timer_global_flags |= TIMER_262MS;
	// TIMER_262MS Flag setzen
	if( timer_100ms_counter > 10 ) {
		timer_1s_counter += timer_100ms_counter / 10;
		timer_100ms_counter -= (timer_100ms_counter / 10) * 10;
	}
}

/**
 * Initializes the timer registers.
 */
void timer_init(void) {
	// Compare Matches einstellen
//	OCR1A = 250;
	// Compare Match A -> 1kHz -> 1ms
//	OCR1B = 2500;
	// Compare Match B -> 100Hz -> 10ms
	OCR1C = 25000;
	// Compare Match C -> 10Hz -> 100ms
	TIMSK1 = (1 << OCIE1C) | /*(1 << OCIE1B) | (1 << OCIE1A) |*/ (1 << TOIE1);
	// Aktiviere Compare Match A+B+C IRQ und Overflow IRQ
	TCCR1B = (3 << CS10);
	// Prescaler = clkIO/64 -> 250kHz -> 4µs
}

/**
 * Reads the speed of the given wheel. (changes every 100ms)
 *
 * @param[in] wheel Either #WHEEL_LEFT or #WHEEL_RIGHT
 * @return <em>int8_t</em> The Speed of the requested wheel.
 */
int8_t wheel_ReadSpeed(const uint8_t wheel) {
	switch (wheel) {
		case WHEEL_LEFT:
			return timer_Speed_W0;
			break;
		case WHEEL_RIGHT:
			return timer_Speed_W1;
			break;
	}
	return 0;
}
/*@}*/
